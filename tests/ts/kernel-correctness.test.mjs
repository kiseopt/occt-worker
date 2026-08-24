import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("retired arena slots never make stale handles valid again", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  let stale;
  for (let index = 0; index < 4095; index++) {
    const shape = await scope.makeBox([1, 1, 1]);
    stale ??= shape;
    await client.release(shape);
  }
  const current = await scope.makeBox([2, 2, 2]);
  await assert.rejects(() => client.bbox(stale), (error) => error.code === "InvalidHandle");
  assert.ok(Math.abs((await client.bbox(current)).max[0] - 2) < 1e-5);
  await scope.end();
});

test("makeFace orients holes and rejects invalid planar faces", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const outer = await scope.makePolygon([[0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0]]);
  const hole = await scope.makePolygon([[3, 3, 0], [7, 3, 0], [7, 7, 0], [3, 7, 0]]);
  const face = await scope.makeFace(outer, [hole]);
  assert.equal(await client.isValid(face), true);
  assert.ok(Math.abs((await client.massProps(face, "surface")).mass - 84) < 1e-8);

  const outside = await scope.makePolygon([[20, 20, 0], [21, 20, 0], [21, 21, 0], [20, 21, 0]]);
  await assert.rejects(() => scope.makeFace(outer, [outside]), (error) => error.code === "ConstructionFailed");
  await scope.end();
});

test("helix handedness does not change pitch direction", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  for (const handedness of ["right", "left"]) {
    const helix = await scope.makeEdgeHelix(2, 1, 2, { handedness });
    const bbox = await client.bbox(helix);
    assert.ok(Math.abs(bbox.min[2]) < 1e-4);
    assert.ok(Math.abs(bbox.max[2] - 2) < 1e-4);
  }
  await scope.end();
});

test("tessellation normals follow the underlying surface", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const sphere = await scope.makeSphere(10);
  const mesh = await client.tessellate(sphere, { linearDeflection: 2, angularDeflection: 1 });
  let maximumAngle = 0;
  for (let index = 0; index < mesh.positions.length / 3; index++) {
    const point = mesh.positions.subarray(index * 3, index * 3 + 3);
    const normal = mesh.normals.subarray(index * 3, index * 3 + 3);
    const radius = Math.hypot(...point);
    const dot = (point[0] * normal[0] + point[1] * normal[1] + point[2] * normal[2]) / radius;
    maximumAngle = Math.max(maximumAngle, Math.acos(Math.max(-1, Math.min(1, dot))));
  }
  assert.ok(maximumAngle < 1e-3, `maximum sphere normal error was ${maximumAngle} radians`);
  await scope.end();
});

test("zero directions are InvalidArgs", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  await assert.rejects(
    () => scope.makeCylinder(1, 1, { direction: [0, 0, 0] }),
    (error) => error.code === "InvalidArgs",
  );
  await assert.rejects(
    () => scope.makeEdgeArc({ center: [0, 0, 0], normal: [0, 0, 0], radius: 1, startAngle: 0, endAngle: 1 }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
});

test("boundary-detectable invalid geometry is InvalidArgs", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([1, 1, 1]);
  await assert.rejects(
    () => scope.extrude(box, [0, 0, 0]),
    (error) => error.code === "InvalidArgs",
  );
  await assert.rejects(
    () => scope.makeCone(1, 1, 2),
    (error) => error.code === "InvalidArgs",
  );
  await assert.rejects(
    () => scope.makeEdgeArc({
      center: [0, 0, 0], normal: [0, 0, 1], xDirection: [0, 0, 2],
      radius: 1, startAngle: 0, endAngle: 1,
    }),
    (error) => error.code === "InvalidArgs",
  );
  const spine = await scope.makePolygon([[0, 0, 0], [0, 0, 2]], false);
  const profile = await scope.makePolygon([[1, 0, 0], [0, 1, 0], [-1, 0, 0]], true);
  await assert.rejects(
    () => scope.sweepPipeShell(spine, [profile], {
      mode: "fixedAxis",
      axis: { direction: [0, 0, 1], xDirection: [0, 0, -1] },
    }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
});

test("box adjacency is available in both topology directions", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([1, 1, 1]);
  const edgeFaces = await client.getAdjacency(box, "edge", "face");
  const faceEdges = await client.getAdjacency(box, "face", "edge");
  const vertexEdges = await client.getAdjacency(box, "vertex", "edge");
  const edgeVertices = await client.getAdjacency(box, "edge", "vertex");
  for (const [adjacency, from, to, itemCount, adjacentCount] of [
    [edgeFaces, "edge", "face", 12, 2],
    [faceEdges, "face", "edge", 6, 4],
    [vertexEdges, "vertex", "edge", 8, 3],
    [edgeVertices, "edge", "vertex", 12, 2],
  ]) {
    assert.equal(adjacency.from, from);
    assert.equal(adjacency.to, to);
    assert.equal(adjacency.items.length, itemCount);
    assert.ok(adjacency.items.every((item) => item.adjacent.length === adjacentCount));
  }
  for (const edge of edgeFaces.items) {
    assert.ok(edge.adjacent.every((faceIndex) => faceEdges.items[faceIndex].adjacent.includes(edge.index)));
  }
  for (const vertex of vertexEdges.items) {
    assert.ok(vertex.adjacent.every((edgeIndex) => edgeVertices.items[edgeIndex].adjacent.includes(vertex.index)));
  }
  assert.equal((await client.request("getAdjacency", { shape: box, from: "face" })).to, "edge");
  assert.equal((await client.request("getAdjacency", { shape: box, from: "vertex" })).to, "edge");
  await scope.end();
});

test("makeSolidFromShell rejects non-shell and open shell inputs", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([1, 1, 1]);
  await assert.rejects(
    () => scope.makeSolidFromShell(box),
    (error) => error.code === "InvalidArgs",
  );

  const facePointSets = [
    [[0, 0, 0], [0, 1, 0], [1, 1, 0], [1, 0, 0]],
    [[0, 0, 1], [1, 0, 1], [1, 1, 1], [0, 1, 1]],
    [[0, 0, 0], [0, 0, 1], [0, 1, 1], [0, 1, 0]],
    [[1, 0, 0], [1, 1, 0], [1, 1, 1], [1, 0, 1]],
    [[0, 0, 0], [1, 0, 0], [1, 0, 1], [0, 0, 1]],
    [[0, 1, 0], [0, 1, 1], [1, 1, 1], [1, 1, 0]],
  ];
  const faces = [];
  for (const points of facePointSets) {
    faces.push(await scope.makeFace(await scope.makePolygon(points)));
  }
  const openShell = await scope.sew(faces.slice(0, 5));
  assert.equal(await client.shapeType(openShell.shape), "shell");
  assert.equal(openShell.freeEdges, 4);
  await assert.rejects(
    () => scope.makeSolidFromShell(openShell.shape),
    (error) => error.code === "InvalidArgs",
  );

  const closedShell = await scope.sew(faces);
  const solid = await scope.makeSolidFromShell(closedShell.shape);
  assert.equal(await client.shapeType(solid), "solid");
  assert.equal(await client.isValid(solid), true);
  await scope.end();
});

test("raw BSpline requests reject unknown modes", async () => {
  const client = await DirectClient.create(wasm);
  const { scopeId } = await client.send("beginScope", {});
  await assert.rejects(
    () => client.send("makeEdgeBSpline", {
      scopeId,
      poles: [[0, 0, 0], [1, 1, 0], [2, 0, 0]],
      mode: "unknown",
    }),
    (error) => error.code === "InvalidArgs",
  );
  await client.send("endScope", { scopeId });
});
