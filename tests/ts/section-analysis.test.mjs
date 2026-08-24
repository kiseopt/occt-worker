import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const near = (actual, expected, tolerance = 1e-5) => {
  assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected}`);
};

test("sectionAnalysis maps section edges to both ancestor faces and isolates point contacts", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();

  const horizontalWire = await scope.makePolygon([[0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0]]);
  const verticalWire = await scope.makePolygon([[5, 0, -5], [5, 10, -5], [5, 10, 5], [5, 0, 5]]);
  const horizontal = await scope.makeFace(horizontalWire);
  const vertical = await scope.makeFace(verticalWire);
  const analysis = await scope.sectionAnalysis(horizontal, vertical, {
    approximation: true,
    computePCurveOnFirst: true,
    computePCurveOnSecond: true,
  });
  assert.equal(await client.isValid(analysis.shape), true);
  assert.equal(analysis.edges.length, 1);
  assert.equal(analysis.edges[0].index, 0);
  assert.equal(analysis.edges[0].firstFaceIndex, 0);
  assert.equal(analysis.edges[0].secondFaceIndex, 0);
  near(analysis.edges[0].length, 10);
  assert.deepEqual(analysis.standaloneVertices, []);

  const firstEdge = await scope.makeEdgeLine([0, 0, 0], [10, 0, 0]);
  const secondEdge = await scope.makeEdgeLine([5, -5, 0], [5, 5, 0]);
  const pointContact = await scope.sectionAnalysis(firstEdge, secondEdge);
  assert.deepEqual(pointContact.edges, []);
  assert.deepEqual(pointContact.standaloneVertices, [0]);
  assert.equal((await client.topologyCounts(pointContact.shape)).vertex, 1);

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
