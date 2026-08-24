import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("topology tolerances are inspectable and editable without changing the input", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const before = await client.inspectTolerances(box);
  assert.equal(before.faces.length, 6);
  assert.equal(before.edges.length, 12);
  assert.equal(before.vertices.length, 8);

  const edited = await scope.setTolerance(box, 0.001, "edge");
  const original = await client.inspectTolerances(box);
  const changed = await client.inspectTolerances(edited);
  assert.deepEqual(original, before);
  assert.ok(changed.edges.every(({ tolerance }) => Math.abs(tolerance - 0.001) < 1e-12));
  assert.deepEqual(changed.faces, before.faces);
  assert.deepEqual(changed.vertices, before.vertices);
  assert.equal(await client.isValid(edited), true);

  await assert.rejects(
    scope.setTolerance(box, 0, "all"),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
