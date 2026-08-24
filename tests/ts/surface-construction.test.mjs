import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

const cylinderPoint = (radius, angle, z) => [
  radius * Math.cos(angle),
  radius * Math.sin(angle),
  z,
];

async function makeCylinderPatchWire(scope, radius, uFirst, uLast, zFirst, zLast) {
  const middle = (uFirst + uLast) / 2;
  const bottom = await scope.makeEdgeArc(
    cylinderPoint(radius, uFirst, zFirst),
    cylinderPoint(radius, middle, zFirst),
    cylinderPoint(radius, uLast, zFirst),
  );
  const right = await scope.makeEdgeLine(
    cylinderPoint(radius, uLast, zFirst),
    cylinderPoint(radius, uLast, zLast),
  );
  const top = await scope.makeEdgeArc(
    cylinderPoint(radius, uLast, zLast),
    cylinderPoint(radius, middle, zLast),
    cylinderPoint(radius, uFirst, zLast),
  );
  const left = await scope.makeEdgeLine(
    cylinderPoint(radius, uFirst, zLast),
    cylinderPoint(radius, uFirst, zFirst),
  );
  return scope.makeWire([bottom, right, top, left]);
}

test("N-side filling builds a valid constrained surface from a closed wire", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const boundary = await scope.makePolygon([
    [0, 0, 0],
    [10, 0, 0],
    [10, 10, 0],
    [0, 10, 0],
  ]);
  const filling = await scope.makeSurfaceFilling(boundary, { points: [[5, 5, 3]] });
  assert.equal(await client.shapeType(filling), "face");
  assert.equal(await client.isValid(filling), true);
  assert.ok((await client.massProps(filling, "surface")).mass > 100);

  const openBoundary = await scope.makePolygon([[0, 0, 0], [5, 0, 0], [5, 5, 0]], false);
  await assert.rejects(
    scope.makeSurfaceFilling(openBoundary),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("N-side filling accepts G1/G2 support-face constraints by boundary edge index", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([10, 10, 10]);
  const top = await scope.getSubShape(box, "face", 0);
  const boundary = await scope.getSubShape(top, "wire", 0);
  const boundaryEdges = [];
  for (let index = 0; index < 4; index += 1) {
    boundaryEdges.push(await scope.getSubShape(boundary, "edge", index));
  }
  const allEdges = await client.getSubShapes(box, "edge");
  const edgeFaces = await client.getAdjacency(box, "edge", "face");
  const constraints = [];
  for (let boundaryIndex = 0; boundaryIndex < 1; boundaryIndex += 1) {
    let matchedIndex = -1;
    for (let edgeIndex = 0; edgeIndex < allEdges.length; edgeIndex += 1) {
      const candidate = await scope.getSubShape(box, "edge", edgeIndex);
      if (await client.isSameShape(boundaryEdges[boundaryIndex], candidate)) {
        matchedIndex = edgeIndex;
        break;
      }
    }
    assert.ok(matchedIndex >= 0, `boundary edge ${boundaryIndex} must belong to box`);
    const sideFaceIndex = edgeFaces.items[matchedIndex].adjacent.find((faceIndex) => faceIndex !== 0);
    assert.notEqual(sideFaceIndex, undefined);
    constraints.push({
      edgeIndex: boundaryIndex,
      support: await scope.getSubShape(box, "face", sideFaceIndex),
      continuity: "g1",
    });
  }
  const filling = await scope.makeSurfaceFilling(boundary, { constraints });
  assert.equal(await client.shapeType(filling), "face");
  assert.equal(await client.isValid(filling), true);
  await assert.rejects(
    scope.makeSurfaceFilling(boundary, {
      constraints: [{ edgeIndex: 0, support: boundaryEdges[0], continuity: "g1" }],
    }),
    (error) => error.code === "InvalidArgs",
  );
  await assert.rejects(
    scope.makeSurfaceFilling(boundary, {
      constraints: [{ edgeIndex: 4, support: top, continuity: "g1" }],
    }),
    (error) => error.code === "InvalidArgs",
  );
  const curvedSupport = await scope.makeSurfaceFace(
    { type: "cylinder", radius: 5 },
    { uFirst: 0, uLast: Math.PI / 2, vFirst: 1, vLast: 7 },
  );
  const curvedBoundary = await makeCylinderPatchWire(scope, 5, 0, Math.PI / 2, 1, 7);
  const projectedBoundaryFace = await scope.makeFaceOnSurface(curvedSupport, curvedBoundary);
  const projectedBoundary = await scope.getSubShape(projectedBoundaryFace, "wire", 0);
  const g2Filling = await scope.makeSurfaceFilling(projectedBoundary, {
    constraints: [{ edgeIndex: 0, support: projectedBoundaryFace, continuity: "g2" }],
  });
  assert.equal(await client.shapeType(g2Filling), "face");
  assert.equal(await client.isValid(g2Filling), true);
  const boundaryPoint = cylinderPoint(5, Math.PI / 4, 1);
  const fillingProjection = (await client.projectPointSurface(g2Filling, boundaryPoint)).solutions[0];
  const supportProjection = (await client.projectPointSurface(projectedBoundaryFace, boundaryPoint)).solutions[0];
  assert.ok(fillingProjection && supportProjection);
  const fillingEvaluation = await client.evaluateSurface(
    g2Filling, fillingProjection.u, fillingProjection.v,
  );
  const supportEvaluation = await client.evaluateSurface(
    projectedBoundaryFace, supportProjection.u, supportProjection.v,
  );
  assert.ok(fillingEvaluation.normal && supportEvaluation.normal);
  const normalDot = fillingEvaluation.normal.reduce(
    (sum, value, index) => sum + value * supportEvaluation.normal[index], 0,
  );
  assert.ok(Math.abs(normalDot) > 0.999);
  assert.equal(fillingEvaluation.curvatureDefined, true);
  assert.equal(supportEvaluation.curvatureDefined, true);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("wire trimming projects pcurves onto a cylindrical surface and preserves a hole", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const radius = 5;
  const support = await scope.makeSurfaceFace(
    { type: "cylinder", radius },
    { uFirst: -0.2, uLast: 1.8, vFirst: 0, vLast: 10 },
  );
  const outer = await makeCylinderPatchWire(scope, radius, 0, Math.PI / 2, 2, 8);
  const hole = await makeCylinderPatchWire(scope, radius, 0.4, 1.1, 4, 6);
  const face = await scope.makeFaceOnSurface(support, outer, [hole]);
  assert.equal(await client.shapeType(face), "face");
  assert.equal(await client.isValid(face), true);
  assert.equal((await client.topologyCounts(face)).wire, 2);
  const expectedArea = radius * (Math.PI / 2) * 6 - radius * (1.1 - 0.4) * 2;
  const actualArea = (await client.massProps(face, "surface")).mass;
  assert.ok(Math.abs(actualArea - expectedArea) < 1e-3, `${actualArea} != ${expectedArea}`);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
