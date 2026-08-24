import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const near = (actual, expected, tolerance = 1e-7) => {
  assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected}`);
};

test("finite hyperbola, parabola, and offset curve constructors preserve analytic domains", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();

  const hyperbola = await scope.makeEdgeHyperbola(2, 1, -1, 1);
  assert.equal(await client.isValid(hyperbola), true);
  assert.deepEqual(await client.curveDomain(hyperbola), {
    first: -1,
    last: 1,
    periodic: false,
    period: 0,
  });
  const hyperbolaGeometry = await client.curveGeometry(hyperbola);
  assert.equal(hyperbolaGeometry.type, "hyperbola");
  near(hyperbolaGeometry.majorRadius, 2);
  near(hyperbolaGeometry.minorRadius, 1);
  const hyperbolaApex = await client.evaluateCurve(hyperbola, 0);
  hyperbolaApex.point.forEach((value, index) => near(value, [2, 0, 0][index]));

  const parabola = await scope.makeEdgeParabola(1, -2, 2);
  assert.equal((await client.curveGeometry(parabola)).type, "parabola");
  const parabolaApex = await client.evaluateCurve(parabola, 0);
  parabolaApex.point.forEach((value, index) => near(value, [0, 0, 0][index]));
  const parabolaEnd = await client.evaluateCurve(parabola, 2);
  parabolaEnd.point.forEach((value, index) => near(value, [1, 2, 0][index]));

  const line = await scope.makeEdgeLine([0, 0, 0], [10, 0, 0]);
  const offset = await scope.makeEdgeOffset(line, 2, [0, 0, 1]);
  assert.deepEqual(await client.curveDomain(offset), await client.curveDomain(line));
  const offsetGeometry = await client.curveGeometry(offset);
  assert.equal(offsetGeometry.type, "offset");
  near(offsetGeometry.offset, 2);
  const offsetMidpoint = await client.evaluateCurve(offset, 5);
  offsetMidpoint.point.forEach((value, index) => near(value, [5, -2, 0][index]));

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("extrusion, revolution, ruled, and offset surface constructors return finite valid faces", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();

  const profile = await scope.makeEdgeLine([0, 0, 0], [2, 0, 0]);
  const extrusion = await scope.makeSurfaceExtrusion(profile, [0, 0, 1], 0, 3);
  assert.equal(await client.isValid(extrusion), true);
  assert.equal((await client.surfaceGeometry(extrusion)).type, "extrusion");
  assert.deepEqual((await client.bbox(extrusion)).max.map(Math.round), [2, 0, 3]);

  const meridian = await scope.makeEdgeLine([2, 0, 0], [2, 0, 3]);
  const revolution = await scope.makeSurfaceRevolution(meridian);
  assert.equal(await client.isValid(revolution), true);
  const revolutionBox = await client.bbox(revolution);
  revolutionBox.min.forEach((value, index) => near(value, [-2, -2, 0][index], 1e-6));
  revolutionBox.max.forEach((value, index) => near(value, [2, 2, 3][index], 1e-6));

  const first = await scope.makeEdgeLine([0, 0, 0], [2, 0, 0]);
  const second = await scope.makeEdgeLine([0, 3, 1], [2, 3, 1]);
  const ruled = await scope.makeSurfaceRuled(first, second);
  assert.equal(await client.isValid(ruled), true);
  const ruledBox = await client.bbox(ruled);
  ruledBox.min.forEach((value, index) => near(value, [0, 0, 0][index], 1e-6));
  ruledBox.max.forEach((value, index) => near(value, [2, 3, 1][index], 1e-6));

  const plane = await scope.makeSurfaceFace(
    { type: "plane" },
    { uFirst: 0, uLast: 2, vFirst: 0, vLast: 3 },
  );
  const offset = await scope.makeSurfaceOffset(plane, 1);
  assert.equal(await client.isValid(offset), true);
  const offsetBox = await client.bbox(offset);
  near(offsetBox.min[2], 1, 1e-6);
  near(offsetBox.max[2], 1, 1e-6);
  assert.deepEqual(await client.surfaceDomain(offset), await client.surfaceDomain(plane));

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

