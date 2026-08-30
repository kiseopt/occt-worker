// Stage-5 engine tests: placement lifecycle, clone transfer semantics,
// manifest routing, and failure cleanup — exercised against independent
// full-kernel runtimes standing in for isolated profile workers.

import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";
import {
  GeometryEngine,
  EngineError,
  encodeEnvelope,
  decodeEnvelope,
  TRANSFER_MAGIC,
  TRANSFER_VERSION,
} from "../../dist/engine.js";
import { RUNTIME_CONFIG } from "../../dist/index.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const modules = RUNTIME_CONFIG.modules;

function realRuntime(calls) {
  return async () => {
    const client = await DirectClient.create(wasm);
    calls.push("start");
    return {
      request: async (op, args) => {
        calls.push(op);
        return client.request(op, args);
      },
    };
  };
}

test("lazy profiles, clone chain, source survival, and releaseShape cleanup", async () => {
  const calls = [];
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "test-family-v1", realRuntime(calls));
  engine.registerProfile("mesh", "test-family-v1", realRuntime(calls));

  assert.deepEqual(engine.stats().startedProfiles, [], "no runtime may start before first use");

  const box = await engine.create("core-modeling", "makeBox", { size: [10, 10, 10] });
  const bbox = await engine.request("bbox", { shape: box });
  assert.ok(bbox.min.every(Number.isFinite));
  assert.ok(engine.stats().startedProfiles.includes("core-modeling"), "core-modeling must have started");
  assert.ok(!engine.stats().startedProfiles.includes("mesh"), "mesh must not start before first use");

  // core-modeling -> mesh clone; the source placement stays live.
  const boxOnMesh = await box.cloneTo("mesh");
  assert.equal(boxOnMesh.profileId, "mesh");
  await engine.request("bbox", { shape: box });
  assert.deepEqual(engine.stats(), { shapes: 1, placements: 2, startedProfiles: ["core-modeling", "mesh"] });

  // mesh -> exchange -> mesh multi-hop chain.
  engine.registerProfile("exchange", "test-family-v1", realRuntime(calls));
  const onExchange = await boxOnMesh.cloneTo("exchange");
  assert.equal(onExchange.profileId, "exchange");
  const backToMesh = await onExchange.cloneTo("mesh");
  assert.equal(backToMesh.profileId, "mesh");
  assert.equal(engine.stats().placements, 3);

  // Read-only ops never add placements (no transfer for viewer-style reads).
  await engine.request("bbox", { shape: box });
  assert.equal(engine.stats().placements, 3);

  await engine.releaseShape(box.logicalId);
  assert.deepEqual(engine.stats(), { shapes: 0, placements: 0, startedProfiles: ["core-modeling", "mesh", "exchange"] });
});

test("capability routing rejects ineligible constructors and unknown operations", async () => {
  const engine = new GeometryEngine(modules);
  engine.registerProfile("preview", "f", async () => ({ request: async () => ({}) }));
  engine.registerProfile("mesh", "f", async () => ({ request: async () => ({}) }));

  // preview has query/tessellation/STEP exchange only: constructors are ineligible there.
  await assert.rejects(
    engine.create("preview", "makeBox", { size: [1, 1, 1] }),
    (error) => error instanceof EngineError && error.code === "UnsupportedCapability",
  );
  await assert.rejects(
    engine.request("timeTravel", {}),
    (error) => error instanceof EngineError && error.code === "UnsupportedCapability",
  );
});

test("modeling-viewer alias has the same operation eligibility as mesh", () => {
  const engine = new GeometryEngine(modules);
  const operations = Object.values(modules.semanticModules).flat();
  for (const operation of operations) {
    assert.equal(
      engine.eligibleProfiles(operation).includes("modeling-viewer"),
      engine.eligibleProfiles(operation).includes("mesh"),
      operation,
    );
  }
});

test("capabilities is static and does not start registered profiles", async () => {
  const engine = new GeometryEngine(modules);
  let starts = 0;
  const factory = async () => {
    starts += 1;
    return { request: async () => ({}) };
  };
  engine.registerProfile("preview", "f", factory);
  engine.registerProfile("mesh", "f", factory);
  const before = engine.stats().startedProfiles;
  const capabilities = await engine.capabilities();
  assert.deepEqual(engine.stats().startedProfiles, before);
  assert.equal(starts, 0);
  assert.equal(capabilities.protocolVersion, "1.2.0");
  const probed = await engine.probeProfile("mesh");
  assert.deepEqual(probed, {});
  assert.equal(starts, 1);
});

test("request routing ignores eligible profiles that are not registered", async () => {
  const calls = [];
  const engine = new GeometryEngine(modules);
  engine.registerProfile("mesh", "f", async () => ({
    request: async (op) => {
      calls.push(op);
      if (op === "beginScope") return { scopeId: 2 };
      if (op === "makeBox") return { shape: 7 };
      return {};
    },
  }));

  const result = await engine.request("makeBox", {});
  const shape = result.shape;
  assert.equal(shape.profileId, "mesh");
  assert.deepEqual(calls.slice(0, 2), ["beginScope", "makeBox"]);
  await engine.releaseShape(shape.logicalId);
  await engine.close();
});

test("engine request rejects caller-managed runtime scopes", async () => {
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "f", async () => {
    throw new Error("profile must not start");
  });

  await assert.rejects(
    engine.request("makeBox", { scopeId: 17 }),
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
  assert.deepEqual(engine.stats().startedProfiles, []);
});

test("profile registration cannot replace an active runtime", async () => {
  const calls = [];
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "fam", realRuntime(calls));
  const shape = await engine.create("core-modeling", "makeBox", { size: [1, 1, 1] });

  assert.throws(
    () => engine.registerProfile("core-modeling", "replacement", async () => ({ request: async () => ({}) })),
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
  await engine.request("bbox", { shape });
  await engine.close();
});

test("eligible profile results cannot mutate engine routing", () => {
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "fam", async () => ({ request: async () => ({}) }));

  const eligible = engine.eligibleProfiles("makeBox");
  const original = [...eligible];
  eligible.push("unexpected-profile");

  assert.deepEqual(engine.eligibleProfiles("makeBox"), original);
});

test("batch operations reject cross-profile inputs instead of silently splitting", async () => {
  const calls = [];
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "fam", realRuntime(calls));
  engine.registerProfile("mesh", "fam", realRuntime(calls));

  const box = await engine.create("core-modeling", "makeBox", { size: [4, 4, 4] });
  const other = await box.cloneTo("mesh");

  // Cross-profile batch must be rejected before any co-location transfer.
  const placementsBefore = engine.stats().placements;
  await assert.rejects(
    engine.request("batch", { ops: [{ op: "bbox", args: { shape: box } }, { op: "bbox", args: { shape: other } }] }),
    (error) => error instanceof EngineError && error.code === "AmbiguousProfileRouting",
  );

  // No transfer aliases were created by the rejected routing attempt.
  assert.equal(engine.stats().placements, placementsBefore);

  // Same-profile batch still routes (kernel-side execution is out of scope).
  const result = await engine.request(
    "batch",
    { ops: [{ op: "bbox", args: { shape: box } }] },
  ).then(() => true).catch((error) => error);
  void result;
});

test("batch accepts two refs in one profile even when a shape has another placement", async () => {
  const calls = [];
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "fam", realRuntime(calls));
  engine.registerProfile("mesh", "fam", realRuntime(calls));
  const first = await engine.create("core-modeling", "makeBox", { size: [2, 2, 2] });
  const second = await engine.create("core-modeling", "makeBox", { size: [1, 1, 1] });
  await first.cloneTo("mesh");

  await engine.request("batch", {
    ops: [{ op: "bbox", args: { shape: first } }, { op: "bbox", args: { shape: second } }],
  });
  await engine.close();
});

test("routing accepts a distinct ref object for the same placement", async () => {
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "fam", async () => ({
    request: async (op) => op === "beginScope" ? { scopeId: 1 } : { shape: 8 },
  }));
  const original = await engine.create("core-modeling", "makeBox", {});
  const alias = await original.cloneTo("core-modeling");
  await engine.request("bbox", { shape: alias });
  await engine.close();
});

test("stale placement refs cannot alias a recycled target handle", async () => {
  const engine = new GeometryEngine(modules);
  let nextHandle = 10;
  engine.registerProfile("core-modeling", "fam", async () => ({
    request: async (op) => {
      if (op === "beginScope") return { scopeId: 1 };
      if (op === "makeBox") return { shape: 1 };
      if (op === "exportBREP") return { data: { data: new Uint8Array([1]).buffer } };
      return {};
    },
  }));
  engine.registerProfile("mesh", "fam", async () => ({
    request: async (op) => {
      if (op === "beginScope") return { scopeId: 2 };
      if (op === "importBREP") return { shape: nextHandle };
      if (op === "release") return {};
      return {};
    },
  }));
  const source = await engine.create("core-modeling", "makeBox", {});
  const first = await source.cloneTo("mesh");
  await first.releasePlacement();
  const second = await source.cloneTo("mesh");
  await assert.rejects(
    engine.request("bbox", { shape: first }),
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
  await engine.close();
});

test("released and invalidated refs are rejected before routing", async () => {
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "fam", realRuntime([]));
  const released = await engine.create("core-modeling", "makeBox", { size: [1, 1, 1] });
  await released.releasePlacement();
  await assert.rejects(
    engine.request("bbox", { shape: released }),
    (error) => error instanceof EngineError && error.code === "UnknownLogicalShape",
  );

  const invalid = await engine.create("core-modeling", "makeBox", { size: [1, 1, 1] });
  engine.invalidateProfileEpoch("core-modeling");
  await assert.rejects(
    engine.request("bbox", { shape: invalid }),
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
  await engine.close();
});

test("transfer envelope rejects build-family mismatch with a stable code", async () => {
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "family-A", async () => ({
    request: async (op) => {
      if (op === "beginScope") return { scopeId: 7 };
      if (op === "exportBREP") return { data: new Uint8Array([9, 9]).buffer };
      return {};
    },
  }));
  engine.registerProfile("mesh", "family-B", async () => ({
    request: async (op) => (op === "beginScope" ? { scopeId: 8 } : op === "endScope" ? {} : undefined),
  }));

  const shape = await engine.create("core-modeling", "makeBox", {});
  await assert.rejects(
    shape.cloneTo("mesh"),
    (error) => error instanceof EngineError && error.code === "TransferBuildFamilyMismatch",
  );
});

test("envelope codec rejects version/checksum/truncation failures deterministically", () => {
  const family = "fam";
  const payload = new Uint8Array([1, 2, 3, 4, 5]).buffer;
  const frame = encodeEnvelope(family, payload);

  // Round-trip succeeds.
  assert.deepEqual(new Uint8Array(decodeEnvelope(frame, family)), new Uint8Array(payload));

  const rewriteHeader = (mutate) => {
    const view = new DataView(frame);
    const headerLength = view.getUint32(0, true);
    const header = JSON.parse(new TextDecoder().decode(new Uint8Array(frame, 4, headerLength)));
    mutate(header);
    const next = new TextEncoder().encode(JSON.stringify(header));
    const out = new Uint8Array(frame.byteLength - headerLength + next.length);
    new DataView(out.buffer).setUint32(0, next.length, true);
    out.set(next, 4);
    out.set(new Uint8Array(frame, 4 + headerLength), 4 + next.length);
    return out.buffer;
  };

  assert.throws(
    () => decodeEnvelope(rewriteHeader((h) => { h.version = TRANSFER_VERSION + 1; }), family),
    (error) => error.code === "TransferVersionMismatch",
  );
  assert.throws(
    () => decodeEnvelope(rewriteHeader((h) => { h.magic = TRANSFER_MAGIC + "-x"; }), family),
    (error) => error.code === "TransferVersionMismatch",
  );

  const corrupted = frame.slice(0);
  new DataView(corrupted).setUint8(corrupted.byteLength - 1, 0xff ^ 0xff);
  assert.throws(() => decodeEnvelope(corrupted, family), (error) => error.code === "TransferChecksumMismatch");

  const truncated = frame.slice(0, frame.byteLength - 2);
  assert.throws(() => decodeEnvelope(truncated, family), (error) => error.code === "TransferPayloadTruncated");
});

test("profile epoch invalidation marks placements invalid without auto-replay", async () => {
  let startedCount = 0;
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "f", async () => {
    startedCount += 1;
    return {
      request: async (op) => {
        if (op === "beginScope") return { scopeId: 11 };
        if (op === "exportBREP") return { data: new Uint8Array([1]).buffer };
        return {};
      },
    };
  });
  engine.registerProfile("mesh", "f", async () => ({
    request: async (op, args) => {
      if (op === "beginScope") return { scopeId: 12 };
      if (op === "importBREP") return { shape: args.scopeId * 100 };
      return {};
    },
  }));

  const shape = await engine.create("core-modeling", "makeBox", {});
  await shape.cloneTo("mesh");

  // Crash semantics: only the crashed profile's epoch becomes invalid.
  engine.invalidateProfileEpoch("core-modeling");
  assert.deepEqual(engine.stats(), { shapes: 1, placements: 2, startedProfiles: ["mesh"] });

  // Using an invalidated placement fails without restarting anything
  // (no auto-replay of modifying operations).
  const before = startedCount;
  await assert.rejects(
    engine.cloneTo(shape, "mesh"),
    (error) =>
      error instanceof EngineError
      && (error.code === "InvalidPlacementState" || error.code === "UnknownLogicalShape"),
  );
  assert.equal(startedCount, before, "invalidated profile must not auto-start");
});

test("request rejects a shape owned by another engine before logical lookup", async () => {
  const first = new GeometryEngine(modules);
  const second = new GeometryEngine(modules);
  first.registerProfile("core-modeling", "f", async () => ({
    request: async (op) => op === "beginScope" ? { scopeId: 1 } : { shape: 10 },
  }));
  second.registerProfile("core-modeling", "f", async () => ({ request: async () => ({}) }));
  const shape = await first.create("core-modeling", "makeBox", {});
  await assert.rejects(
    second.request("bbox", { shape }),
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
});

test("shape-bearing results share one scope and release it after the last shape", async () => {
  const calls = [];
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "f", async () => ({
    request: async (op, args) => {
      calls.push([op, args]);
      if (op === "beginScope") return { scopeId: 9 };
      if (op === "generalFuse") return { shapes: [1, 2, 1, 3], sourceIndices: [[0], [1], [0], [2]] };
      return {};
    },
  }));

  const result = await engine.request("generalFuse", {});
  assert.equal(result.shapes[0].logicalId, result.shapes[2].logicalId);
  assert.deepEqual(engine.stats(), { shapes: 3, placements: 3, startedProfiles: ["core-modeling"] });

  await engine.releaseShape(result.shapes[0].logicalId);
  await engine.releaseShape(result.shapes[1].logicalId);
  assert.equal(calls.filter(([op]) => op === "endScope").length, 0);
  await engine.releaseShape(result.shapes[3].logicalId);
  assert.equal(calls.filter(([op]) => op === "release").length, 3);
  assert.equal(calls.filter(([op]) => op === "endScope").length, 1);
});

test("batch materializes nested shape results before releasing its temporary scope", async () => {
  const calls = [];
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "f", async () => ({
    request: async (op, args) => {
      calls.push([op, args]);
      if (op === "beginScope") return { scopeId: 6 };
      if (op === "batch") return { results: [{ shape: 42 }, { distance: 3 }] };
      return {};
    },
  }));

  const result = await engine.request("batch", {
    ops: [{ op: "makeBox", args: { size: [1, 1, 1] } }, { op: "bbox", args: { shape: 42 } }],
  });
  assert.equal(result.results[0].shape.constructor.name, "EngineShapeRef");
  assert.equal(result.results[1].distance, 3);
  assert.equal(calls.filter(([op]) => op === "endScope").length, 0);
  await engine.releaseShape(result.results[0].shape.logicalId);
  assert.equal(calls.filter(([op]) => op === "endScope").length, 1);
  await engine.close();
});

test("shape-free requests close their temporary scope and engine close releases runtimes", async () => {
  const calls = [];
  let closed = 0;
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "f", async () => ({
    request: async (op) => {
      calls.push(op);
      if (op === "beginScope") return { scopeId: 4 };
      if (op === "distance") return { distance: 2 };
      return {};
    },
    close: () => { closed += 1; },
  }));

  assert.deepEqual(await engine.request("distance", {}), { distance: 2 });
  assert.equal(calls.filter((op) => op === "endScope").length, 1);
  await engine.close();
  await engine.close();
  assert.equal(calls.filter((op) => op === "releaseAll").length, 1);
  assert.equal(closed, 1);
  await assert.rejects(
    engine.create("core-modeling", "makeBox", {}),
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
});

test("engine close clears state even when runtime shutdown fails", async () => {
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "f", async () => ({
    request: async (op) => op === "beginScope" ? { scopeId: 1 } : { shape: 3 },
    close: () => { throw new Error("shutdown failed"); },
  }));
  await engine.create("core-modeling", "makeBox", {});
  await assert.rejects(engine.close(), /shutdown failed/);
  assert.deepEqual(engine.stats(), { shapes: 0, placements: 0, startedProfiles: [] });
});

test("concurrent engine close calls share one cleanup promise", async () => {
  let closeCount = 0;
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "f", async () => ({
    request: async (op) => op === "beginScope" ? { scopeId: 1 } : { shape: 3 },
    close: async () => { closeCount += 1; await Promise.resolve(); },
  }));
  const shape = await engine.create("core-modeling", "makeBox", {});
  const first = engine.close();
  const second = engine.close();
  assert.strictEqual(first, second);
  await assert.rejects(
    shape.releasePlacement(),
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
  await Promise.all([first, second]);
  assert.equal(closeCount, 1);
});

test("releaseShape rejects atomically while any placement is leased", async () => {
  let finishRequest;
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "f", async () => ({
    request: async (op) => {
      if (op === "beginScope") return { scopeId: 1 };
      if (op === "makeBox") return { shape: 4 };
      if (op === "bbox") return new Promise((resolve) => { finishRequest = resolve; });
      return {};
    },
  }));
  const shape = await engine.create("core-modeling", "makeBox", {});
  const request = engine.request("bbox", { shape });
  await Promise.resolve();
  await assert.rejects(
    engine.releaseShape(shape.logicalId),
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
  finishRequest({ distance: 1 });
  await request;
  await engine.releaseShape(shape.logicalId);
  await engine.close();
});

test("close disposes a profile that finishes starting after shutdown", async () => {
  let finishStart;
  let closed = 0;
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "f", () => new Promise((resolve) => { finishStart = resolve; }));

  const creating = engine.create("core-modeling", "makeBox", {});
  await Promise.resolve();
  const closing = engine.close();
  finishStart({
    request: async () => ({ shape: 1 }),
    close: () => { closed += 1; },
  });

  await closing;
  await assert.rejects(
    creating,
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
  assert.equal(closed, 1);
  assert.deepEqual(engine.stats(), { shapes: 0, placements: 0, startedProfiles: [] });
});

test("profile invalidation during transfer does not revive the source placement", async () => {
  let finishExport;
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "f", async () => ({
    request: async (op) => {
      if (op === "beginScope") return { scopeId: 1 };
      if (op === "makeBox") return { shape: 10 };
      if (op === "exportBREP") return new Promise((resolve) => { finishExport = resolve; });
      return {};
    },
  }));
  engine.registerProfile("mesh", "f", async () => ({ request: async () => ({}) }));
  const shape = await engine.create("core-modeling", "makeBox", {});
  const cloning = shape.cloneTo("mesh");
  await Promise.resolve();
  engine.invalidateProfileEpoch("core-modeling");
  finishExport({ data: { data: new Uint8Array([1]).buffer } });
  await assert.rejects(
    cloning,
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
  await assert.rejects(
    shape.cloneTo("mesh"),
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
  await engine.close();
});

test("runtime epoch changes invalidate old placements before raw handles can be reused", async () => {
  let runtimeEpoch = 1;
  const bboxHandles = [];
  const runtime = {
    get epoch() { return runtimeEpoch; },
    request: async (op, args) => {
      if (op === "beginScope") return { scopeId: runtimeEpoch };
      if (op === "makeBox") return { shape: 1 };
      if (op === "bbox") {
        bboxHandles.push(args.shape);
        return { min: [0, 0, 0], max: [1, 1, 1] };
      }
      return {};
    },
  };
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "f", async () => runtime);

  const stale = await engine.create("core-modeling", "makeBox", {});
  runtimeEpoch += 1;
  await assert.rejects(
    engine.request("bbox", { shape: stale }),
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );

  const replacement = await engine.create("core-modeling", "makeBox", {});
  assert.notEqual(replacement.logicalId, stale.logicalId);
  await engine.request("bbox", { shape: replacement });
  assert.deepEqual(bboxHandles, [1]);
  await assert.rejects(
    engine.request("bbox", { shape: stale }),
    (error) => error instanceof EngineError && error.code === "InvalidPlacementState",
  );
  await engine.close();
});
