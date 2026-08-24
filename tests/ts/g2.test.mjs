import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("G2 planar construction, extrusion, transform, and queries", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const polygon = await scope.makePolygon([[0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0]]);
  const face = await scope.makeFace(polygon);
  const solid = await scope.extrude(face, [0, 0, 4]);
  assert.equal(await client.shapeType(solid), "solid");
  assert.deepEqual((await client.topologyCounts(solid)).face, 6);
  assert.equal(await client.isValid(solid), true);
  assert.ok(Math.abs((await client.massProps(solid)).mass - 400) < 1e-8);

  const moved = await scope.transform(solid, { translation: [2, 3, 4] });
  const movedBox = await client.bbox(moved);
  assert.ok(Math.abs(movedBox.min[0] - 2) < 1e-5 && Math.abs(movedBox.min[1] - 3) < 1e-5 && Math.abs(movedBox.min[2] - 4) < 1e-5);
  assert.ok(Math.abs(movedBox.max[0] - 12) < 1e-5 && Math.abs(movedBox.max[1] - 13) < 1e-5 && Math.abs(movedBox.max[2] - 8) < 1e-5);

  const sphere = await scope.makeSphere(2, { origin: [5, 5, 2] });
  const common = await scope.booleanCommon(moved, [sphere]);
  assert.equal(await client.isValid(common.shape), true);
  const fixed = await scope.fixShape(solid);
  assert.equal(await client.isSameShape(fixed, solid), false);
  const originalBox = await client.bbox(solid);
  assert.ok(originalBox.min.every((value) => Math.abs(value) < 1e-5));
  assert.ok(originalBox.max.every((value, axis) => Math.abs(value - [10, 10, 4][axis]) < 1e-5));
  const unified = await scope.unifySameDomain(solid);
  assert.equal(await client.isValid(unified), true);
  const sheared = await scope.generalTransform(solid, [1, 0, 0.2, 0, 0, 1, 0, 0, 0, 0, 1, 0]);
  assert.equal(await client.isValid(sheared), true);
  const overlap = await scope.makeBox([10, 10, 4], [5, 5, 0]);
  const section = await scope.section(solid, overlap);
  assert.equal(await client.shapeType(section), "compound");
  await scope.end();
});
