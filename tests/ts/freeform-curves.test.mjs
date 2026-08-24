import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const near = (actual, expected, tolerance = 1e-8) => {
  assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected}`);
};

test("Bezier curve control data and pole editing are scoped and independent", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const poles = [[0, 0, 0], [2, 3, 0], [4, 0, 0]];
  const curve = await scope.makeEdgeBezier(poles);

  assert.deepEqual(await client.curveControlData(curve), {
    type: "bezier",
    degree: 2,
    poles,
    weights: [1, 1, 1],
    knots: [],
    multiplicities: [],
    periodic: false,
  });

  const rationalWeights = [1, 2, 1];
  const rational = await scope.makeEdgeBezier(poles, { weights: rationalWeights });
  assert.deepEqual((await client.curveControlData(rational)).weights, rationalWeights);

  const edited = await scope.updateCurvePole(curve, 1, [2, 3, 2], { weight: 2 });
  const editedData = await client.curveControlData(edited);
  near(editedData.poles[1][2], 2);
  near(editedData.weights[1], 2);
  near((await client.curveControlData(curve)).poles[1][2], 0);
  near((await client.curveControlData(curve)).weights[1], 1);
  assert.deepEqual(await client.curveDomain(edited), await client.curveDomain(curve));

  await assert.rejects(
    scope.updateCurvePole(curve, 3, [0, 0, 0]),
    (error) => error.code === "InvalidArgs",
  );
  const line = await scope.makeEdgeLine([0, 0, 0], [1, 0, 0]);
  await assert.rejects(
    client.curveControlData(line),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("BSpline curve control data exposes knots and editing preserves a trimmed domain", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const poles = [[0, 0, 0], [1, 2, 0], [3, 2, 0], [4, 0, 0]];
  const weights = [1, 1.5, 1, 1];
  const curve = await scope.makeEdgeBSpline(poles, {
    mode: "controlPoints",
    degree: 3,
    weights,
  });
  const data = await client.curveControlData(curve);
  assert.equal(data.type, "bspline");
  assert.equal(data.degree, 3);
  assert.deepEqual(data.poles, poles);
  assert.deepEqual(data.weights, weights);
  assert.deepEqual(data.knots, [0, 1]);
  assert.deepEqual(data.multiplicities, [4, 4]);
  assert.equal(data.periodic, false);

  const trimmed = await scope.trimCurve(curve, 0.2, 0.8);
  const edited = await scope.updateCurvePole(trimmed, 2, [3, 2, 1]);
  const domain = await client.curveDomain(edited);
  near(domain.first, 0.2);
  near(domain.last, 0.8);
  near((await client.curveControlData(edited)).poles[2][2], 1);
  near((await client.curveControlData(trimmed)).poles[2][2], 0);

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("curve degree reduction returns a lower-degree independent edge and achieved error", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const source = await scope.makeEdgeBezier([
    [0, 0, 0], [1, 2, 0], [2, -1, 0], [3, 2, 0], [4, -1, 0], [5, 0, 0],
  ]);
  const trimmed = await scope.trimCurve(source, 0.15, 0.85);
  const reduced = await scope.reduceCurveDegree(trimmed, 3, {
    tolerance: 0.02,
    continuity: "c1",
    maxSegments: 40,
  });

  assert.equal((await client.curveControlData(source)).degree, 5);
  assert.ok((await client.curveControlData(reduced.shape)).degree <= 3);
  assert.ok(reduced.maxError >= 0 && reduced.maxError <= 0.02);
  const domain = await client.curveDomain(reduced.shape);
  near(domain.first, 0.15);
  near(domain.last, 0.85);
  await assert.rejects(
    scope.reduceCurveDegree(trimmed, 5),
    (error) => error.code === "InvalidArgs",
  );

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
