import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("OBJ and ASCII PLY memory-stream round trips preserve triangulated faces", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);

  const obj = await client.exportOBJ(box, { linearDeflection: 0.05 });
  assert.equal((await client.probeFormat(obj)).format, "obj");
  const objText = new TextDecoder().decode(obj);
  assert.match(objText, /^vt /m);
  assert.match(objText, /^vn /m);
  const objShape = await scope.importOBJ(obj);
  const objCounts = await client.topologyCounts(objShape);
  assert.ok(obj.byteLength > 0);
  assert.ok(objCounts.face > 0);
  assert.equal(await client.isValid(objShape), true);

  const attributeObj = new TextEncoder().encode(
    "v 0 0 0\nv 1 0 0\nv 0 1 0\n"
      + "vt 0 0\nvt 1 0\nvt 0 1\n"
      + "vn 0 0 1\n"
      + "f 1/1/1 2/2/1 3/3/1\n",
  );
  const attributeShape = await scope.importOBJ(attributeObj.buffer);
  const attributeFace = await scope.getSubShape(attributeShape, "face", 0);
  const attributeMesh = await client.triangulationData(attributeFace, { includeUV: true });
  assert.equal(attributeMesh.uvs.length, 6);
  assert.deepEqual(Array.from(attributeMesh.normals.slice(0, 3)), [0, 0, 1]);
  assert.deepEqual(Array.from(attributeMesh.uvs), [0, 0, 1, 0, 0, 1]);

  const ply = await client.exportPLY(box, { linearDeflection: 0.05 });
  assert.deepEqual(await client.probeFormat(ply), {
    format: "ply", encoding: "text", confidence: "exact",
  });
  const plyText = new TextDecoder().decode(ply);
  assert.match(plyText, /property double nx/);
  assert.match(plyText, /property double s/);
  const plyShape = await scope.importPLY(ply);
  const plyCounts = await client.topologyCounts(plyShape);
  assert.ok(ply.byteLength > 0);
  assert.ok(plyCounts.face > 0);
  assert.equal(await client.isValid(plyShape), true);
  const plyFace = await scope.getSubShape(plyShape, "face", 0);
  const plyAttributes = await client.triangulationData(plyFace, { includeUV: true });
  assert.equal(plyAttributes.normals.length, plyAttributes.positions.length);
  assert.equal(plyAttributes.uvs.length, plyAttributes.positions.length / 3 * 2);

  const binaryPly = await client.exportPLY(box, {
    linearDeflection: 0.05,
    encoding: "binary_little_endian",
  });
  assert.deepEqual(await client.probeFormat(binaryPly), {
    format: "ply", encoding: "binary", confidence: "exact",
  });
  const binaryShape = await scope.importPLY(binaryPly);
  assert.ok((await client.topologyCounts(binaryShape)).face > 0);
  assert.equal(await client.isValid(binaryShape), true);
  const binaryFace = await scope.getSubShape(binaryShape, "face", 0);
  const binaryAttributes = await client.triangulationData(binaryFace, { includeUV: true });
  assert.equal(binaryAttributes.normals.length, binaryAttributes.positions.length);
  assert.equal(binaryAttributes.uvs.length, binaryAttributes.positions.length / 3 * 2);

  const binaryBigEndianPly = await client.exportPLY(box, {
    linearDeflection: 0.05,
    encoding: "binary_big_endian",
  });
  assert.deepEqual(await client.probeFormat(binaryBigEndianPly), {
    format: "ply", encoding: "binary", confidence: "exact",
  });
  const binaryBigEndianShape = await scope.importPLY(binaryBigEndianPly);
  assert.ok((await client.topologyCounts(binaryBigEndianShape)).face > 0);
  assert.equal(await client.isValid(binaryBigEndianShape), true);

  const coloredPly = new TextEncoder().encode(
    "ply\nformat ascii 1.0\n"
      + "comment source application\ncomment calibrated mesh\n"
      + "obj_info coordinate_frame machine\nobj_info units millimeter\n"
      + "element vertex 3\n"
      + "property float x\nproperty float y\nproperty float z\n"
      + "property uchar red\nproperty uchar green\nproperty uchar blue\nproperty uchar alpha\n"
      + "element face 1\nproperty list uchar uint vertex_indices\nend_header\n"
      + "0 0 0 255 0 0 255\n1 0 0 0 255 0 255\n0 1 0 0 0 255 128\n"
      + "3 0 1 2\n",
  );
  const coloredMesh = await scope.importPLY(coloredPly.buffer, { includeMesh: true });
  assert.deepEqual(coloredMesh.document, {
    comments: ["source application", "calibrated mesh"],
    objectInfo: ["coordinate_frame machine", "units millimeter"],
  });
  assert.deepEqual(Array.from(coloredMesh.colors), [255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 128]);
  assert.deepEqual(Array.from(coloredMesh.indices), [0, 1, 2]);
  const coloredAscii = await client.exportPLY(coloredMesh, { encoding: "ascii" });
  const coloredAsciiText = new TextDecoder().decode(coloredAscii);
  assert.match(coloredAsciiText, /property uchar red/);
  assert.match(coloredAsciiText, /255 0 0 255/);
  assert.match(coloredAsciiText, /comment source application\ncomment calibrated mesh\n/);
  assert.match(coloredAsciiText, /obj_info coordinate_frame machine\nobj_info units millimeter\n/);
  const coloredAsciiRoundTrip = await scope.importPLY(coloredAscii, { includeMesh: true });
  assert.deepEqual(coloredAsciiRoundTrip.document, coloredMesh.document);
  const coloredBinary = await client.exportPLY(coloredMesh, { encoding: "binary_little_endian" });
  const coloredRoundTrip = await scope.importPLY(coloredBinary, { includeMesh: true });
  assert.deepEqual(coloredRoundTrip.document, coloredMesh.document);
  assert.deepEqual(Array.from(coloredRoundTrip.colors), Array.from(coloredMesh.colors));
  assert.deepEqual(Array.from(coloredRoundTrip.indices), [0, 1, 2]);
  const coloredBigEndian = await client.exportPLY(coloredMesh, { encoding: "binary_big_endian" });
  const coloredBigEndianRoundTrip = await scope.importPLY(coloredBigEndian, { includeMesh: true });
  assert.deepEqual(coloredBigEndianRoundTrip.document, coloredMesh.document);
  assert.deepEqual(Array.from(coloredBigEndianRoundTrip.colors), Array.from(coloredMesh.colors));
  assert.deepEqual(Array.from(coloredBigEndianRoundTrip.indices), [0, 1, 2]);

  const bigHeader = new TextEncoder().encode(
    "ply\nformat binary_big_endian 1.0\nelement vertex 3\n"
      + "property float x\nproperty float y\nproperty float z\n"
      + "element face 1\nproperty list uchar uint vertex_indices\nend_header\n",
  );
  const bigBody = new Uint8Array(3 * 12 + 1 + 3 * 4);
  const bigView = new DataView(bigBody.buffer);
  [[0, 0, 0], [2, 0, 0], [0, 2, 0]].forEach((point, vertex) => {
    point.forEach((value, coordinate) => bigView.setFloat32(vertex * 12 + coordinate * 4, value, false));
  });
  bigBody[36] = 3;
  bigView.setUint32(37, 0, false);
  bigView.setUint32(41, 1, false);
  bigView.setUint32(45, 2, false);
  const bigPly = new Uint8Array(bigHeader.byteLength + bigBody.byteLength);
  bigPly.set(bigHeader);
  bigPly.set(bigBody, bigHeader.byteLength);
  const bigShape = await scope.importPLY(bigPly.buffer);
  assert.equal((await client.topologyCounts(bigShape)).face, 1);
  assert.equal(await client.isValid(bigShape), true);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("OBJ document import preserves primitive state, mesh buffers, and companion MTL data", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const obj = new TextEncoder().encode(
    "mtllib materials.mtl\n"
      + "o first\n"
      + "g body front\n"
      + "usemtl red\n"
      + "s 1\n"
      + "v 0 0 0\n"
      + "v 1 0 0\n"
      + "v 0 1 0\n"
      + "v 0 0 1\n"
      + "vt 0 0\n"
      + "vt 1 0\n"
      + "vt 0 1\n"
      + "vn 0 0 1\n"
      + "f 1/1/1 2/2/1 3/3/1 # first triangle\n"
      + "o second\n"
      + "g cap\n"
      + "usemtl blue\n"
      + "s off\n"
      + "f 1/1/1 3/3/1 4/2/1\n",
  );
  const mtl = new TextEncoder().encode(
    "newmtl red\n"
      + "Ka 0.1 0.2 0.3\n"
      + "Kd 1 0 0\n"
      + "Ks 0.4 0.5 0.6\n"
      + "Ke 0.01 0.02 0.03\n"
      + "d 0.8\n"
      + "Ns 32\n"
      + "Ni 1.5\n"
      + "illum 2\n"
      + "map_Kd -clamp on red-diffuse.png\n"
      + "map_Ks red-specular.png\n"
      + "map_d red-opacity.png\n"
      + "bump -bm 0.5 red-normal.png\n"
      + "newmtl blue\n"
      + "Kd 0 0 1\n"
      + "Tr 0.25\n",
  );

  const imported = await scope.importOBJ(obj.buffer, {
    includeDocument: true,
    resources: { "materials.mtl": mtl.buffer },
  });
  assert.equal(await client.isValid(imported.shape), true);
  assert.equal((await client.topologyCounts(imported.shape)).face, 2);
  assert.deepEqual(Array.from(imported.positions), [
    0, 0, 0,
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
  ]);
  assert.deepEqual(Array.from(imported.indices), [0, 1, 2, 0, 2, 3]);
  assert.deepEqual(Array.from(imported.normals), [
    0, 0, 1,
    0, 0, 1,
    0, 0, 1,
    0, 0, 1,
  ]);
  assert.deepEqual(Array.from(imported.uvs), [0, 0, 1, 0, 0, 1, 1, 0]);
  assert.deepEqual(imported.document.materialLibraries, ["materials.mtl"]);
  assert.deepEqual(imported.document.primitives, [
    {
      indexStart: 0,
      indexCount: 3,
      object: "first",
      groups: ["body", "front"],
      material: "red",
      smoothingGroup: "1",
    },
    {
      indexStart: 3,
      indexCount: 3,
      object: "second",
      groups: ["cap"],
      material: "blue",
      smoothingGroup: "off",
    },
  ]);
  assert.deepEqual(imported.document.materials, [
    {
      name: "red",
      ambient: [0.1, 0.2, 0.3],
      diffuse: [1, 0, 0],
      specular: [0.4, 0.5, 0.6],
      emissive: [0.01, 0.02, 0.03],
      opacity: 0.8,
      shininess: 32,
      opticalDensity: 1.5,
      illuminationModel: 2,
      diffuseMap: "red-diffuse.png",
      specularMap: "red-specular.png",
      opacityMap: "red-opacity.png",
      bumpMap: "red-normal.png",
    },
    { name: "blue", diffuse: [0, 0, 1], opacity: 0.75 },
  ]);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("OBJ document export round-trips primitive and MTL metadata", async () => {
  const client = await DirectClient.create(wasm);
  const mesh = {
    positions: new Float32Array([
      0, 0, 0,
      1, 0, 0,
      0, 1, 0,
      0, 0, 1,
    ]),
    indices: new Uint32Array([0, 1, 2, 0, 2, 3]),
    normals: new Float32Array([
      0, 0, 1,
      0, 0, 1,
      0, 0, 1,
      0, 0, 1,
    ]),
    uvs: new Float32Array([0, 0, 1, 0, 0, 1, 1, 0]),
    document: {
      materialLibraries: ["source.mtl"],
      primitives: [
        { indexStart: 0, indexCount: 3, object: "body", groups: ["front"], material: "red", smoothingGroup: "1" },
        { indexStart: 3, indexCount: 3, object: "cap", groups: ["back"], material: "blue", smoothingGroup: "off" },
      ],
      materials: [
        { name: "red", ambient: [0.1, 0.2, 0.3], diffuse: [1, 0, 0], shininess: 16, diffuseMap: "red.png" },
        { name: "blue", diffuse: [0, 0, 1], opacity: 0.5, illuminationModel: 2 },
      ],
    },
  };

  const exported = await client.exportOBJ(mesh, { materialLibrary: "scene.mtl" });
  assert.deepEqual(Object.keys(exported.resources), ["scene.mtl"]);
  const objText = new TextDecoder().decode(exported.data);
  const mtlText = new TextDecoder().decode(exported.resources["scene.mtl"]);
  assert.match(objText, /^# occt-worker OBJ\nmtllib scene\.mtl\n/);
  assert.match(objText, /o body\ng front\nusemtl red\ns 1\nf 1\/1\/1 2\/2\/2 3\/3\/3\n/);
  assert.match(objText, /o cap\ng back\nusemtl blue\ns off\nf 1\/1\/1 3\/3\/3 4\/4\/4\n/);
  assert.match(mtlText, /newmtl red\nKa \S+ \S+ \S+\nKd 1 0 0\nNs 16\nmap_Kd red\.png\n/);

  const bare = await client.exportOBJ({ positions: mesh.positions, indices: mesh.indices });
  assert.deepEqual(bare.resources, {});
  assert.doesNotMatch(new TextDecoder().decode(bare.data), /^mtllib /m);
  await assert.rejects(
    client.exportOBJ({
      ...mesh,
      document: {
        ...mesh.document,
        primitives: [{ indexStart: 3, indexCount: 3, groups: [] }],
      },
    }),
    (error) => error.code === "InvalidArgs" && /partition/.test(error.message),
  );

  const scope = await client.beginScope();
  const imported = await scope.importOBJ(exported.data, {
    includeDocument: true,
    resources: exported.resources,
  });
  assert.deepEqual(imported.document.primitives, mesh.document.primitives);
  assert.deepEqual(imported.document.materials, mesh.document.materials);
  assert.deepEqual(imported.document.materialLibraries, ["scene.mtl"]);
  assert.deepEqual(Array.from(imported.indices), Array.from(mesh.indices));
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("OBJ and PLY imports reject malformed mesh payloads", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  await assert.rejects(
    scope.importOBJ(new TextEncoder().encode("v 0 0 0\nf 1 2 3\n").buffer),
    (error) => error.code === "ImportExportFailed",
  );
  await assert.rejects(
    scope.importPLY(new TextEncoder().encode("ply\nformat ascii 1.0\nend_header\n").buffer),
    (error) => error.code === "ImportExportFailed",
  );
  await scope.end();
});

