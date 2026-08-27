import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("glTF 2.0 JSON and GLB memory streams preserve shape bounds", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);

  const glb = await client.exportGLTF(box, { format: "glb", linearDeflection: 0.05 });
  assert.equal(new TextDecoder().decode(glb.slice(0, 4)), "glTF");
  assert.deepEqual(await client.probeFormat(glb), {
    format: "gltf", encoding: "glb", confidence: "exact",
  });
  const fromGlb = await scope.importGLTF(glb);
  const glbBounds = await client.bbox(fromGlb);
  assert.ok(glbBounds.min.every((value) => Math.abs(value) < 1e-6));
  assert.ok(glbBounds.max.every((value, index) => Math.abs(value - [2, 3, 4][index]) < 1e-6));
  assert.equal(await client.isValid(fromGlb), true);
  const glbFace = await scope.getSubShape(fromGlb, "face", 0);
  const glbAttributes = await client.triangulationData(glbFace, { includeUV: true });
  assert.equal(glbAttributes.normals.length, glbAttributes.positions.length);
  assert.equal(glbAttributes.uvs.length, glbAttributes.positions.length / 3 * 2);

  const gltf = await client.exportGLTF(box, { format: "gltf", linearDeflection: 0.05 });
  assert.deepEqual(await client.probeFormat(gltf), {
    format: "gltf", encoding: "json", confidence: "exact",
  });
  const document = JSON.parse(new TextDecoder().decode(gltf));
  assert.equal(document.asset.version, "2.0");
  assert.deepEqual(document.meshes[0].primitives[0].attributes, { POSITION: 0, NORMAL: 1, TEXCOORD_0: 2 });
  assert.match(document.buffers[0].uri, /^data:application\/octet-stream;base64,/);
  const fromGltf = await scope.importGLTF(gltf);
  const gltfBounds = await client.bbox(fromGltf);
  assert.ok(gltfBounds.min.every((value) => Math.abs(value) < 1e-6));
  assert.ok(gltfBounds.max.every((value, index) => Math.abs(value - [2, 3, 4][index]) < 1e-6));
  const gltfFace = await scope.getSubShape(fromGltf, "face", 0);
  const gltfAttributes = await client.triangulationData(gltfFace, { includeUV: true });
  assert.equal(gltfAttributes.normals.length, gltfAttributes.positions.length);
  assert.equal(gltfAttributes.uvs.length, gltfAttributes.positions.length / 3 * 2);

  document.nodes[0].translation = [5, 7, -3];
  const transformed = await scope.importGLTF(new TextEncoder().encode(JSON.stringify(document)).buffer);
  const transformedBounds = await client.bbox(transformed);
  assert.ok(Math.abs(transformedBounds.min[0] - 5) < 1e-6);
  assert.ok(Math.abs(transformedBounds.min[1] - 3) < 1e-6);
  assert.ok(Math.abs(transformedBounds.min[2] - 7) < 1e-6);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF document import preserves scenes, instances, materials, and resolved resources", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const mesh = new ArrayBuffer(42);
  const meshView = new DataView(mesh);
  [0, 0, 0, 1, 0, 0, 0, 1, 0].forEach((value, index) => meshView.setFloat32(index * 4, value, true));
  [0, 1, 2].forEach((value, index) => meshView.setUint16(36 + index * 2, value, true));
  const source = {
    asset: { version: "2.0", extras: { source: "fixture" } },
    extensionsUsed: ["VENDOR_metadata"],
    scene: 1,
    scenes: [{ nodes: [0] }, { nodes: [1], extras: { active: true } }],
    nodes: [
      { camera: 0 },
      { translation: [5, 0, 0], children: [2] },
      { mesh: 0, scale: [-1, 1, 1], extras: { instance: true } },
    ],
    meshes: [{ primitives: [
      { attributes: { POSITION: 0 }, indices: 1, material: 0 },
      { attributes: { POSITION: 0 }, indices: 1, material: 1 },
    ] }],
    materials: [
      { pbrMetallicRoughness: { baseColorTexture: { index: 0 }, metallicFactor: 0.25 } },
      { alphaMode: "BLEND", doubleSided: true, extensions: { VENDOR_material: { value: 1 } } },
    ],
    samplers: [{ magFilter: 9729 }],
    textures: [{ sampler: 0, source: 0 }],
    images: [{ uri: "albedo.png", mimeType: "image/png" }],
    cameras: [{ type: "perspective", perspective: { yfov: 1, znear: 0.1 } }],
    skins: [{ joints: [2] }],
    animations: [{ samplers: [{ input: 1, output: 0 }], channels: [{ sampler: 0, target: { node: 2, path: "translation" } }] }],
    buffers: [
      { uri: "positions.bin", byteLength: 36 },
      { uri: "indices.bin", byteLength: 6 },
    ],
    bufferViews: [
      { buffer: 0, byteOffset: 0, byteLength: 36 },
      { buffer: 1, byteOffset: 0, byteLength: 6 },
    ],
    accessors: [
      { bufferView: 0, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: 1, componentType: 5123, count: 3, type: "SCALAR" },
    ],
  };
  const data = new TextEncoder().encode(JSON.stringify(source)).buffer;
  const image = Uint8Array.of(137, 80, 78, 71).buffer;
  const positions = mesh.slice(0, 36);
  const indices = mesh.slice(36);
  const imported = await scope.importGLTF(data, {
    includeDocument: true,
    resources: { "positions.bin": positions, "indices.bin": indices, "albedo.png": image },
  });
  assert.ok(imported.shape);
  assert.equal(imported.activeScene, 1);
  assert.deepEqual(imported.sceneRoots, [1]);
  assert.equal(imported.primitives.length, 2);
  assert.deepEqual(imported.primitives.map(({ nodeIndex, material, vertexStart, indexStart }) => (
    { nodeIndex, material, vertexStart, indexStart }
  )), [
    { nodeIndex: 2, material: 0, vertexStart: 0, indexStart: 0 },
    { nodeIndex: 2, material: 1, vertexStart: 3, indexStart: 3 },
  ]);
  assert.deepEqual([...imported.indices], [0, 2, 1, 3, 5, 4]);
  assert.deepEqual(imported.document, source);
  assert.deepEqual(imported.buffers.map(({ uri }) => uri), ["positions.bin", "indices.bin"]);
  assert.deepEqual([...new Uint8Array(imported.buffers[0].data)], [...new Uint8Array(positions)]);
  assert.deepEqual([...new Uint8Array(imported.buffers[1].data)], [...new Uint8Array(indices)]);
  assert.deepEqual(imported.resources.map(({ uri }) => uri), ["albedo.png"]);
  assert.deepEqual([...new Uint8Array(imported.resources[0].data)], [...new Uint8Array(image)]);

  const exportedJSON = await client.exportGLTF(imported, { format: "gltf" });
  assert.deepEqual(JSON.parse(new TextDecoder().decode(exportedJSON.data)), source);
  assert.deepEqual(Object.keys(exportedJSON.resources).sort(), ["albedo.png", "indices.bin", "positions.bin"]);
  const jsonRoundTrip = await scope.importGLTF(exportedJSON.data, {
    includeDocument: true,
    resources: exportedJSON.resources,
  });
  assert.deepEqual(jsonRoundTrip.document, source);
  assert.deepEqual(jsonRoundTrip.primitives, imported.primitives);

  const exportedGLB = await client.exportGLTF(imported, { format: "glb" });
  assert.deepEqual(Object.keys(exportedGLB.resources), ["albedo.png"]);
  assert.deepEqual(await client.probeFormat(exportedGLB.data), {
    format: "gltf", encoding: "glb", confidence: "exact",
  });
  const glbRoundTrip = await scope.importGLTF(exportedGLB.data, {
    includeDocument: true,
    resources: exportedGLB.resources,
  });
  assert.equal(glbRoundTrip.activeScene, 1);
  assert.deepEqual(glbRoundTrip.sceneRoots, [1]);
  assert.deepEqual(glbRoundTrip.document.materials, source.materials);
  assert.deepEqual(glbRoundTrip.document.animations, source.animations);
  assert.deepEqual(glbRoundTrip.document.skins, source.skins);
  assert.deepEqual(glbRoundTrip.positions, imported.positions);
  assert.deepEqual(glbRoundTrip.indices, imported.indices);

  await assert.rejects(
    scope.importGLTF(data, {
      includeDocument: true,
      resources: { "positions.bin": positions, "indices.bin": indices },
    }),
    (error) => error.code === "ImportExportFailed" && /albedo\.png/.test(error.message),
  );
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF sparse accessors override positions, normals, UVs, and indices", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const mesh = new ArrayBuffer(96);
  const view = new DataView(mesh);
  [0, 0, 0, 1, 0, 0, 0, 1, 0].forEach((value, index) => {
    view.setFloat32(index * 4, value, true);
  });
  [0, 127, 0, 0, 127, 0, 0, 127, 0].forEach((value, index) => {
    view.setInt8(36 + index, value);
  });
  [0, 0, 65535, 0, 0, 65535].forEach((value, index) => {
    view.setUint16(46 + index * 2, value, true);
  });
  [0, 0, 0].forEach((value, index) => view.setUint16(58 + index * 2, value, true));
  view.setUint8(64, 1);
  [2, 0, 0].forEach((value, index) => view.setFloat32(68 + index * 4, value, true));
  view.setUint8(80, 2);
  [127, 0, 0].forEach((value, index) => view.setInt8(81 + index, value));
  view.setUint8(84, 1);
  [32768, 65535].forEach((value, index) => view.setUint16(86 + index * 2, value, true));
  view.setUint8(90, 1);
  view.setUint8(91, 2);
  view.setUint16(92, 1, true);
  view.setUint16(94, 2, true);

  const source = {
    asset: { version: "2.0" },
    scene: 0,
    scenes: [{ nodes: [0] }],
    nodes: [{ mesh: 0 }],
    meshes: [{ primitives: [{
      attributes: { POSITION: 0, NORMAL: 1, TEXCOORD_0: 2 },
      indices: 3,
    }] }],
    buffers: [{
      byteLength: mesh.byteLength,
      uri: `data:application/octet-stream;base64,${Buffer.from(mesh).toString("base64")}`,
    }],
    bufferViews: [
      { buffer: 0, byteOffset: 0, byteLength: 36 },
      { buffer: 0, byteOffset: 36, byteLength: 9 },
      { buffer: 0, byteOffset: 46, byteLength: 12 },
      { buffer: 0, byteOffset: 58, byteLength: 6 },
      { buffer: 0, byteOffset: 64, byteLength: 1 },
      { buffer: 0, byteOffset: 68, byteLength: 12 },
      { buffer: 0, byteOffset: 80, byteLength: 1 },
      { buffer: 0, byteOffset: 81, byteLength: 3 },
      { buffer: 0, byteOffset: 84, byteLength: 1 },
      { buffer: 0, byteOffset: 86, byteLength: 4 },
      { buffer: 0, byteOffset: 90, byteLength: 2 },
      { buffer: 0, byteOffset: 92, byteLength: 4 },
    ],
    accessors: [
      {
        bufferView: 0, componentType: 5126, count: 3, type: "VEC3",
        sparse: { count: 1, indices: { bufferView: 4, componentType: 5121 }, values: { bufferView: 5 } },
      },
      {
        bufferView: 1, componentType: 5120, normalized: true, count: 3, type: "VEC3",
        sparse: { count: 1, indices: { bufferView: 6, componentType: 5121 }, values: { bufferView: 7 } },
      },
      {
        bufferView: 2, componentType: 5123, normalized: true, count: 3, type: "VEC2",
        sparse: { count: 1, indices: { bufferView: 8, componentType: 5121 }, values: { bufferView: 9 } },
      },
      {
        componentType: 5123, count: 3, type: "SCALAR",
        sparse: { count: 2, indices: { bufferView: 10, componentType: 5121 }, values: { bufferView: 11 } },
      },
    ],
  };
  const imported = await scope.importGLTF(
    new TextEncoder().encode(JSON.stringify(source)).buffer,
    { includeDocument: true },
  );
  assert.deepEqual([...imported.positions].map((value) => Math.abs(value)), [0, 0, 0, 2, 0, 0, 0, 0, 1]);
  assert.deepEqual([...imported.indices], [0, 1, 2]);
  assert.deepEqual([...imported.normals].map((value) => Math.abs(value)), [0, 0, 1, 0, 0, 1, 1, 0, 0]);
  assert.ok(Math.abs(imported.uvs[2] - 32768 / 65535) < 1e-6);
  assert.deepEqual([...imported.uvs].slice(0, 2), [0, 0]);
  assert.deepEqual([...imported.uvs].slice(3), [1, 0, 1]);
  assert.deepEqual(imported.document, source);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF import statically evaluates sparse POSITION and dense NORMAL morph targets", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const mesh = new ArrayBuffer(124);
  const view = new DataView(mesh);
  [0, 0, 0, 1, 0, 0, 0, 1, 0].forEach((value, index) => {
    view.setFloat32(index * 4, value, true);
  });
  [0, 0, 1, 0, 0, 1, 0, 0, 1].forEach((value, index) => {
    view.setFloat32(36 + index * 4, value, true);
  });
  view.setUint8(72, 1);
  [1, 0, 0].forEach((value, index) => view.setFloat32(76 + index * 4, value, true));
  [0, 1, -1, 0, 1, -1, 0, 1, -1].forEach((value, index) => {
    view.setFloat32(88 + index * 4, value, true);
  });
  const source = {
    asset: { version: "2.0" },
    scene: 0,
    scenes: [{ nodes: [0, 1] }],
    nodes: [{ mesh: 0, weights: [0.5] }, { mesh: 0 }],
    meshes: [{
      weights: [0.25],
      primitives: [{
        attributes: { POSITION: 0, NORMAL: 1 },
        targets: [{ POSITION: 2, NORMAL: 3 }],
      }],
    }],
    buffers: [{
      byteLength: mesh.byteLength,
      uri: `data:application/octet-stream;base64,${Buffer.from(mesh).toString("base64")}`,
    }],
    bufferViews: [
      { buffer: 0, byteOffset: 0, byteLength: 36 },
      { buffer: 0, byteOffset: 36, byteLength: 36 },
      { buffer: 0, byteOffset: 72, byteLength: 1 },
      { buffer: 0, byteOffset: 76, byteLength: 12 },
      { buffer: 0, byteOffset: 88, byteLength: 36 },
    ],
    accessors: [
      { bufferView: 0, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: 1, componentType: 5126, count: 3, type: "VEC3" },
      {
        componentType: 5126, count: 3, type: "VEC3",
        sparse: { count: 1, indices: { bufferView: 2, componentType: 5121 }, values: { bufferView: 3 } },
      },
      { bufferView: 4, componentType: 5126, count: 3, type: "VEC3" },
    ],
  };
  const data = new TextEncoder().encode(JSON.stringify(source)).buffer;
  const imported = await scope.importGLTF(data, { includeDocument: true });
  assert.ok(Math.abs(imported.positions[3] - 1.5) < 1e-6);
  assert.ok(Math.abs(imported.positions[12] - 1.25) < 1e-6);
  assert.ok(Math.abs(imported.normals[1] + Math.SQRT1_2) < 1e-6);
  assert.ok(Math.abs(imported.normals[2] - Math.SQRT1_2) < 1e-6);
  assert.ok(Math.abs(imported.normals[10] + 3 / Math.sqrt(10)) < 1e-6);
  assert.ok(Math.abs(imported.normals[11] - 1 / Math.sqrt(10)) < 1e-6);
  assert.deepEqual(imported.document, source);

  const overridden = await scope.importGLTF(data, { includeDocument: true, morphWeights: [1] });
  assert.ok(Math.abs(overridden.positions[3] - 2) < 1e-6);
  assert.ok(Math.abs(overridden.positions[12] - 2) < 1e-6);
  await assert.rejects(
    scope.importGLTF(data, { morphWeights: [] }),
    (error) => error.code === "InvalidArgs" && /target count/.test(error.message),
  );

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF import samples node transforms and morph weights", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const bytes = [];
  const bufferViews = [];
  const append = (array) => {
    while (bytes.length % 4 !== 0) bytes.push(0);
    const byteOffset = bytes.length;
    bytes.push(...new Uint8Array(array.buffer, array.byteOffset, array.byteLength));
    bufferViews.push({ buffer: 0, byteOffset, byteLength: array.byteLength });
    return bufferViews.length - 1;
  };
  const positionView = append(new Float32Array([
    0, 0, 0, 1, 0, 0, 0, 1, 0,
  ]));
  const morphView = append(new Float32Array([
    1, 0, 0, 1, 0, 0, 1, 0, 0,
  ]));
  const timeView = append(new Float32Array([0, 1]));
  const translationView = append(new Float32Array([
    0, 0, 0, 10, 0, 0,
  ]));
  const halfSqrt = Math.SQRT1_2;
  const rotationView = append(new Float32Array([
    0, 0, 0, 1, 0, 0, -halfSqrt, -halfSqrt,
  ]));
  const scaleView = append(new Float32Array([
    0, 0, 0, 1, 1, 1, 1, 0, 0,
    1, 0, 0, 2, 1, 1, 0, 0, 0,
  ]));
  const weightView = append(new Float32Array([0, 1]));
  const binary = Uint8Array.from(bytes);
  const source = {
    asset: { version: "2.0" },
    scene: 0,
    scenes: [{ nodes: [0] }],
    nodes: [{ mesh: 0 }],
    meshes: [{ primitives: [{
      attributes: { POSITION: 0 },
      targets: [{ POSITION: 1 }],
    }] }],
    animations: [{
      samplers: [
        { input: 2, output: 3, interpolation: "STEP" },
        { input: 2, output: 4, interpolation: "LINEAR" },
        { input: 2, output: 5, interpolation: "CUBICSPLINE" },
        { input: 2, output: 6, interpolation: "LINEAR" },
      ],
      channels: [
        { sampler: 0, target: { node: 0, path: "translation" } },
        { sampler: 1, target: { node: 0, path: "rotation" } },
        { sampler: 2, target: { node: 0, path: "scale" } },
        { sampler: 3, target: { node: 0, path: "weights" } },
      ],
    }],
    buffers: [{
      byteLength: binary.byteLength,
      uri: `data:application/octet-stream;base64,${Buffer.from(binary).toString("base64")}`,
    }],
    bufferViews,
    accessors: [
      { bufferView: positionView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: morphView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: timeView, componentType: 5126, count: 2, type: "SCALAR" },
      { bufferView: translationView, componentType: 5126, count: 2, type: "VEC3" },
      { bufferView: rotationView, componentType: 5126, count: 2, type: "VEC4" },
      { bufferView: scaleView, componentType: 5126, count: 6, type: "VEC3" },
      { bufferView: weightView, componentType: 5126, count: 2, type: "SCALAR" },
    ],
  };
  const data = new TextEncoder().encode(JSON.stringify(source)).buffer;
  const imported = await scope.importGLTF(data, {
    includeDocument: true,
    animationIndex: 0,
    animationTime: 0.5,
  });
  assert.deepEqual(imported.document, source);
  const expected = 0.75 * Math.SQRT1_2;
  assert.ok(Math.abs(imported.positions[0] - expected) < 1e-6);
  assert.ok(Math.abs(imported.positions[1]) < 1e-6);
  assert.ok(Math.abs(imported.positions[2] - expected) < 1e-6);
  assert.ok(Math.abs(imported.positions[3] - 3 * expected) < 1e-6);
  assert.ok(Math.abs(imported.positions[5] - 3 * expected) < 1e-6);

  await assert.rejects(
    scope.importGLTF(data, { animationIndex: 0 }),
    (error) => error.code === "InvalidArgs" && /supplied together/.test(error.message),
  );
  const duplicate = structuredClone(source);
  duplicate.animations[0].channels.push(structuredClone(duplicate.animations[0].channels[0]));
  await assert.rejects(
    scope.importGLTF(new TextEncoder().encode(JSON.stringify(duplicate)).buffer, {
      animationIndex: 0,
      animationTime: 0.5,
    }),
    (error) => error.code === "ImportExportFailed" && /duplicate target/.test(error.message),
  );
  const badSampler = structuredClone(source);
  badSampler.accessors[4].count = 1;
  await assert.rejects(
    scope.importGLTF(new TextEncoder().encode(JSON.stringify(badSampler)).buffer, {
      animationIndex: 0,
      animationTime: 0.5,
    }),
    (error) => error.code === "ImportExportFailed" && /output count/.test(error.message),
  );

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF import applies static skin deformation after morph targets", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const bytes = [];
  const bufferViews = [];
  const append = (array) => {
    while (bytes.length % 4 !== 0) bytes.push(0);
    const byteOffset = bytes.length;
    bytes.push(...new Uint8Array(array.buffer, array.byteOffset, array.byteLength));
    bufferViews.push({ buffer: 0, byteOffset, byteLength: array.byteLength });
    return bufferViews.length - 1;
  };
  const positionView = append(new Float32Array([0, 0, 0, 1, 0, 0, 0, 1, 0]));
  const normalView = append(new Float32Array([0, 1, 0, 0, 1, 0, 0, 1, 0]));
  const morphPositionView = append(new Float32Array([1, 0, 0, 1, 0, 0, 1, 0, 0]));
  const morphNormalView = append(new Float32Array([1, -1, 0, 1, -1, 0, 1, -1, 0]));
  const joints8View = append(new Uint8Array([
    0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
  ]));
  const weights8View = append(new Uint8Array([
    255, 0, 0, 0, 255, 0, 0, 0, 128, 127, 0, 0,
  ]));
  const joints16View = append(new Uint16Array([
    0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
  ]));
  const weights16View = append(new Uint16Array([
    65535, 0, 0, 0, 65535, 0, 0, 0, 32768, 32767, 0, 0,
  ]));
  const weightsFloatView = append(new Float32Array([
    1, 0, 0, 0, 1, 0, 0, 0, 0.5, 0.5, 0, 0,
  ]));
  const identity = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];
  const translatedInverseBind = [...identity];
  translatedInverseBind[12] = -0.25;
  const inverseBindView = append(new Float32Array([
    ...translatedInverseBind, ...identity, ...identity,
  ]));
  const binary = Uint8Array.from(bytes);
  const commonPrimitive = {
    attributes: { POSITION: 0, NORMAL: 1 },
    targets: [{ POSITION: 2, NORMAL: 3 }],
  };
  const source = {
    asset: { version: "2.0" },
    scene: 0,
    scenes: [{ nodes: [3] }],
    nodes: [
      { mesh: 0, skin: 0, translation: [1, 0, 0], scale: [0, 0, 0] },
      { translation: [1, 2, 0], scale: [2, 1, 1] },
      { translation: [1, 0, 2] },
      { translation: [5, 0, 0], children: [0, 1, 2] },
    ],
    skins: [{ joints: [1, 2], inverseBindMatrices: 9 }],
    meshes: [{
      weights: [0.5],
      primitives: [
        { ...commonPrimitive, attributes: { ...commonPrimitive.attributes, JOINTS_0: 4, WEIGHTS_0: 5 } },
        { ...commonPrimitive, attributes: { ...commonPrimitive.attributes, JOINTS_0: 6, WEIGHTS_0: 7 } },
        { ...commonPrimitive, attributes: { ...commonPrimitive.attributes, JOINTS_0: 4, WEIGHTS_0: 8 } },
      ],
    }],
    buffers: [{
      byteLength: binary.byteLength,
      uri: `data:application/octet-stream;base64,${Buffer.from(binary).toString("base64")}`,
    }],
    bufferViews,
    accessors: [
      { bufferView: positionView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: normalView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: morphPositionView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: morphNormalView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: joints8View, componentType: 5121, count: 3, type: "VEC4" },
      { bufferView: weights8View, componentType: 5121, normalized: true, count: 3, type: "VEC4" },
      { bufferView: joints16View, componentType: 5123, count: 3, type: "VEC4" },
      { bufferView: weights16View, componentType: 5123, normalized: true, count: 3, type: "VEC4" },
      { bufferView: weightsFloatView, componentType: 5126, count: 3, type: "VEC4" },
      { bufferView: inverseBindView, componentType: 5126, count: 3, type: "MAT4" },
    ],
  };
  const data = new TextEncoder().encode(JSON.stringify(source)).buffer;
  const imported = await scope.importGLTF(data, { includeDocument: true });
  assert.deepEqual(imported.document, source);
  assert.equal(imported.primitives.length, 3);
  for (const vertexStart of [0, 3, 6]) {
    const offset = vertexStart * 3;
    assert.ok(Math.abs(imported.positions[offset] - 6.5) < 1e-6);
    assert.ok(Math.abs(imported.positions[offset + 2] - 2) < 1e-6);
    assert.ok(Math.abs(imported.positions[offset + 3] - 7.5) < 1e-6);
    assert.ok(Math.abs(imported.positions[offset + 4] + 2) < 1e-6);
    assert.ok(Math.abs(imported.normals[offset] - 1 / Math.sqrt(5)) < 1e-6);
    assert.ok(Math.abs(imported.normals[offset + 2] - 2 / Math.sqrt(5)) < 1e-6);
  }
  assert.ok(Math.abs(imported.positions[6] - 6.5) < 1e-6);
  assert.ok(Math.abs(imported.positions[7] + 2 * 127 / 255) < 1e-6);
  assert.ok(Math.abs(imported.positions[8] - (1 + 2 * 128 / 255)) < 1e-6);
  assert.ok(Math.abs(imported.positions[15] - 6.5) < 1e-6);
  assert.ok(Math.abs(imported.positions[16] + 2 * 32767 / 65535) < 1e-6);
  assert.ok(Math.abs(imported.positions[17] - (1 + 2 * 32768 / 65535)) < 1e-6);
  assert.ok(Math.abs(imported.positions[24] - 6.5) < 1e-6);
  assert.ok(Math.abs(imported.positions[25] + 1) < 1e-6);
  assert.ok(Math.abs(imported.positions[26] - 2) < 1e-6);

  const malformed = structuredClone(source);
  delete malformed.meshes[0].primitives[0].attributes.WEIGHTS_0;
  await assert.rejects(
    scope.importGLTF(new TextEncoder().encode(JSON.stringify(malformed)).buffer),
    (error) => error.code === "ImportExportFailed" && /JOINTS_0 and WEIGHTS_0/.test(error.message),
  );

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF document import accepts non-mesh documents and validates required extensions", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const cameraOnly = {
    asset: { version: "2.0" },
    scene: 0,
    scenes: [{ nodes: [0] }],
    nodes: [{ camera: 0 }],
    cameras: [{ type: "orthographic", orthographic: { xmag: 1, ymag: 1, znear: 0, zfar: 10 } }],
    images: [{ uri: "data:application/octet-stream,%41%42" }],
  };
  const imported = await scope.importGLTF(
    new TextEncoder().encode(JSON.stringify(cameraOnly)).buffer,
    { includeDocument: true },
  );
  assert.equal(imported.shape, undefined);
  assert.equal(imported.positions.length, 0);
  assert.equal(imported.indices.length, 0);
  assert.deepEqual([...new Uint8Array(imported.resources[0].data)], [65, 66]);
  await assert.rejects(
    scope.importGLTF(new TextEncoder().encode(JSON.stringify({
      asset: { version: "2.0" },
      extensionsRequired: ["KHR_draco_mesh_compression"],
    })).buffer, { includeDocument: true }),
    (error) => error.code === "ImportExportFailed" && /KHR_draco_mesh_compression/.test(error.message),
  );
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveBufferBytes, 0);
});

test("importGLTF rejects malformed payloads", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  await assert.rejects(
    scope.importGLTF(new TextEncoder().encode('{"asset":{"version":"1.0"}}').buffer),
    (error) => error.code === "ImportExportFailed",
  );
  await scope.end();
});

test("VRML 2.0 memory streams round-trip through OCCT", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const vrml = await client.exportVRML(box, { version: 2, linearDeflection: 0.05 });
  assert.match(new TextDecoder().decode(vrml.slice(0, 32)), /^#VRML V2\.0/);
  assert.deepEqual(await client.probeFormat(vrml), {
    format: "vrml", encoding: "text", confidence: "exact",
  });
  const restored = await scope.importVRML(vrml);
  const bounds = await client.bbox(restored);
  assert.ok(bounds.min.every((value) => Math.abs(value) < 1e-6));
  assert.ok(bounds.max.every((value, index) => Math.abs(value - [2, 3, 4][index]) < 1e-6));
  assert.equal(await client.isValid(restored), true);
  const restoredMesh = await scope.importVRML(vrml, { includeMesh: true });
  assert.ok(restoredMesh.positions.length > 0);
  assert.equal(restoredMesh.positions.length, restoredMesh.normals.length);
  assert.equal(restoredMesh.indices.length % 3, 0);
  assert.ok(restoredMesh.indices.length > 0);
  const meshVRML = await client.exportVRML({
    positions: restoredMesh.positions,
    indices: restoredMesh.indices,
    normals: restoredMesh.normals,
    ...(restoredMesh.uvs === undefined ? {} : { uvs: restoredMesh.uvs }),
  }, { version: 2 });
  assert.match(new TextDecoder().decode(meshVRML.slice(0, 32)), /^#VRML V2\.0/);
  const meshRoundTrip = await scope.importVRML(meshVRML, { includeMesh: true });
  assert.equal(meshRoundTrip.indices.length, restoredMesh.indices.length);
  assert.equal(meshRoundTrip.positions.length, restoredMesh.positions.length);
  const attributedVRML = await client.exportVRML({
    positions: new Float32Array([0, 0, 0, 1, 0, 0, 0, 1, 0]),
    indices: new Uint32Array([0, 1, 2]),
    normals: new Float32Array([0, 0, 1, 0, 0, 1, 0, 0, 1]),
    uvs: new Float32Array([0, 0, 1, 0, 0, 1]),
    colors: new Uint8Array([255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255]),
  }, { version: 2 });
  const attributedRoundTrip = await scope.importVRML(attributedVRML, { includeMesh: true });
  assert.deepEqual(Array.from(attributedRoundTrip.positions), [0, 0, 0, 1, 0, 0, 0, 1, 0]);
  assert.deepEqual(Array.from(attributedRoundTrip.indices), [0, 1, 2]);
  assert.deepEqual(Array.from(attributedRoundTrip.normals), [0, 0, 1, 0, 0, 1, 0, 0, 1]);
  assert.deepEqual(Array.from(attributedRoundTrip.uvs), [0, 0, 1, 0, 0, 1]);
  assert.deepEqual(Array.from(attributedRoundTrip.colors),
    [255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255]);
  await assert.rejects(
    client.exportVRML({
      positions: new Float32Array([0, 0, 0, 1, 0, 0, 0, 1, 0]),
      indices: new Uint32Array([0, 1, 2]),
      colors: new Uint8Array([255, 0, 0, 128, 0, 255, 0, 255, 0, 0, 255, 255]),
    }),
    (error) => error.code === "InvalidArgs" && /opaque alpha/.test(error.message),
  );
  const independentlyIndexedVRML = new TextEncoder().encode(`#VRML V2.0 utf8
Shape { geometry IndexedFaceSet {
 coord Coordinate { point [ 0 0 0, 1 0 0, 1 0 -1, 0 0 -1 ] }
 coordIndex [ 0 1 2 3 -1 ]
 texCoord TextureCoordinate { point [ 0 0, 1 0, 1 1, 0 1 ] }
 texCoordIndex [ 3 2 1 0 -1 ]
 solid FALSE
} }
`).buffer;
  const independentlyIndexed = await scope.importVRML(independentlyIndexedVRML, { includeMesh: true });
  assert.deepEqual(Array.from(independentlyIndexed.indices), [0, 1, 2, 3, 4, 5]);
  assert.deepEqual(Array.from(independentlyIndexed.uvs), [0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0]);
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("VRML export reports OCCT progress", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const boxes = [];
  for (let index = 0; index < 24; index++) {
    boxes.push(await scope.makeBox([10, 20, 30], [index * 12, 0, 0]));
  }
  const shape = await scope.makeCompound(boxes);
  const exportProgress = [];
  const exported = await client.request("exportVRML", { shape, version: 2 }, {
    onProgress: (event) => exportProgress.push(event.fraction),
  });
  assert.equal(exportProgress[0], 0);
  assert.equal(exportProgress.at(-1), 1);
  assert.ok(exportProgress.some((fraction) => fraction > 0 && fraction < 1));

  const exportController = new AbortController();
  const cancellationProgress = [];
  await assert.rejects(
    client.request("exportVRML", { shape, version: 2, linearDeflection: 0.01 }, {
      signal: exportController.signal,
      onProgress: (event) => {
        cancellationProgress.push(event.fraction);
        if (event.fraction > 0 && event.fraction < 1)
          exportController.abort(new Error("VRML export cancelled"));
      },
    }),
    /VRML export cancelled/,
  );
  assert.ok(cancellationProgress.some((fraction) => fraction > 0 && fraction < 1));

  const importProgress = [];
  await client.request("importVRML", {
    scopeId: scope.scopeId,
    data: { $inputBuffer: exported.data.data.slice(0) },
  }, { onProgress: (event) => importProgress.push(event.fraction) });
  assert.equal(importProgress[0], 0);
  assert.equal(importProgress.at(-1), 1);

  await scope.end();
});

test("importVRML rejects malformed payloads", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  await assert.rejects(
    scope.importVRML(new TextEncoder().encode("not vrml").buffer),
    (error) => error.code === "ImportExportFailed",
  );
  await scope.end();
});

