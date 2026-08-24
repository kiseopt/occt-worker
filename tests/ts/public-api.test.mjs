import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const near = (actual, expected, tolerance = 1e-5) => {
  assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected}`);
};

test("public primitives, queries, transform aliases, and releaseAll", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([1, 2, 3]);
  const separated = await scope.makeBox([1, 2, 3], [4, 0, 0]);
  const cone = await scope.makeCone(2, 1, 4);
  const vertex = await scope.makeVertex([1, 2, 3]);
  const ellipse = await scope.makeEdgeEllipse(3, 1.5);

  assert.equal(await client.shapeType(cone), "solid");
  assert.equal(await client.isValid(cone), true);
  assert.equal(await client.shapeType(vertex), "vertex");
  assert.equal(await client.shapeType(ellipse), "edge");
  assert.ok((await client.tessellateEdges(ellipse)).positions.length > 6);

  assert.equal(await client.classifyPoint(box, [0.5, 0.5, 0.5]), "inside");
  assert.equal(await client.classifyPoint(box, [4, 4, 4]), "outside");
  near((await client.distance(box, separated)).distance, 3);
  assert.deepEqual(await client.getSubShapes(box, "face"), [
    { type: "face", index: 0 }, { type: "face", index: 1 },
    { type: "face", index: 2 }, { type: "face", index: 3 },
    { type: "face", index: 4 }, { type: "face", index: 5 },
  ]);
  const adjacency = await client.getAdjacency(box);
  assert.equal(adjacency.from, "edge");
  assert.equal(adjacency.to, "face");
  assert.equal(adjacency.items.length, 12);
  assert.ok(adjacency.items.every((item) => item.adjacent.length === 2));

  const translated = await scope.translate(box, [2, 0, 0]);
  const rotated = await scope.rotate(box, { direction: [0, 0, 1], angle: Math.PI / 2 });
  const scaled = await scope.scale(box, 2);
  const mirrored = await scope.mirror(box, [1, 0, 0]);
  near((await client.bbox(translated)).min[0], 2);
  near((await client.bbox(rotated)).min[0], -2);
  near((await client.bbox(scaled)).max[2], 6);
  near((await client.bbox(mirrored)).min[0], -1);

  await client.releaseAll();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("line-wire-face revolution and sewn-shell solid construction", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const profilePoints = [[1, 0, 0], [2, 0, 0], [2, 0, 3], [1, 0, 3]];
  const profileEdges = [];
  for (let index = 0; index < profilePoints.length; index++) {
    profileEdges.push(await scope.makeEdgeLine(
      profilePoints[index],
      profilePoints[(index + 1) % profilePoints.length],
    ));
  }
  const profileWire = await scope.makeWire(profileEdges);
  const profileFace = await scope.makeFace(profileWire);
  const revolved = await scope.revolve(profileFace, [0, 0, 0], [0, 0, 1]);
  assert.equal(await client.shapeType(profileWire), "wire");
  assert.equal(await client.shapeType(revolved), "solid");
  assert.equal(await client.isValid(revolved), true);
  near((await client.massProps(revolved)).mass, 9 * Math.PI);

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
  const sewn = await scope.sew(faces);
  assert.equal(await client.shapeType(sewn.shape), "shell");
  assert.equal(sewn.freeEdges, 0);
  assert.equal(sewn.multipleEdges, 0);
  const solid = await scope.makeSolidFromShell(sewn.shape);
  assert.equal(await client.shapeType(solid), "solid");
  assert.equal(await client.isValid(solid), true);
  near((await client.massProps(solid)).mass, 1);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});
