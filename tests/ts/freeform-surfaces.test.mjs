import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const near = (actual, expected, tolerance = 1e-8) => {
  assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected}`);
};

const bezierPoles = [
  [[0, 0, 0], [0, 1, 0], [0, 2, 0]],
  [[1, 0, 0], [1, 1, 0], [1, 2, 0]],
  [[2, 0, 0], [2, 1, 0], [2, 2, 0]],
];

test("Bezier surface control data, pole editing, and finite trimming are scoped and independent", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const weights = bezierPoles.map((row) => row.map(() => 1));
  const surface = await scope.makeSurfaceBezier(bezierPoles, { weights });
  assert.equal(await client.shapeType(surface), "face");
  assert.equal(await client.isValid(surface), true);

  const data = await client.surfaceControlData(surface);
  assert.equal(data.type, "bezier");
  assert.equal(data.uDegree, 2);
  assert.equal(data.vDegree, 2);
  assert.deepEqual(data.poles, bezierPoles);
  assert.deepEqual(data.weights, weights);
  assert.deepEqual(data.uKnots, []);
  assert.deepEqual(data.vMultiplicities, []);

  const edited = await scope.updateSurfacePole(surface, 1, 1, [1, 1, 4], { weight: 2 });
  const editedData = await client.surfaceControlData(edited);
  near(editedData.poles[1][1][2], 4);
  near(editedData.weights[1][1], 2);
  near((await client.surfaceControlData(surface)).poles[1][1][2], 0);
  near((await client.surfaceControlData(surface)).weights[1][1], 1);

  const trimmed = await scope.trimSurface(surface, { uFirst: 0.2, uLast: 0.8, vFirst: 0.25, vLast: 0.75 });
  const domain = await client.surfaceDomain(trimmed);
  near(domain.uFirst, 0.2);
  near(domain.uLast, 0.8);
  near(domain.vFirst, 0.25);
  near(domain.vLast, 0.75);

  const converted = await scope.convertSurfaceToBSpline(trimmed);
  assert.equal((await client.surfaceControlData(converted)).type, "bspline");
  await assert.rejects(
    scope.updateSurfacePole(surface, 3, 0, [0, 0, 0]),
    (error) => error.code === "InvalidArgs",
  );
  await assert.rejects(
    scope.trimSurface(surface, { uFirst: -1, uLast: 0.5, vFirst: 0, vLast: 1 }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("surface conversion and pole editing preserve trimmed boundaries", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const support = await scope.makeSurfaceFace(
    { type: "plane" },
    { uFirst: 0, uLast: 10, vFirst: 0, vLast: 10 },
  );
  const outer = await scope.makePolygon([
    [0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0],
  ]);
  const hole = await scope.makePolygon([
    [3, 3, 0], [7, 3, 0], [7, 7, 0], [3, 7, 0],
  ]);
  const trimmed = await scope.makeFaceOnSurface(support, outer, [hole]);
  assert.equal((await client.topologyCounts(trimmed)).wire, 2);

  const converted = await scope.convertSurfaceToBSpline(trimmed);
  assert.equal((await client.topologyCounts(converted)).wire, 2);
  const convertedData = await client.surfaceControlData(converted);
  const edited = await scope.updateSurfacePole(converted, 0, 0, [0, 0, 1]);
  assert.equal((await client.topologyCounts(edited)).wire, 2);
  assert.notDeepEqual((await client.surfaceControlData(edited)).poles, convertedData.poles);

  await assert.rejects(
    scope.trimSurface(trimmed, { uFirst: 1, uLast: 9, vFirst: 1, vLast: 9 }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("BSpline surface construction validates rectangular control data and exposes knots", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const poles = Array.from({ length: 4 }, (_, u) =>
    Array.from({ length: 4 }, (_, v) => [u, v, (u * v) / 5]),
  );
  const weights = poles.map((row) => row.map((_, v) => (v === 3 ? 2 : 1)));
  const surface = await scope.makeSurfaceBSpline(poles, { weights, uDegree: 3, vDegree: 3 });
  const data = await client.surfaceControlData(surface);
  assert.equal(data.type, "bspline");
  assert.equal(data.uDegree, 3);
  assert.equal(data.vDegree, 3);
  assert.deepEqual(data.uKnots, [0, 1]);
  assert.deepEqual(data.vKnots, [0, 1]);
  assert.deepEqual(data.uMultiplicities, [4, 4]);
  assert.deepEqual(data.vMultiplicities, [4, 4]);
  assert.equal(data.uPeriodic, false);
  assert.equal(data.vPeriodic, false);
  near(data.weights[0][3], 2);
  assert.equal(await client.isValid(surface), true);
  const converted = await scope.convertSurfaceToBSpline(surface);
  assert.equal((await client.surfaceControlData(converted)).type, "bspline");

  await assert.rejects(
    scope.makeSurfaceBezier([[[0, 0, 0], [0, 1, 0]], [[1, 0, 0]]]),
    (error) => error.code === "InvalidArgs",
  );
  await assert.rejects(
    scope.makeSurfaceBSpline(poles, { uDegree: 2, vDegree: 3, uKnots: [0, 1], uMultiplicities: [4, 4] }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("surface degree reduction lowers U/V degree and preserves bounded face topology", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const poles = Array.from({ length: 5 }, (_, u) =>
    Array.from({ length: 5 }, (_, v) => [u, v, ((u - 2) * (v - 2)) / 8]),
  );
  const source = await scope.makeSurfaceBezier(poles);
  const reduced = await scope.reduceSurfaceDegree(source, 3, 3, {
    tolerance: 0.02,
    uContinuity: "c1",
    vContinuity: "c1",
    maxSegments: 40,
  });

  const sourceData = await client.surfaceControlData(source);
  const reducedData = await client.surfaceControlData(reduced.shape);
  assert.equal(sourceData.uDegree, 4);
  assert.equal(sourceData.vDegree, 4);
  assert.ok(reducedData.uDegree <= 3);
  assert.ok(reducedData.vDegree <= 3);
  assert.ok(reduced.maxError >= 0 && reduced.maxError <= 0.02);
  assert.equal((await client.topologyCounts(reduced.shape)).wire, 1);
  assert.equal(await client.isValid(reduced.shape), true);
  await assert.rejects(
    scope.reduceSurfaceDegree(source, 4, 4),
    (error) => error.code === "InvalidArgs",
  );

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
