import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import draco3dgltf from "draco3dgltf";
import { MeshoptEncoder } from "meshoptimizer/encoder";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const positions = new Float32Array([0, 0, 0, 2, 0, 0, 0, 3, 0]);
const indices = new Uint16Array([0, 1, 2]);
const dataUri = (value) => `data:application/octet-stream;base64,${Buffer.from(value).toString("base64")}`;

test("glTF import decodes EXT_meshopt_compression geometry", async () => {
  await MeshoptEncoder.ready;
  const encodedPositions = MeshoptEncoder.encodeGltfBuffer(
    new Uint8Array(positions.buffer), positions.length / 3, 12, "ATTRIBUTES",
  );
  const encodedIndices = MeshoptEncoder.encodeGltfBuffer(
    new Uint8Array(indices.buffer), indices.length, 2, "TRIANGLES",
  );
  const compressed = Buffer.concat([encodedPositions, encodedIndices]);
  const source = {
    asset: { version: "2.0" },
    extensionsUsed: ["EXT_meshopt_compression"],
    extensionsRequired: ["EXT_meshopt_compression"],
    scene: 0,
    scenes: [{ nodes: [0] }],
    nodes: [{ mesh: 0 }],
    meshes: [{ primitives: [{ attributes: { POSITION: 0 }, indices: 1 }] }],
    buffers: [{ byteLength: compressed.byteLength, uri: dataUri(compressed) }],
    bufferViews: [
      {
        buffer: 0,
        byteLength: positions.byteLength,
        byteStride: 12,
        extensions: {
          EXT_meshopt_compression: {
            buffer: 0,
            byteOffset: 0,
            byteLength: encodedPositions.byteLength,
            byteStride: 12,
            count: 3,
            mode: "ATTRIBUTES",
            filter: "NONE",
          },
        },
      },
      {
        buffer: 0,
        byteLength: indices.byteLength,
        extensions: {
          EXT_meshopt_compression: {
            buffer: 0,
            byteOffset: encodedPositions.byteLength,
            byteLength: encodedIndices.byteLength,
            byteStride: 2,
            count: 3,
            mode: "TRIANGLES",
            filter: "NONE",
          },
        },
      },
    ],
    accessors: [
      { bufferView: 0, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: 1, componentType: 5123, count: 3, type: "SCALAR" },
    ],
  };
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const imported = await scope.importGLTF(
    new TextEncoder().encode(JSON.stringify(source)).buffer,
    { includeDocument: true },
  );
  assert.deepEqual(imported.document, source);
  assert.deepEqual([...imported.positions].map((value) => Math.abs(value)), [0, 0, 0, 2, 0, 0, 0, 0, 3]);
  assert.deepEqual([...imported.indices], [0, 1, 2]);
  assert.equal((await client.topologyCounts(imported.shape)).face, 1);
  await scope.end();
});

test("glTF import decodes KHR_draco_mesh_compression geometry", async () => {
  const module = await draco3dgltf.createEncoderModule({});
  const builder = new module.MeshBuilder();
  const mesh = new module.Mesh();
  const encoder = new module.Encoder();
  const output = new module.DracoInt8Array();
  let encoded;
  let positionId;
  try {
    builder.AddFacesToMesh(mesh, 1, new Uint32Array(indices));
    positionId = builder.AddFloatAttributeToMesh(mesh, module.POSITION, 3, 3, positions);
    const length = encoder.EncodeMeshToDracoBuffer(mesh, output);
    assert.ok(length > 0);
    encoded = Uint8Array.from({ length }, (_, index) => output.GetValue(index));
  } finally {
    module.destroy(output);
    module.destroy(encoder);
    module.destroy(mesh);
    module.destroy(builder);
  }
  const source = {
    asset: { version: "2.0" },
    extensionsUsed: ["KHR_draco_mesh_compression"],
    extensionsRequired: ["KHR_draco_mesh_compression"],
    scene: 0,
    scenes: [{ nodes: [0] }],
    nodes: [{ mesh: 0 }],
    meshes: [{ primitives: [{
      attributes: { POSITION: 0 },
      indices: 1,
      extensions: {
        KHR_draco_mesh_compression: { bufferView: 0, attributes: { POSITION: positionId } },
      },
    }] }],
    buffers: [{ byteLength: encoded.byteLength, uri: dataUri(encoded) }],
    bufferViews: [{ buffer: 0, byteOffset: 0, byteLength: encoded.byteLength }],
    accessors: [
      { componentType: 5126, count: 3, type: "VEC3" },
      { componentType: 5125, count: 3, type: "SCALAR" },
    ],
  };
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const imported = await scope.importGLTF(
    new TextEncoder().encode(JSON.stringify(source)).buffer,
    { includeDocument: true },
  );
  assert.deepEqual(imported.document, source);
  assert.deepEqual([...imported.positions].map((value) => Math.abs(value)), [0, 0, 0, 2, 0, 0, 0, 0, 3]);
  assert.deepEqual([...imported.indices], [0, 1, 2]);
  assert.equal((await client.topologyCounts(imported.shape)).face, 1);
  await scope.end();
});
