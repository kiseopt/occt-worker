import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const near = (actual, expected, tolerance = 1e-6) => {
  assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected}`);
};

async function makeFaceXY(scope, z) {
  const points = [[0, 0, z], [10, 0, z], [10, 10, z], [0, 10, z]];
  const edges = [];
  for (let index = 0; index < points.length; index += 1) {
    edges.push(await scope.makeEdgeLine(points[index], points[(index + 1) % points.length]));
  }
  return scope.makeFace(await scope.makeWire(edges));
}

async function makeFaceYZ(scope, x) {
  const points = [[x, 0, -5], [x, 10, -5], [x, 10, 5], [x, 0, 5]];
  const edges = [];
  for (let index = 0; index < points.length; index += 1) {
    edges.push(await scope.makeEdgeLine(points[index], points[(index + 1) % points.length]));
  }
  return scope.makeFace(await scope.makeWire(edges));
}

async function makeFaceWithHole(scope) {
  const outer = await scope.makePolygon([[0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0]]);
  const hole = await scope.makePolygon([[3, 3, 0], [3, 7, 0], [7, 7, 0], [7, 3, 0]]);
  return scope.makeFace(outer, [hole]);
}

test("point projection and curve-surface intersections use native OCCT analysis", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const line = await scope.makeEdgeLine([0, 0, 0], [10, 0, 0]);
  const projection = await client.projectPointCurve(line, [4, 3, 0]);
  assert.ok(projection.solutions.length >= 1);
  const nearest = projection.solutions.reduce((a, b) => (a.distance < b.distance ? a : b));
  near(nearest.point[0], 4);
  near(nearest.point[1], 0);
  near(nearest.distance, 3);

  const face = await makeFaceXY(scope, 0);
  const surfaceProjection = await client.projectPointSurface(face, [4, 3, 5]);
  assert.ok(surfaceProjection.solutions.length >= 1);
  const surfaceNearest = surfaceProjection.solutions.reduce((a, b) => (a.distance < b.distance ? a : b));
  near(surfaceNearest.point[0], 4);
  near(surfaceNearest.point[1], 3);
  near(surfaceNearest.point[2], 0);
  near(surfaceNearest.distance, 5);

  const crossing = await scope.makeEdgeLine([4, 3, -2], [4, 3, 2]);
  const intersections = await client.intersectCurveSurface(crossing, face);
  assert.equal(intersections.segments.length, 0);
  assert.ok(intersections.points.length >= 1);
  near(intersections.points[0].point[0], 4);
  near(intersections.points[0].point[1], 3);
  near(intersections.points[0].point[2], 0);
  await scope.end();
});

test("curve and surface extrema return parameters and endpoint geometry", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const first = await scope.makeEdgeLine([0, 0, 0], [10, 0, 0]);
  const second = await scope.makeEdgeLine([0, 3, 0], [10, 3, 0]);
  const curveExtrema = await client.extremaCurveCurve(first, second);
  assert.equal(curveExtrema.parallel, true);
  assert.ok(curveExtrema.solutions.length >= 1);
  assert.ok(curveExtrema.solutions.some((solution) => Math.abs(solution.distance - 3) < 1e-5));

  const circle = await scope.makeEdgeCircle(5);
  const crossingLine = await scope.makeEdgeLine([10, -20, 0], [10, 20, 0]);
  const stationaryExtrema = await client.extremaCurveCurve(circle, crossingLine);
  assert.equal(stationaryExtrema.parallel, false);
  assert.ok(stationaryExtrema.solutions.some((solution) => Math.abs(solution.distance - 5) < 1e-5));
  assert.ok(stationaryExtrema.solutions.some((solution) => Math.abs(solution.distance - 15) < 1e-5));

  const face = await makeFaceXY(scope, 0);
  const curveSurface = await client.extremaCurveSurface(first, face);
  assert.ok(curveSurface.solutions.length >= 1);
  const surfaceExtrema = await client.extremaSurfaceSurface(face, await makeFaceXY(scope, 2));
  assert.equal(surfaceExtrema.parallel, true);
  assert.ok(surfaceExtrema.solutions.length >= 1);
  assert.ok(surfaceExtrema.solutions.some((solution) => Math.abs(solution.distance - 2) < 1e-5));

  const perpendicular = await makeFaceYZ(scope, 5);
  const intersectionShapes = await scope.intersectSurfaceSurface(face, perpendicular);
  assert.ok(intersectionShapes.length >= 1);
  assert.ok((await Promise.all(intersectionShapes.map((shape) => client.shapeType(shape)))).every((type) => type === "edge"));
  await scope.end();
});

test("curve-curve intersection returns only finite topological results", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const horizontal = await scope.makeEdgeLine([0, 0, 0], [10, 0, 0]);
  const crossing = await scope.makeEdgeLine([5, -5, 0], [5, 5, 0]);
  const points = await scope.intersectCurveCurve(horizontal, crossing);
  assert.equal(points.length, 1);
  assert.equal(await client.shapeType(points[0]), "vertex");
  const bounds = await client.bbox(points[0]);
  near(bounds.min[0], 5);
  near(bounds.min[1], 0);

  const missedFiniteSegment = await scope.makeEdgeLine([15, -5, 0], [15, 5, 0]);
  assert.deepEqual(await scope.intersectCurveCurve(horizontal, missedFiniteSegment), []);
  await scope.end();
});

test("surface projection and intersections respect finite topology and holes", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const face = await makeFaceWithHole(scope);

  const overHole = await client.projectPointSurface(face, [5, 5, 2]);
  assert.equal(overHole.solutions.length, 0);
  const overMaterial = await client.projectPointSurface(face, [1, 1, 2]);
  assert.equal(overMaterial.solutions.length, 1);

  const edgeMissingFiniteFace = await scope.makeEdgeLine([1, 1, 2], [1, 1, 3]);
  assert.deepEqual(await client.intersectCurveSurface(edgeMissingFiniteFace, face), {
    points: [],
    segments: [],
  });
  const edgeThroughHole = await scope.makeEdgeLine([5, 5, -1], [5, 5, 1]);
  assert.deepEqual(await client.intersectCurveSurface(edgeThroughHole, face), {
    points: [],
    segments: [],
  });

  const perpendicular = await makeFaceYZ(scope, 5);
  const sections = await scope.intersectSurfaceSurface(face, perpendicular);
  assert.equal(sections.length, 2);
  const lengths = await Promise.all(sections.map((shape) => client.massProps(shape, "linear")));
  near(lengths.reduce((total, item) => total + item.mass, 0), 6, 1e-5);
  await scope.end();
});

test("curve-surface segments preserve UV branches across periodic seams", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const cylinder = await scope.makeCylinder(5, 10);
  const side = await scope.getSubShape(cylinder, "face", 0);
  const circle = await scope.makeEdgeCircle(5, { center: [0, 0, 5] });
  const intersection = await client.intersectCurveSurface(circle, side);
  assert.equal(intersection.segments.length, 1);
  near(Math.abs(intersection.segments[0].u2 - intersection.segments[0].u1), 2 * Math.PI, 1e-5);
  near(intersection.segments[0].v1, 5, 1e-5);
  near(intersection.segments[0].v2, 5, 1e-5);
  await scope.end();
});

test("oriented bounds and detailed shape diagnostics are available", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([10, 2, 1]);
  const rotated = await scope.rotate(box, {
    origin: [0, 0, 0],
    direction: [0, 0, 1],
    angle: Math.PI / 4,
  });
  const bounds = await client.obb(rotated, { optimal: true, useShapeTolerance: false });
  near(Math.hypot(...bounds.axes[0]), 1);
  near(Math.hypot(...bounds.axes[1]), 1);
  near(Math.hypot(...bounds.axes[2]), 1);
  near(8 * bounds.halfSizes[0] * bounds.halfSizes[1] * bounds.halfSizes[2], 20, 1e-4);

  const diagnostics = await client.diagnoseShape(rotated, { exact: true });
  assert.equal(diagnostics.valid, true);
  assert.deepEqual(diagnostics.issues, []);
  await scope.end();
});

test("wedge, half-space, and splitter cover advanced topology workflows", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const wedge = await scope.makeWedge([10, 5, 4], 3);
  assert.equal(await client.shapeType(wedge), "solid");
  assert.equal(await client.isValid(wedge), true);

  const cuttingFace = await makeFaceXY(scope, 5);
  const box = await scope.makeBox([10, 10, 10]);
  const split = await scope.split([box], [cuttingFace]);
  const counts = await client.topologyCounts(split);
  assert.equal(counts.solid, 2);
  assert.equal(await client.isValid(split), true);

  const boundary = await makeFaceXY(scope, 0);
  const halfSpace = await scope.makeHalfSpace(boundary, [5, 5, -1]);
  assert.equal(await client.shapeType(halfSpace), "solid");
  await scope.end();
});

test("general fuse exposes split cells and source-input mapping", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const first = await scope.makeBox([4, 4, 4]);
  const second = await scope.makeBox([4, 4, 4], [2, 0, 0]);
  const result = await scope.generalFuse([first, second]);
  assert.equal(result.shapes.length, 3);
  assert.equal(result.sourceIndices.length, result.shapes.length);
  assert.deepEqual(
    result.sourceIndices
      .map((indices) => [...indices].sort((a, b) => a - b))
      .sort((a, b) => a.length - b.length || a[0] - b[0]),
    [[0], [1], [0, 1]],
  );
  for (const cell of result.shapes) {
    assert.equal(await client.shapeType(cell), "solid");
    assert.equal(await client.isValid(cell), true);
  }
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("general fuse cell rules select, merge, and containerize partition cells", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const first = await scope.makeBox([4, 4, 4]);
  const second = await scope.makeBox([4, 4, 4], [2, 0, 0]);

  const intersection = await scope.selectGeneralFuseCells(
    [first, second],
    [{ take: [0, 1] }],
  );
  assert.ok(Math.abs((await client.massProps(intersection)).mass - 32) < 1e-8);

  const mergedUnion = await scope.selectGeneralFuseCells(
    [first, second],
    [
      { take: [0], avoid: [1], material: 1 },
      { take: [1], avoid: [0], material: 1 },
      { take: [0, 1], material: 1 },
    ],
    { removeInternalBoundaries: true },
  );
  assert.ok(Math.abs((await client.massProps(mergedUnion)).mass - 96) < 1e-8);
  assert.equal((await client.topologyCounts(mergedUnion)).solid, 1);
  assert.equal(await client.isValid(mergedUnion), true);

  const containers = await scope.selectGeneralFuseCells(
    [first, second],
    [{ take: [0], avoid: [1] }, { take: [1], avoid: [0] }, { take: [0, 1] }],
    { makeContainers: true },
  );
  assert.equal((await client.topologyCounts(containers)).compsolid, 1);

  await assert.rejects(
    scope.selectGeneralFuseCells([first, second], [{ take: [0], avoid: [0] }]),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("draft angle tapers selected faces without changing the input", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([10, 10, 10]);
  const drafted = await scope.draftAngle(
    box,
    [0, 1, 2, 3],
    [0, 0, 1],
    0.1,
    { neutralPlane: { origin: [0, 0, 0], normal: [0, 0, 1] } },
  );
  const draftedVolume = (await client.massProps(drafted)).mass;
  assert.ok(draftedVolume > 800 && draftedVolume < 900);
  assert.ok(Math.abs((await client.massProps(box)).mass - 1000) < 1e-8);
  assert.equal(await client.isValid(drafted), true);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("local prism adds and removes material from a selected base face", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([10, 10, 10]);
  const added = await scope.localPrism(box, [5], [0, 0, 1], 2, "add");
  const removed = await scope.localPrism(box, [5], [0, 0, -1], 2, "cut");
  assert.ok(Math.abs((await client.massProps(added)).mass - 1200) < 1e-8);
  assert.ok(Math.abs((await client.massProps(removed)).mass - 800) < 1e-8);
  assert.ok(Math.abs((await client.massProps(box)).mass - 1000) < 1e-8);
  assert.equal(await client.isValid(added), true);
  assert.equal(await client.isValid(removed), true);

  const until = await scope.makeSurfaceFace(
    { type: "plane", origin: [0, 0, 15], direction: [0, 0, 1] },
    { uFirst: -20, uLast: 20, vFirst: -20, vLast: 20 },
  );
  const limited = await scope.localPrism(box, [5], [0, 0, 1], { mode: "until", until });
  assert.ok(Math.abs((await client.massProps(limited)).mass - 1500) < 1e-8);
  assert.equal(await client.isValid(limited), true);
  const revolved = await scope.localRevolution(
    box, [5], [0, 0, 10], [0, 1, 0], Math.PI / 2,
  );
  assert.equal(await client.isValid(revolved), true);
  const revolvedMass = (await client.massProps(revolved)).mass;
  assert.ok(revolvedMass > 0, `local revolution mass ${revolvedMass}`);
  await assert.rejects(
    client.request("localPrism", {
      scopeId: scope.scopeId,
      base: box,
      faceIndices: [5],
      direction: [0, 0, 1],
      mode: "unknown",
    }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("linear form builds native rib and slot features from a planar wire", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([100, 100, 100]);
  const profile = await scope.makeWire([
    await scope.makeEdgeLine([50, 50, 120], [50, 50, 50]),
    await scope.makeEdgeLine([50, 50, 50], [120, 50, 50]),
  ]);
  const added = await scope.linearForm(box, profile, {
    planeOrigin: [50, 50, 50],
    planeNormal: [0, -1, 0],
    direction: [0, 30, 0],
    direction1: [0, 0, 0],
    modify: false,
    operation: "add",
  });
  const removed = await scope.linearForm(box, profile, {
    planeOrigin: [50, 50, 50],
    planeNormal: [0, -1, 0],
    direction: [0, 30, 0],
    direction1: [0, 0, 0],
    modify: false,
    operation: "cut",
  });
  assert.equal(await client.isValid(added), true);
  assert.equal(await client.isValid(removed), true);
  assert.ok((await client.massProps(added)).mass > 1000000);
  assert.ok((await client.massProps(removed)).mass < 1000000);
  assert.ok(Math.abs((await client.massProps(box)).mass - 1000000) < 1e-6);
  const opposing = await scope.linearForm(box, profile, {
    planeOrigin: [50, 50, 50],
    planeNormal: [0, -1, 0],
    direction: [0, 30, 0],
    direction1: [0, -30, 0],
    modify: false,
    operation: "add",
  });
  assert.equal(await client.isValid(opposing), true);
  await assert.rejects(
    client.request("linearForm", {
      scopeId: scope.scopeId,
      base: box,
      profile,
      planeOrigin: [50, 50, 50],
      planeNormal: [0, -1, 0],
      direction: [0, 30, 0],
      direction1: [0, 30, 0],
      modify: false,
      operation: "add",
    }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("revolution form builds native rotational rib and slot features", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const base = await scope.makeCylinder(50, 100, { origin: [0, 0, 0], direction: [0, 0, 1] });
  const profile = await scope.makeWire([
    await scope.makeEdgeLine([50, 0, 40], [60, 0, 50]),
    await scope.makeEdgeLine([60, 0, 50], [50, 0, 60]),
  ]);
  const common = {
    planeOrigin: [0, 0, 0],
    planeNormal: [0, 1, 0],
    origin: [0, 0, 0],
    direction: [0, 0, 1],
    height1: 5,
    height2: 5,
    modify: false,
  };
  const baseMass = (await client.massProps(base)).mass;
  const rib = await scope.revolutionForm(base, profile, { ...common, operation: "add" });
  const slot = await scope.revolutionForm(base, profile, { ...common, operation: "cut" });
  assert.equal(await client.isValid(rib), true);
  assert.equal(await client.isValid(slot), true);
  assert.ok((await client.massProps(rib)).mass > baseMass);
  assert.ok((await client.massProps(slot)).mass < baseMass);
  await assert.rejects(
    client.request("revolutionForm", {
      scopeId: scope.scopeId,
      base,
      profile,
      ...common,
      height1: 0,
      height2: 0,
      operation: "add",
    }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("glue binds coincident faces and preserves a valid joined solid", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const base = await scope.makeBox([10, 10, 10]);
  const newShape = await scope.makeBox([4, 4, 4], [3, 3, 10]);
  const glued = await scope.glue(newShape, base, [{ newIndex: 4, baseIndex: 5 }]);
  assert.equal(await client.isValid(glued), true);
  assert.equal(await client.shapeType(glued), "solid");
  assert.ok(Math.abs((await client.massProps(glued)).mass - 1064) < 1e-8);
  await assert.rejects(
    client.request("glue", {
      scopeId: scope.scopeId,
      newShape,
      baseShape: base,
      faceBindings: [{ newIndex: 99, baseIndex: 5 }],
    }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("cylindrical hole supports through and blind limits with operation history", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([10, 10, 10]);
  const through = await scope.cylindricalHole(box, [5, 5, -1], [0, 0, 1], 1, {
    mode: "throughAll",
    includeHistory: true,
  });
  const blind = await scope.cylindricalHole(box, [5, 5, -1], [0, 0, 1], 1, {
    mode: "blind",
    length: 5,
  });
  assert.ok(Math.abs((await client.massProps(through.shape)).mass - (1000 - Math.PI * 10)) < 1e-6);
  assert.ok(Math.abs((await client.massProps(blind.shape)).mass - (1000 - Math.PI * 4)) < 1e-6);
  assert.ok(through.history);
  assert.ok(through.history.retained.length + through.history.modified.length > 0);
  assert.equal(await client.isValid(through.shape), true);
  assert.equal(await client.isValid(blind.shape), true);
  assert.ok(Math.abs((await client.massProps(box)).mass - 1000) < 1e-8);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("defeaturing removes a cylindrical through-hole face", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([10, 10, 10]);
  const holed = (await scope.cylindricalHole(box, [5, 5, -1], [0, 0, 1], 1)).shape;
  const faces = await client.getSubShapes(holed, "face");
  let cylindricalFace = -1;
  for (const face of faces) {
    const handle = await scope.getSubShape(holed, "face", face.index);
    if ((await client.surfaceGeometry(handle)).type === "cylinder") cylindricalFace = face.index;
  }
  assert.ok(cylindricalFace >= 0);
  const restored = await scope.defeature(holed, [cylindricalFace], { includeHistory: true });
  assert.ok(Math.abs((await client.massProps(restored.shape)).mass - 1000) < 1e-6);
  assert.ok(restored.history);
  assert.equal(await client.isValid(restored.shape), true);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
