import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("triangulationData reads existing OCCT mesh and replaceTriangulation returns an independent face", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const support = await scope.makeSurfaceFace(
    { type: "plane" },
    { uFirst: 0, uLast: 10, vFirst: 0, vLast: 10 },
  );
  await client.tessellate(support);
  const existing = await client.triangulationData(support);
  assert.ok(existing.positions.length > 0);
  assert.ok(existing.indices.length > 0);

  const replaced = await scope.replaceTriangulation(support, {
    positions: [[0, 0, 0], [10, 0, 0], [0, 10, 0]],
    indices: [0, 1, 2],
    normals: [[0, 0, 1], [0, 0, 1], [0, 0, 1]],
    uvs: [0, 0, 1, 0, 0, 1],
  });
  const direct = await client.triangulationData(replaced, { includeUV: true });
  assert.equal(direct.positions.length, 9);
  assert.deepEqual([...direct.indices], [0, 1, 2]);
  assert.equal(direct.uvs?.length, 6);
  assert.equal(await client.isValid(replaced), true);
  assert.equal((await client.triangulationData(support)).indices.length, existing.indices.length);
  await scope.end();
});

test("validateTriangulation reports missing mesh and repair rebuilds an independent copy", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const support = await scope.makeSurfaceFace(
    { type: "plane" },
    { uFirst: 0, uLast: 10, vFirst: 0, vLast: 10 },
  );
  const before = await client.validateTriangulation(support);
  assert.equal(before.valid, false);
  assert.equal(before.faces, 1);
  assert.equal(before.triangulatedFaces, 0);
  assert.deepEqual(before.issues.map(({ code }) => code), ["missingTriangulation"]);

  const repaired = await scope.repairTriangulation(support);
  assert.equal(repaired.faces, 1);
  assert.equal(repaired.triangulatedFaces, 1);
  assert.ok(repaired.triangles > 0);
  assert.equal((await client.validateTriangulation(repaired.shape)).valid, true);
  assert.equal((await client.validateTriangulation(support)).valid, false);
  await scope.end();
});

test("validateTriangulation reports missing normals and degenerate triangles", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const support = await scope.makeSurfaceFace(
    { type: "plane" },
    { uFirst: 0, uLast: 10, vFirst: 0, vLast: 10 },
  );
  const replaced = await scope.replaceTriangulation(support, {
    positions: [[0, 0, 0], [5, 0, 0], [10, 0, 0]],
    indices: [0, 1, 2],
  });
  const result = await client.validateTriangulation(replaced, { checkUV: false });
  assert.equal(result.valid, false);
  assert.deepEqual(result.issues.map(({ code }) => code), ["missingNormals", "degenerateTriangle"]);
  await scope.end();
});
