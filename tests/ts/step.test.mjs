import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("STEP round-trip through memory streams with explicit units", async () => {
  const client = await DirectClient.create(wasm);
  const capabilities = await client.initialize();
  assert.ok(capabilities.ops.includes("exportSTEP"));
  assert.ok(capabilities.ops.includes("importSTEP"));

  const scope = await client.beginScope();
  const plate = await scope.makeBox([10, 6, 4]);
  const hole = await scope.makeCylinder(1.5, 4, { origin: [5, 3, 0] });
  const cut = await scope.booleanCut(plate, [hole]);
  const box = await client.bbox(cut.shape);

  const step = await client.exportSTEP(cut.shape, { unit: "mm", timestamp: "2026-01-02T03:04:05" });
  assert.deepEqual(await client.probeFormat(step), {
    format: "step", encoding: "text", confidence: "exact", documentMetadata: false,
  });
  assert.ok(step.byteLength > 0);
  const text = new TextDecoder().decode(step);
  assert.ok(text.startsWith("ISO-10303-21;"));
  assert.ok(text.includes("2026-01-02T03:04:05"));
  assert.ok(text.includes(".MILLI."));

  const again = await client.exportSTEP(cut.shape, { unit: "mm", timestamp: "2026-01-02T03:04:05" });
  assert.deepEqual(new Uint8Array(again), new Uint8Array(step));

  const restored = await scope.importSTEP(step, { unit: "mm" });
  assert.equal(restored.rootCount, 1);
  const restoredBox = await client.bbox(restored.shape);
  for (let axis = 0; axis < 3; axis++) {
    assert.ok(Math.abs(restoredBox.min[axis] - box.min[axis]) < 1e-4);
    assert.ok(Math.abs(restoredBox.max[axis] - box.max[axis]) < 1e-4);
  }

  const inMeters = await scope.importSTEP(step, { unit: "m" });
  assert.equal(inMeters.rootCount, 1);
  const meterBox = await client.bbox(inMeters.shape);
  assert.ok(Math.abs(meterBox.max[0] - box.max[0] / 1000) < 1e-6);

  await assert.rejects(
    client.request("importSTEP", { scopeId: scope.scopeId, unit: "furlong", data: { $inputBuffer: step.slice(0) } }),
    (error) => error.code === "InvalidArgs",
  );

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("STEP multi-root export and import preserve independent scoped root shapes", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const first = await scope.makeBox([2, 3, 4]);
  const second = await scope.makeBox([5, 6, 7], [20, 0, 0]);
  const data = await client.exportSTEP([first, second], {
    unit: "mm",
    timestamp: "2026-01-02T03:04:05",
  });
  const imported = await scope.importSTEP(data, { unit: "mm" });
  assert.equal(imported.rootCount, 2);
  assert.equal(imported.shapes.length, 2);
  const bounds = await Promise.all(imported.shapes.map((shape) => client.bbox(shape)));
  bounds.sort((left, right) => left.min[0] - right.min[0]);
  assert.ok(Math.abs(bounds[0].max[0] - 2) < 1e-4);
  assert.ok(Math.abs(bounds[1].min[0] - 20) < 1e-4);
  assert.ok(Math.abs(bounds[1].max[0] - 25) < 1e-4);
  assert.equal(await client.isValid(imported.shape), true);

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("STEP export supports AP203, AP214, and AP242 schemas", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  for (const schema of ["AP203", "AP214", "AP242"]) {
    const data = await client.exportSTEP(box, { schema, timestamp: "2026-01-02T03:04:05" });
    assert.ok(data.byteLength > 0);
    const imported = await scope.importSTEP(data);
    const bounds = await client.bbox(imported.shape);
    for (let axis = 0; axis < 3; axis++) {
      assert.ok(Math.abs(bounds.max[axis] - [2, 3, 4][axis]) < 1e-4);
    }
  }
  await scope.end();
});

test("STEP transfer reports OCCT progress and cooperatively observes cancellation", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const shape = await scope.makeBox([10, 20, 30]);
  const exportProgress = [];
  const exported = await client.request("exportSTEP", { shape }, {
    onProgress: (event) => exportProgress.push(event.fraction),
  });
  assert.equal(exportProgress[0], 0);
  assert.equal(exportProgress.at(-1), 1);
  assert.ok(exportProgress.some((fraction) => fraction > 0 && fraction < 1));

  const controller = new AbortController();
  const importProgress = [];
  await assert.rejects(
    client.request("importSTEP", {
      scopeId: scope.scopeId,
      data: { $inputBuffer: exported.data.data.slice(0) },
    }, {
      signal: controller.signal,
      onProgress: (event) => {
        importProgress.push(event.fraction);
        if (event.fraction > 0 && event.fraction < 1) {
          controller.abort(new Error("STEP import cancelled"));
        }
      },
    }),
    /STEP import cancelled/,
  );
  assert.ok(importProgress.some((fraction) => fraction > 0 && fraction < 1));
  assert.equal((await client.stats()).liveShapeHandles, 1);

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("importSTEP rejects non-STEP payloads with ImportExportFailed", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  await assert.rejects(
    client.request("importSTEP", {
      scopeId: scope.scopeId,
      data: { $inputBuffer: new TextEncoder().encode("not a step file").buffer },
    }),
    (error) => error.code === "ImportExportFailed",
  );
  await scope.end();
});

test("imports OCCT's external screw STEP fixture with valid topology", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const fixture = await readFile(new URL("../../occt/data/step/screw.step", import.meta.url));
  const data = fixture.buffer.slice(fixture.byteOffset, fixture.byteOffset + fixture.byteLength);
  const imported = await scope.importSTEP(data, { unit: "mm" });

  assert.equal(imported.rootCount, 1);
  assert.equal(await client.shapeType(imported.shape), "solid");
  assert.equal(await client.isValid(imported.shape), true);
  const counts = await client.topologyCounts(imported.shape);
  assert.equal(counts.solid, 1);
  assert.equal(counts.shell, 1);
  assert.equal(counts.face, 10);
  assert.equal(counts.edge, 22);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});
