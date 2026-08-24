import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient, KernelError } from "../../dist/index.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("projectHLR returns reusable exact visible and hidden edge shapes", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([10, 8, 6]);

  const parallel = await scope.projectHLR(box, {
    direction: [1, 1, -1],
    up: [0, 0, 1],
  });
  assert.equal(await client.shapeType(parallel.visible), "compound");
  assert.equal(await client.shapeType(parallel.hidden), "compound");
  assert.ok((await client.topologyCounts(parallel.visible)).edge > 0);
  assert.ok((await client.topologyCounts(parallel.hidden)).edge > 0);
  for (const projected of [parallel.visible, parallel.hidden]) {
    const bounds = await client.bbox(projected);
    assert.ok(Math.abs(bounds.min[2]) < 1e-6);
    assert.ok(Math.abs(bounds.max[2]) < 1e-6);
  }

  const perspective = await scope.projectHLR(box, {
    direction: [1, 1, -1],
    up: [0, 0, 1],
    projection: "perspective",
    focus: 100,
  });
  assert.ok((await client.topologyCounts(perspective.visible)).edge > 0);

  await assert.rejects(
    scope.projectHLR(box, { direction: [0, 0, -1], up: [0, 0, 1] }),
    (error) => error instanceof KernelError && error.code === "InvalidArgs",
  );
  await scope.end();
});
