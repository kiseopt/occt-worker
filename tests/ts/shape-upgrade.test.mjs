import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("shapeUpgrade continuity wraps OCCT ShapeUpgrade_ShapeDivideContinuity", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const edge = await scope.makeEdgeBSpline(
    [[0, 0, 0], [1, 0, 0], [2, 1, 0], [3, 1, 0], [4, 0, 0]],
    { mode: "controlPoints", degree: 2, knots: [0, 0.5, 1], multiplicities: [3, 2, 3] },
  );
  assert.equal(await client.curveContinuity(edge), "c0");
  const upgraded = await scope.shapeUpgrade(edge, {
    boundaryCriterion: "c0",
    pcurveCriterion: "c0",
    surfaceCriterion: "c1",
    edgeMode: 2,
  });
  assert.equal(await client.shapeType(upgraded), "edge");
  assert.equal(await client.isValid(upgraded), true);
  await assert.rejects(
    scope.shapeUpgrade(edge, { edgeMode: 3 }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("shapeUpgrade exposes native divide, conversion, wire-removal, and location modes", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();

  const cylinder = await scope.makeSurfaceFace(
    { type: "cylinder", radius: 5 },
    { uFirst: 0, uLast: 2 * Math.PI, vFirst: 0, vLast: 10 },
  );
  const angle = await scope.shapeUpgrade(cylinder, { mode: "angle", maxAngle: Math.PI / 2 });
  assert.ok((await client.topologyCounts(angle)).face >= 4);

  const plane = await scope.makeSurfaceFace(
    { type: "plane" },
    { uFirst: 0, uLast: 10, vFirst: 0, vLast: 10 },
  );
  const byArea = await scope.shapeUpgrade(plane, { mode: "area", areaMode: "maxArea", maxArea: 30 });
  assert.ok((await client.topologyCounts(byArea)).face >= 4);
  const byParts = await scope.shapeUpgrade(plane, { mode: "area", areaMode: "parts", nbParts: 4 });
  assert.ok((await client.topologyCounts(byParts)).face >= 4);
  const byUV = await scope.shapeUpgrade(plane, { mode: "area", areaMode: "uv", uSplits: 2, vSplits: 3 });
  assert.ok((await client.topologyCounts(byUV)).face >= 6);

  const closedFaces = await scope.shapeUpgrade(cylinder, { mode: "closedFaces", splitPoints: 2 });
  assert.ok((await client.topologyCounts(closedFaces)).face >= 2);
  const circle = await scope.makeEdgeCircle(5);
  const closedEdges = await scope.shapeUpgrade(circle, { mode: "closedEdges" });
  assert.ok((await client.topologyCounts(closedEdges)).edge >= 2);

  const bezier = await scope.shapeUpgrade(circle, {
    mode: "convertToBezier",
    convert2d: false,
    convertSurfaces: false,
  });
  const bezierCounts = await client.topologyCounts(bezier);
  assert.ok(bezierCounts.edge >= 2);
  for (let index = 0; index < bezierCounts.edge; index++) {
    assert.equal((await client.curveGeometry(await scope.getSubShape(bezier, "edge", index))).type, "bezier");
  }

  const outer = await scope.makePolygon([[0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0]]);
  const hole = await scope.makePolygon([[4, 4, 0], [4, 6, 0], [6, 6, 0], [6, 4, 0]]);
  const faceWithHole = await scope.makeFace(outer, [hole]);
  const withoutHole = await scope.shapeUpgrade(faceWithHole, {
    mode: "removeInternalWires",
    minArea: 5,
    removeFaces: false,
  });
  assert.equal((await client.topologyCounts(faceWithHole)).wire, 2);
  assert.equal((await client.topologyCounts(withoutHole)).wire, 1);

  const moved = await scope.translate(plane, [5, 6, 7]);
  const flattened = await scope.shapeUpgrade(moved, { mode: "removeLocations", removeLevel: "face" });
  assert.deepEqual(await client.bbox(flattened), await client.bbox(moved));
  assert.equal(await client.isValid(flattened), true);

  await assert.rejects(
    scope.shapeUpgrade(plane, { mode: "angle", maxAngle: 3 * Math.PI }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
