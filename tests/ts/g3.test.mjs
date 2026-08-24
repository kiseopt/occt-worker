import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("G3 curves, loft, local features, and export queries", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const bezier = await scope.makeEdgeBezier([[0, 0, 0], [2, 3, 0], [4, 0, 0]]);
  const bspline = await scope.makeEdgeBSpline([[0, 0, 0], [2, 2, 0], [4, 0, 0]]);
  const controlBSpline = await scope.makeEdgeBSpline(
    new Float64Array([0, 0, 0, 1, 2, 0, 3, 2, 0, 4, 0, 0]),
    { mode: "controlPoints", degree: 3 },
  );
  const helix = await scope.makeEdgeHelix(2, 1, 1, { handedness: "left" });
  const parameterizedArc = await scope.makeEdgeArc({
    center: [0, 0, 0], normal: [0, 0, 1], radius: 2, startAngle: 0, endAngle: Math.PI / 2,
  });
  assert.equal(await client.shapeType(bezier), "edge");
  assert.equal(await client.shapeType(bspline), "edge");
  assert.equal(await client.shapeType(controlBSpline), "edge");
  assert.equal(await client.shapeType(helix), "edge");
  assert.equal(await client.shapeType(parameterizedArc), "edge");
  assert.ok((await client.tessellateEdges(helix)).positions.length > 0);

  const lower = await scope.makePolygon([[0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0]]);
  const upper = await scope.makePolygon([[2, 2, 5], [8, 2, 5], [8, 8, 5], [2, 8, 5]]);
  const loft = await scope.loft([lower, upper], { solid: true });
  assert.equal(await client.shapeType(loft), "solid");
  const loftFillet = await scope.fillet(loft, [0], 0.5);
  assert.equal(await client.isValid(loftFillet), true);
  const fillet = await scope.fillet(await scope.makeBox([10, 10, 10]), [0], 0.5);
  assert.equal(await client.isValid(fillet), true);
  const variableFillet = await scope.fillet(await scope.makeBox([10, 10, 10]), [0, 1], 0.5, { radii: [0.25, 0.4] });
  const lawFillet = await scope.fillet(await scope.makeBox([10, 10, 10]), [0], 0.5, {
    radiusLaw: [
      { parameter: 0, radius: 0.25 },
      { parameter: 0.5, radius: 0.8 },
      { parameter: 1, radius: 0.5 },
    ],
  });
  const linearLawFillet = await scope.fillet(await scope.makeBox([10, 10, 10]), [0], 0.25, { radius2: 0.5 });
  await assert.rejects(
    scope.fillet(await scope.makeBox([10, 10, 10]), [0], 0.5, {
      radiusLaw: [
        { parameter: 0, radius: 0.25 },
        { parameter: 0.75, radius: 0.8 },
        { parameter: 0.5, radius: 0.5 },
      ],
    }),
    (error) => error.code === "InvalidArgs",
  );
  assert.equal(await client.isValid(variableFillet), true);
  await assert.rejects(
    async () => scope.fillet(await scope.makeBox([10, 10, 10]), [0, 1], 0.5, { radii: [0.25] }),
    (error) => error.code === "InvalidArgs",
  );
  const chamfer = await scope.chamfer(await scope.makeBox([10, 10, 10]), [0], 0.5);
  const variableChamfer = await scope.chamfer(await scope.makeBox([10, 10, 10]), [0, 1], 0.5, { distances: [0.25, 0.4] });
  const twoDistanceBox = await scope.makeBox([10, 10, 10]);
  const edgeFaces = await client.getAdjacency(twoDistanceBox, "edge", "face");
  const referenceFace = edgeFaces.items[0].adjacent[0];
  const twoDistanceChamfer = await scope.chamfer(twoDistanceBox, [0], 0.25, {
    distance2: 0.75,
    referenceFaceIndices: [referenceFace],
  });
  const nonAdjacentFace = [0, 1, 2, 3, 4, 5].find((index) => !edgeFaces.items[0].adjacent.includes(index));
  await assert.rejects(
    scope.chamfer(await scope.makeBox([10, 10, 10]), [0], 0.25, {
      distance2: 0.75,
      referenceFaceIndices: [nonAdjacentFace],
    }),
    (error) => error.code === "InvalidArgs",
  );
  const hollow = await scope.hollow(await scope.makeBox([10, 10, 10]), -1, [5]);
  await assert.rejects(
    scope.hollow(await scope.makeBox([10, 10, 10]), -1, [5], -1),
    (error) => error.code === "InvalidArgs",
  );
  const offset = await scope.offsetShape(await scope.makeBox([10, 10, 10]), 0.5);
  const offsetWire = await scope.offsetWire2D(lower, 0.5);
  assert.equal(await client.isValid(chamfer), true);
  assert.equal(await client.isValid(variableChamfer), true);
  assert.equal(await client.isValid(lawFillet), true);
  assert.ok(Math.abs((await client.massProps(lawFillet)).mass - (await client.massProps(linearLawFillet)).mass) > 0.1);
  assert.equal(await client.isValid(twoDistanceChamfer), true);
  assert.ok(Math.abs((await client.massProps(twoDistanceChamfer)).mass - 999.0625) < 1e-6);
  assert.equal(await client.isValid(hollow), true);
  assert.equal(await client.isValid(offset), true);
  assert.equal(await client.isValid(offsetWire), true);
  const uvMesh = await client.tessellate(fillet, { includeUV: true });
  assert.equal(uvMesh.uvs.length, uvMesh.positions.length / 3 * 2);
  const spine = await scope.makePolygon([[0, 0, 0], [0, 0, 10]], false);
  const profile = await scope.makePolygon([[1, 0, 0], [0, 1, 0], [-1, 0, 0], [0, -1, 0]], true);
  const fixedSweep = await scope.sweepPipeShell(spine, [profile], {
    mode: "fixedAxis",
    axis: { direction: [0, 0, 1], xDirection: [1, 0, 0] },
  });
  const auxiliarySpine = await scope.makePolygon([[2, 0, 0], [2, 0, 10]], false);
  const guidedSweep = await scope.sweepPipeShell(spine, [profile], {
    mode: "auxiliarySpine",
    auxiliarySpine,
  });
  assert.equal(await client.shapeType(fixedSweep), "shell");
  assert.equal(await client.shapeType(guidedSweep), "shell");
  const pipe = await scope.sweepPipe(spine, await scope.makeEdgeCircle(1));
  assert.equal(await client.isValid(pipe), true);
  const mesh = await client.tessellateEdges(loft);
  assert.ok(mesh.positions.length > 0 && mesh.edgeGroups.length > 0);
  assert.ok((await client.exportSTL(loft)).byteLength > 84);
  await scope.end();
});

test("G3 transform copy and array semantics", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([1, 1, 1]);
  const shared = await scope.transform(box, { translation: [3, 0, 0], copy: false });
  const copied = await scope.transform(box, { translation: [0, 3, 0], copy: true });
  assert.deepEqual((await client.bbox(box)).min.map((value) => Math.round(value) || 0), [0, 0, 0]);
  assert.deepEqual((await client.bbox(shared)).min.map((value) => Math.round(value) || 0), [3, 0, 0]);
  assert.deepEqual((await client.bbox(copied)).min.map((value) => Math.round(value) || 0), [0, 3, 0]);

  const linear = await scope.batchTransformCopy(box, {
    mode: "linear",
    count: 2,
    translation: [2, 0, 0],
  });
  assert.equal(linear.length, 2);
  assert.deepEqual((await client.bbox(linear[0])).min.map((value) => Math.round(value) || 0), [2, 0, 0]);
  assert.deepEqual((await client.bbox(linear[1])).min.map((value) => Math.round(value) || 0), [4, 0, 0]);

  const circular = await scope.batchTransformCopy(box, {
    mode: "circular",
    count: 2,
    angle: Math.PI,
    direction: [0, 0, 1],
  });
  assert.equal(circular.length, 2);
  assert.deepEqual((await client.bbox(circular[1])).max.map((value) => Math.round(value) || 0), [0, 0, 1]);
  await scope.end();
});
