import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const near = (actual, expected, tolerance = 1e-6) => {
  assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected}`);
};

test("minimum distance exposes all topology supports, parameters, and inner state", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();

  const point = await scope.makeVertex([5, 3, 0]);
  const edge = await scope.makeEdgeLine([0, 0, 0], [10, 0, 0]);
  const edgeDistance = await client.distance(point, edge);
  near(edgeDistance.distance, 3);
  assert.equal(edgeDistance.innerSolution, false);
  const edgeSolution = edgeDistance.solutions.find(
    ({ supportOnFirst, supportOnSecond }) => supportOnFirst.type === "vertex" && supportOnSecond.type === "edge",
  );
  assert.ok(edgeSolution);
  assert.equal(edgeSolution.supportOnFirst.index, 0);
  assert.equal(edgeSolution.supportOnSecond.index, 0);
  const edgeEvaluation = await client.evaluateCurve(edge, edgeSolution.supportOnSecond.parameter);
  near(edgeEvaluation.point[0], edgeSolution.pointOnSecond[0]);
  near(edgeEvaluation.point[1], edgeSolution.pointOnSecond[1]);
  near(edgeEvaluation.point[2], edgeSolution.pointOnSecond[2]);

  const wire = await scope.makePolygon([[0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0]]);
  const face = await scope.makeFace(wire);
  const above = await scope.makeVertex([2, 3, 4]);
  const faceDistance = await client.distance(above, face);
  const faceSolution = faceDistance.solutions.find(({ supportOnSecond }) => supportOnSecond.type === "face");
  assert.ok(faceSolution);
  assert.equal(faceSolution.supportOnSecond.index, 0);
  const surfaceEvaluation = await client.evaluateSurface(
    face,
    faceSolution.supportOnSecond.u,
    faceSolution.supportOnSecond.v,
  );
  near(surfaceEvaluation.point[0], faceSolution.pointOnSecond[0]);
  near(surfaceEvaluation.point[1], faceSolution.pointOnSecond[1]);
  near(surfaceEvaluation.point[2], faceSolution.pointOnSecond[2]);

  const box = await scope.makeBox([10, 10, 10]);
  const inside = await scope.makeVertex([5, 5, 5]);
  const innerDistance = await client.distance(box, inside);
  near(innerDistance.distance, 0);
  assert.equal(innerDistance.innerSolution, true);
  assert.ok(innerDistance.solutions.length >= 1);

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
