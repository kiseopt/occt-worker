import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const near = (actual, expected, tolerance = 1e-8) => {
  assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected}`);
};

test("curve domains expose edge evaluation, derivatives, tangents, and curvature", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const circle = await scope.makeEdgeCircle(2);
  const domain = await client.curveDomain(circle);
  near(domain.first, 0);
  near(domain.last, Math.PI * 2);
  assert.equal(domain.periodic, true);
  near(domain.period, Math.PI * 2);

  const evaluation = await client.evaluateCurve(circle, domain.first);
  evaluation.point.forEach((value, index) => near(value, [2, 0, 0][index]));
  evaluation.tangent.forEach((value, index) => near(value, [0, 1, 0][index]));
  assert.equal(evaluation.tangentDefined, true);
  assert.equal(evaluation.curvatureDefined, true);
  near(evaluation.curvature, 0.5);

  await assert.rejects(
    client.evaluateCurve(circle, domain.last + 1),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
});

test("curve geometry, continuity, trimming, and BSpline conversion preserve finite geometry", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const line = await scope.makeEdgeLine([0, 0, 0], [10, 0, 0]);
  assert.deepEqual(await client.curveGeometry(line), {
    type: "line",
    origin: [0, 0, 0],
    direction: [1, 0, 0],
  });
  assert.equal(await client.curveContinuity(line), "cn");
  const trimmedLine = await scope.trimCurve(line, 2, 8);
  const trimmedBounds = await client.bbox(trimmedLine);
  near(trimmedBounds.min[0], 2, 1e-6);
  near(trimmedBounds.max[0], 8, 1e-6);
  await assert.rejects(
    scope.trimCurve(line, -1, 8),
    (error) => error.code === "InvalidArgs",
  );

  const circle = await scope.makeEdgeCircle(3, { center: [1, 2, 3], normal: [0, 0, 1] });
  const circleGeometry = await client.curveGeometry(circle);
  assert.equal(circleGeometry.type, "circle");
  near(circleGeometry.radius, 3);
  circleGeometry.center.forEach((value, index) => near(value, [1, 2, 3][index]));
  circleGeometry.normal.forEach((value, index) => near(value, [0, 0, 1][index]));

  const arc = await scope.trimCurve(circle, 0.25, 2.5);
  const converted = await scope.convertCurveToBSpline(arc);
  const convertedGeometry = await client.curveGeometry(converted);
  assert.equal(convertedGeometry.type, "bspline");
  assert.equal(convertedGeometry.rational, true);
  const convertedDomain = await client.curveDomain(converted);
  const originalStart = await client.evaluateCurve(arc, 0.25);
  const originalEnd = await client.evaluateCurve(arc, 2.5);
  const convertedStart = await client.evaluateCurve(converted, convertedDomain.first);
  const convertedEnd = await client.evaluateCurve(converted, convertedDomain.last);
  convertedStart.point.forEach((value, index) => near(value, originalStart.point[index]));
  convertedEnd.point.forEach((value, index) => near(value, originalEnd.point[index]));

  const plane = await scope.makeSurfaceFace(
    { type: "plane" },
    { uFirst: -1, uLast: 1, vFirst: -1, vLast: 1 },
  );
  assert.deepEqual(await client.surfaceContinuity(plane), { u: "cn", v: "cn" });
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("subshape extraction enables oriented face evaluation and principal curvatures", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const sphere = await scope.makeSphere(2);
  const face = await scope.getSubShape(sphere, "face", 0);
  assert.equal(await client.shapeType(face), "face");

  const domain = await client.surfaceDomain(face);
  const u = (domain.uFirst + domain.uLast) / 2;
  const v = (domain.vFirst + domain.vLast) / 2;
  const evaluation = await client.evaluateSurface(face, u, v);
  near(Math.hypot(...evaluation.point), 2);
  assert.equal(evaluation.normalDefined, true);
  near(Math.hypot(...evaluation.normal), 1);
  assert.equal(evaluation.curvatureDefined, true);
  near(Math.abs(evaluation.minimumCurvature), 0.5);
  near(Math.abs(evaluation.maximumCurvature), 0.5);
  near(evaluation.gaussianCurvature, 0.25);

  await assert.rejects(
    scope.getSubShape(sphere, "face", 1),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("bounded analytic surfaces expose exact geometry and topology-clipped iso-curves", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const axis = { origin: [1, 2, 3], direction: [0, 0, 1], xDirection: [0, 1, 0] };
  const cases = [
    [{ type: "plane", ...axis }, { uFirst: -2, uLast: 3, vFirst: -4, vLast: 5 }],
    [{ type: "cylinder", radius: 2, ...axis }, { uFirst: 0, uLast: Math.PI, vFirst: -2, vLast: 4 }],
    [{ type: "cone", referenceRadius: 2, semiAngle: 0.3, ...axis }, { uFirst: 0, uLast: Math.PI, vFirst: 0, vLast: 3 }],
    [{ type: "sphere", radius: 3, ...axis }, { uFirst: 0, uLast: Math.PI, vFirst: -1, vLast: 1 }],
    [{ type: "torus", majorRadius: 5, minorRadius: 1, ...axis }, { uFirst: 0, uLast: Math.PI, vFirst: 0.5, vLast: 2 }],
  ];
  for (const [definition, bounds] of cases) {
    const face = await scope.makeSurfaceFace(definition, bounds);
    assert.equal(await client.shapeType(face), "face");
    assert.equal(await client.isValid(face), true);
    const domain = await client.surfaceDomain(face);
    for (const key of ["uFirst", "uLast", "vFirst", "vLast"]) near(domain[key], bounds[key]);
    const geometry = await client.surfaceGeometry(face);
    assert.equal(geometry.type, definition.type);
    geometry.origin.forEach((value, index) => near(value, axis.origin[index]));
    geometry.direction.forEach((value, index) => near(value, axis.direction[index]));
    geometry.xDirection.forEach((value, index) => near(value, axis.xDirection[index]));
    for (const key of ["radius", "referenceRadius", "semiAngle", "majorRadius", "minorRadius"]) {
      if (key in definition) near(geometry[key], definition[key]);
    }
  }

  const outer = await scope.makePolygon([[-5, -5, 0], [5, -5, 0], [5, 5, 0], [-5, 5, 0]]);
  const hole = await scope.makePolygon([[-1, -2, 0], [1, -2, 0], [1, 2, 0], [-1, 2, 0]]);
  const faceWithHole = await scope.makeFace(outer, [hole]);
  const domain = await client.surfaceDomain(faceWithHole);
  const segments = await scope.surfaceIsoCurve(faceWithHole, "v", (domain.vFirst + domain.vLast) / 2);
  assert.equal(segments.length, 2);
  assert.deepEqual(await Promise.all(segments.map((segment) => client.shapeType(segment))), ["edge", "edge"]);
  await assert.rejects(
    scope.surfaceIsoCurve(faceWithHole, "u", domain.uLast + 1),
    (error) => error.code === "InvalidArgs",
  );

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("C0 BSpline seams omit undefined differential geometry and iso-curves remain edge-only", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const curve = await scope.makeEdgeBSpline([
    [-3, 0, 0], [-2, 0, 0], [-1, 0, 0], [0, 0, 0],
    [0, 1, 0], [0, 2, 0], [0, 3, 0],
  ], {
    mode: "controlPoints",
    degree: 3,
    knots: [0, 0.5, 1],
    multiplicities: [4, 3, 4],
  });
  const curveAtSeam = await client.evaluateCurve(curve, 0.5);
  assert.equal(curveAtSeam.tangentDefined, false);
  assert.equal(curveAtSeam.curvatureDefined, false);
  assert.equal("tangent" in curveAtSeam, false);
  assert.equal("curvature" in curveAtSeam, false);

  const surface = await scope.makeSurfaceBSpline([
    [[-3, 0, 0], [-3, 1, 0]],
    [[-2, 0, 0], [-2, 1, 0]],
    [[-1, 0, 0], [-1, 1, 0]],
    [[0, 0, 0], [0, 1, 0]],
    [[0, 0, 1], [0, 1, 1]],
    [[0, 0, 2], [0, 1, 2]],
    [[0, 0, 3], [0, 1, 3]],
  ], {
    uDegree: 3,
    vDegree: 1,
    uKnots: [0, 0.5, 1],
    vKnots: [0, 1],
    uMultiplicities: [4, 3, 4],
    vMultiplicities: [2, 2],
  });
  const surfaceAtSeam = await client.evaluateSurface(surface, 0.5, 0.5);
  assert.equal(surfaceAtSeam.normalDefined, false);
  assert.equal(surfaceAtSeam.curvatureDefined, false);
  assert.equal("normal" in surfaceAtSeam, false);
  assert.equal("minimumCurvature" in surfaceAtSeam, false);

  const triangle = await scope.makePolygon([[0, 0, 0], [10, 0, 0], [0, 10, 0]]);
  const face = await scope.makeFace(triangle);
  const domain = await client.surfaceDomain(face);
  assert.deepEqual(await scope.surfaceIsoCurve(face, "u", domain.uLast), []);

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
