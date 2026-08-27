import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { Worker } from "node:worker_threads";
import { DirectClient } from "../../dist/direct-client.js";
import { WorkerClient } from "../../dist/worker-client.js";
import { TimeoutError } from "../../dist/types.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("G1 plate-with-hole workflow and scoped resources", async () => {
  const client = await DirectClient.create(wasm);
  const capabilities = await client.initialize();
  assert.equal(capabilities.protocolVersion, "1.2.0");

  const scope = await client.beginScope();
  const plate = await scope.makeBox([10, 10, 4]);
  const tool = await scope.makeCylinder(2, 4, { origin: [5, 5, 0] });
  const plateBefore = await client.bbox(plate);
  const toolBefore = await client.bbox(tool);
  const cut = await scope.booleanCut(plate, [tool], { includeHistory: true });
  assert.ok(cut.history);
  assert.deepEqual(await client.bbox(plate), plateBefore);
  assert.deepEqual(await client.bbox(tool), toolBefore);

  const box = await client.bbox(cut.shape);
  assert.ok(Math.abs(box.min[0]) < 1e-5);
  assert.ok(Math.abs(box.max[0] - 10) < 1e-5);

  const mesh = await client.tessellate(cut.shape, { linearDeflection: 0.25 });
  assert.ok(mesh.positions.length > 0);
  assert.equal(mesh.positions.length, mesh.normals.length);
  assert.equal(mesh.indices.length % 3, 0);
  assert.equal(mesh.faceGroups.length % 3, 0);
  const fineAfterCoarse = await client.tessellate(cut.shape, { linearDeflection: 0.05 });

  const freshClient = await DirectClient.create(wasm);
  const freshScope = await freshClient.beginScope();
  const freshPlate = await freshScope.makeBox([10, 10, 4]);
  const freshTool = await freshScope.makeCylinder(2, 4, { origin: [5, 5, 0] });
  const freshCut = await freshScope.booleanCut(freshPlate, [freshTool]);
  const directFine = await freshClient.tessellate(freshCut.shape, { linearDeflection: 0.05 });
  assert.deepEqual(fineAfterCoarse.positions, directFine.positions);
  assert.deepEqual(fineAfterCoarse.indices, directFine.indices);
  await freshScope.end();

  const brep = await client.exportBREP(cut.shape);
  assert.ok(brep.byteLength > 0);
  const restored = await scope.importBREP(brep);
  assert.deepEqual(await client.bbox(restored), box);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("stale handles are rejected by the client epoch", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const shape = await scope.makeBox([1, 1, 1]);
  const otherClient = await DirectClient.create(wasm);
  await assert.rejects(() => otherClient.bbox(shape), TypeError);
  await scope.end();
});

test("WorkerClient transfers results and rebuilds after timeout", async () => {
  const factory = () => {
    const worker = new Worker(new URL("./node-worker.mjs", import.meta.url));
    return {
      postMessage: (message, transfer) => worker.postMessage(message, transfer),
      terminate: () => { void worker.terminate(); },
      addEventListener: (type, listener) => {
        if (type === "message") worker.on("message", (data) => listener({ data }));
        else worker.on("error", (error) => listener({ error, message: error.message }));
      },
    };
  };
  const client = await WorkerClient.create(factory, wasm.buffer.slice(wasm.byteOffset, wasm.byteOffset + wasm.byteLength));
  try {
    const scope = await client.beginScope();
    const box = await scope.makeBox([2, 3, 4]);
    assert.deepEqual((await client.bbox(box)).max.map(Math.round), [2, 3, 4]);
    const step = await client.exportSTEP(box, { unit: "mm", timestamp: "2026-01-02T03:04:05" });
    const restored = await scope.importSTEP(step, { unit: "mm" });
    assert.equal(restored.rootCount, 1);
    assert.deepEqual((await client.bbox(restored.shape)).max.map(Math.round), [2, 3, 4]);

    const sharedBREP = await client.requestShared("exportBREP", { shape: box });
    assert.ok(sharedBREP.data.data instanceof SharedArrayBuffer);
    const sharedLength = sharedBREP.data.data.byteLength;
    const sharedRestored = await scope.importBREP(sharedBREP.data.data);
    assert.equal(sharedBREP.data.data.byteLength, sharedLength);
    assert.deepEqual((await client.bbox(sharedRestored)).max.map(Math.round), [2, 3, 4]);

    const torus = await scope.makeTorus(10, 3);
    await assert.rejects(
      client.request("tessellate", { shape: torus, linearDeflection: 0.001 }, 1),
      TimeoutError,
    );
    const rebuiltScope = await client.beginScope();
    const rebuiltBox = await rebuiltScope.makeBox([1, 1, 1]);
    assert.equal(await client.shapeType(rebuiltBox), "solid");
    await rebuiltScope.end();
  } finally {
    client.close();
  }
});
