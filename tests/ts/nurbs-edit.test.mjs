import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("BSpline curve editing preserves the input and supports knot/degree/control-net edits", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const curve = await scope.makeEdgeBSpline(
    [[0, 0, 0], [1, 2, 0], [3, 2, 0], [4, 0, 0]],
    { mode: "controlPoints", degree: 2, knots: [0, 1, 2], multiplicities: [3, 1, 3] },
  );
  const original = await client.curveControlData(curve);
  const inserted = await scope.editCurveBSpline(curve, { action: "insertKnot", knot: 0.5 });
  assert.equal((await client.curveControlData(inserted)).knots.length, original.knots.length + 1);
  const elevated = await scope.editCurveBSpline(inserted, { action: "increaseDegree", degree: 3 });
  assert.equal((await client.curveControlData(elevated)).degree, 3);
  const elevatedData = await client.curveControlData(elevated);
  elevatedData.poles[0][2] += 1;
  const edited = await scope.editCurveBSpline(elevated, {
    action: "setControlNet",
    poles: elevatedData.poles,
    weights: elevatedData.weights,
  });
  assert.equal((await client.curveControlData(edited)).poles[0][2], elevatedData.poles[0][2]);
  assert.equal((await client.curveControlData(curve)).degree, original.degree);
  const removed = await scope.editCurveBSpline(inserted, {
    action: "removeKnot",
    knotIndex: 1,
    multiplicity: 0,
    tolerance: 1e-4,
  });
  assert.equal((await client.curveControlData(removed)).knots.length, original.knots.length);
  const oldDomain = await client.curveDomain(curve);
  const oldMiddle = await client.evaluateCurve(curve, (oldDomain.first + oldDomain.last) / 2);
  const reparameterized = await scope.reparameterizeCurve(curve, 10, 20);
  assert.deepEqual(await client.curveDomain(reparameterized), { first: 10, last: 20, periodic: false, period: 0 });
  const newMiddle = await client.evaluateCurve(reparameterized, 15);
  assert.ok(Math.hypot(...newMiddle.point.map((value, index) => value - oldMiddle.point[index])) < 1e-9);
  await assert.rejects(scope.reparameterizeCurve(curve, 1, 1), (error) => error.code === "InvalidArgs");
  await scope.end();
});

test("BSpline surface editing preserves face topology and supports U/V edits", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const poles = Array.from({ length: 4 }, (_, u) =>
    Array.from({ length: 4 }, (_, v) => [u, v, (u * v) / 10]));
  const surface = await scope.makeSurfaceBSpline(poles, { uDegree: 2, vDegree: 2 });
  const original = await client.surfaceControlData(surface);
  const inserted = await scope.editSurfaceBSpline(surface, { action: "insertUKnot", uKnot: 0.5, uMultiplicity: 1 });
  assert.equal((await client.surfaceControlData(inserted)).uKnots.length, original.uKnots.length + 1);
  const insertedV = await scope.editSurfaceBSpline(inserted, { action: "insertVKnot", vKnot: 0.5, vMultiplicity: 1 });
  const elevated = await scope.editSurfaceBSpline(insertedV, { action: "increaseDegree", uDegree: 3, vDegree: 3 });
  const elevatedData = await client.surfaceControlData(elevated);
  assert.equal(elevatedData.uDegree, 3);
  assert.equal(elevatedData.vDegree, 3);
  elevatedData.poles[0][0][2] += 2;
  const edited = await scope.editSurfaceBSpline(elevated, {
    action: "setControlNet",
    poles: elevatedData.poles,
    weights: elevatedData.weights,
  });
  assert.equal((await client.surfaceControlData(edited)).poles[0][0][2], elevatedData.poles[0][0][2]);
  assert.equal(await client.isValid(edited), true);
  assert.equal((await client.topologyCounts(edited)).wire, (await client.topologyCounts(surface)).wire);
  const removed = await scope.editSurfaceBSpline(inserted, {
    action: "removeUKnot",
    uKnotIndex: 1,
    uMultiplicity: 0,
    tolerance: 1e-4,
  });
  assert.equal((await client.surfaceControlData(removed)).uKnots.length, original.uKnots.length);
  const oldDomain = await client.surfaceDomain(surface);
  const oldMiddle = await client.evaluateSurface(
    surface,
    (oldDomain.uFirst + oldDomain.uLast) / 2,
    (oldDomain.vFirst + oldDomain.vLast) / 2,
  );
  const reparameterized = await scope.reparameterizeSurface(surface, {
    uFirst: -2, uLast: 2, vFirst: 10, vLast: 20,
  });
  const newDomain = await client.surfaceDomain(reparameterized);
  assert.deepEqual(
    [newDomain.uFirst, newDomain.uLast, newDomain.vFirst, newDomain.vLast],
    [-2, 2, 10, 20],
  );
  const newMiddle = await client.evaluateSurface(reparameterized, 0, 15);
  assert.ok(Math.hypot(...newMiddle.point.map((value, index) => value - oldMiddle.point[index])) < 1e-8);
  assert.equal(await client.isValid(reparameterized), true);
  assert.deepEqual(await client.topologyCounts(reparameterized), await client.topologyCounts(surface));
  await scope.end();
});
