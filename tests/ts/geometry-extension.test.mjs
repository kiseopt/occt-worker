import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const near = (actual, expected, tolerance = 1e-7) => {
  assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected}`);
};

test("bounded Bezier curve extension returns a new edge through the target point", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const curve = await scope.makeEdgeBezier([[0, 0, 0], [1, 1, 0], [2, 0, 0]]);
  const extended = await scope.extendCurve(curve, [3, 0, 0], { continuity: "c2" });
  assert.equal(await client.shapeType(extended), "edge");
  assert.equal(await client.isValid(extended), true);
  const domain = await client.curveDomain(extended);
  const endpoint = await client.evaluateCurve(extended, domain.last);
  near(endpoint.point[0], 3, 1e-5);
  near(endpoint.point[1], 0, 1e-5);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("bounded plane surface extension grows the requested UV side", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const plane = await scope.makeSurfaceFace(
    { type: "plane" },
    { uFirst: -1, uLast: 1, vFirst: -2, vLast: 2 },
  );
  const before = await client.surfaceDomain(plane);
  const extended = await scope.extendSurface(plane, 3, "u", "after", "c1");
  assert.equal(await client.shapeType(extended), "face");
  assert.equal(await client.isValid(extended), true);
  const after = await client.surfaceDomain(extended);
  near(after.uFirst, before.uFirst);
  near(after.uLast, before.uLast + 3);
  near(after.vFirst, before.vFirst);
  near(after.vLast, before.vLast);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("surface extension preserves inner trimming wires", async () => {
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
  const before = await client.massProps(trimmed, "surface");
  const extended = await scope.extendSurface(trimmed, 2, "u", "after", "c1");
  assert.equal(await client.isValid(extended), true);
  assert.equal((await client.topologyCounts(extended)).wire, 2);
  assert.ok((await client.massProps(extended, "surface")).mass > before.mass);
  const domain = await client.surfaceDomain(extended);
  near(domain.uLast, 12);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
