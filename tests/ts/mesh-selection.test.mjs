import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient, EdgeSelectionMap, FaceSelectionMap } from "../../dist/index.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("release wasm tessellation maps every triangle and polyline vertex to OCCT topology", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const counts = await client.topologyCounts(box);

  const mesh = await client.tessellate(box);
  const faces = new FaceSelectionMap(mesh);
  assert.equal(faces.faceIndices.length, counts.face);
  for (let triangle = 0; triangle < mesh.indices.length / 3; triangle++) {
    const faceIndex = faces.faceAtTriangle(triangle);
    assert.ok(faceIndex !== undefined && faceIndex < counts.face);
  }
  for (const faceIndex of faces.faceIndices) {
    assert.ok(faces.rangesForFace(faceIndex).some((range) => range.triangleCount > 0));
  }

  const edgeMesh = await client.tessellateEdges(box);
  const edges = new EdgeSelectionMap(edgeMesh);
  assert.equal(edges.edgeIndices.length, counts.edge);
  for (let vertex = 0; vertex < edgeMesh.positions.length / 3; vertex++) {
    const edgeIndex = edges.edgeAtVertex(vertex);
    assert.ok(edgeIndex !== undefined && edgeIndex < counts.edge);
  }
  for (const edgeIndex of edges.edgeIndices) {
    assert.ok(edges.rangesForEdge(edgeIndex).some((range) => range.vertexCount >= 2));
  }

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
