import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("point-set BSpline curve approximation exposes a valid finite edge", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const points = Array.from({ length: 9 }, (_, index) => {
    const x = index / 2;
    return [x, Math.sin(x), 0];
  });
  const curve = await scope.approximateCurveBSpline(points, {
    degreeMin: 2,
    degreeMax: 5,
    continuity: "c2",
    tolerance: 0.05,
    parameterization: "centripetal",
  });
  assert.equal(await client.shapeType(curve), "edge");
  assert.equal(await client.isValid(curve), true);
  const data = await client.curveControlData(curve);
  assert.equal(data.type, "bspline");
  assert.ok(data.degree >= 2 && data.degree <= 5);
  assert.ok(data.poles.length >= 2);
  const faired = await scope.approximateCurveBSpline(points, {
    degreeMax: 8,
    continuity: "c1",
    tolerance: 0.1,
    variationalSmoothing: { length: 0, curvature: 1, torsion: 0.1 },
  });
  assert.equal(await client.isValid(faired), true);
  assert.equal((await client.curveControlData(faired)).type, "bspline");
  await assert.rejects(
    scope.approximateCurveBSpline(points, {
      variationalSmoothing: { length: 0, curvature: 0, torsion: 0 },
    }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("point-grid BSpline surface approximation returns a bounded face", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const points = Array.from({ length: 6 }, (_, u) =>
    Array.from({ length: 6 }, (_, v) => [u / 2, v / 2, Math.sin(u / 2) * Math.cos(v / 2)]));
  const surface = await scope.approximateSurfaceBSpline(points, {
    degreeMin: 2,
    degreeMax: 5,
    continuity: "c2",
    tolerance: 0.05,
    parameterization: "uniform",
  });
  assert.equal(await client.shapeType(surface), "face");
  assert.equal(await client.isValid(surface), true);
  const geometry = await client.surfaceGeometry(surface);
  assert.equal(geometry.type, "bspline");
  const domain = await client.surfaceDomain(surface);
  assert.ok(domain.uFirst < domain.uLast);
  assert.ok(domain.vFirst < domain.vLast);
  const faired = await scope.approximateSurfaceBSpline(points, {
    degreeMax: 8,
    continuity: "c1",
    tolerance: 0.1,
    variationalSmoothing: { length: 0, curvature: 1, torsion: 0 },
  });
  assert.equal(await client.isValid(faired), true);
  assert.equal((await client.surfaceGeometry(faired)).type, "bspline");
  await assert.rejects(
    scope.approximateSurfaceBSpline([[[0, 0, 0], [1, 0, 0]], [[0, 1, 0]]]),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
