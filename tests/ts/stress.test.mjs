import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("repeated scopes and BREP round-trips leave no live resources", async () => {
  const client = await DirectClient.create(wasm);

  for (let iteration = 0; iteration < 25; iteration++) {
    const scope = await client.beginScope();
    const box = await scope.makeBox([iteration + 1, 2, 3]);
    const brep = await client.exportBREP(box);
    const restored = await scope.importBREP(brep);
    assert.deepEqual(await client.bbox(restored), await client.bbox(box));
    await scope.end();

    const stats = await client.stats();
    assert.equal(stats.liveShapeHandles, 0);
    assert.equal(stats.liveBufferBytes, 0);
  }
});

test("corrupt BREP input fails without leaking buffers", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  await assert.rejects(
    scope.importBREP(new TextEncoder().encode("not a brep file").buffer),
    (error) => error.code === "ImportExportFailed",
  );
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});
