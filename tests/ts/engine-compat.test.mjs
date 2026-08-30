// Stage-6 compatibility-layer tests: ref recognition, recursive encoding,
// foreign-backend rejection, and shape-bearing result materialization.

import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";
import { GeometryEngine } from "../../dist/engine.js";
import { ParametricModel } from "../../dist/parametric.js";
import { Worker } from "node:worker_threads";
import { createWorkerProfileRuntime } from "../../dist/isolated-runtime.js";
import { RUNTIME_CONFIG } from "../../dist/index.js";
import {
  EngineCompatClient,
} from "../../dist/engine-compat.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const modules = RUNTIME_CONFIG.modules;
const artifacts = JSON.parse(await readFile(new URL("../../protocol/artifacts.json", import.meta.url), "utf8"));

function createNodeWorker() {
  const worker = new Worker(new URL("./node-worker.mjs", import.meta.url));
  return {
    postMessage: (message, transfer) => worker.postMessage(message, transfer),
    terminate: () => { void worker.terminate(); },
    addEventListener: (type, listener) => {
      if (type === "message") worker.on("message", (data) => listener({ data }));
      else worker.on("error", (error) => listener({ error, message: error.message }));
    },
  };
}

test("compat routes nested engine refs, materializes declared result handles, and rejects handles from another client", async () => {
  let nextShape = 10;
  let received;
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "compat-fam", async () => ({
    request: async (op, args) => {
      if (op === "beginScope") return { scopeId: 1 };
      if (op === "makeBox") return { shape: nextShape++ };
      if (op === "booleanFuse") {
        received = args;
        return {
          shape: 20,
          history: { retained: [], generated: [], modified: [], deleted: [] },
        };
      }
      return {};
    },
  }));
  const compat = new EngineCompatClient(engine);
  const scope = await compat.beginScope();
  const base = await scope.makeBox([1, 1, 1]);
  const tool = await scope.makeBox([2, 2, 2]);
  const result = await scope.booleanFuse(base, [tool], { includeHistory: true });
  assert.equal(received.base, 10);
  assert.deepEqual(received.tools, [11]);
  assert.equal(result.shape.constructor.name, "ShapeHandle");
  assert.deepEqual(result.history, { retained: [], generated: [], modified: [], deleted: [] });

  const direct = await DirectClient.create(wasm);
  const directScope = await direct.beginScope();
  const foreign = await directScope.makeBox([1, 1, 1]);
  await assert.rejects(compat.request("bbox", { shape: foreign }), TypeError);
  await directScope.end();
  await compat.close();
});

test("compat capabilities are derived from the registered manifest", async () => {
  const engine = new GeometryEngine(modules);
  const runtimeCapabilities = {
    protocolVersion: "1.2.0",
    kernelVersion: "kernel-test",
    occtVersion: "occt-test",
    ops: ["capabilities", "beginScope", "makeBox"],
    historySupport: { capabilities: "unsupported", beginScope: "unsupported", makeBox: "unsupported" },
    buildFlags: { wasmExceptions: true, source: "runtime" },
  };
  engine.registerProfile("core-modeling", "compat-fam", async () => ({
    request: async (operation) => operation === "capabilities" ? runtimeCapabilities : {},
  }));
  const compat = new EngineCompatClient(engine);
  const capabilities = await compat.initialize();
  assert.equal(capabilities.kernelVersion, "1.2.0");
  assert.equal(capabilities.occtVersion, "8.0.1+git.4c2c356490");
  assert.deepEqual(capabilities.buildFlags, { threads: false, simd: false, wasmExceptions: true });
  assert.ok(capabilities.ops.includes("makeBox"));
  assert.deepEqual(engine.stats().startedProfiles, []);
  await compat.close();
});

test("compat close invalidates scope handles even when engine shutdown fails", async () => {
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "compat-fam", async () => ({
    request: async (op) => op === "beginScope" ? { scopeId: 1 } : {},
    close: () => { throw new Error("shutdown failed"); },
  }));
  const compat = new EngineCompatClient(engine);
  const scope = await compat.beginScope();
  await scope.makeBox([1, 1, 1]);
  await assert.rejects(compat.close(), /shutdown failed/);
  await assert.rejects(scope.makeBox([1, 1, 1]), /expired kernel instance/);
});

test("compat scope cleanup continues after an independent shape release fails", async () => {
  let nextScope = 1;
  let nextShape = 1;
  let failedOnce = false;
  const released = [];
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "compat-fam", async () => ({
    request: async (operation, args) => {
      if (operation === "beginScope") return { scopeId: nextScope++ };
      if (operation === "makeBox") return { shape: nextShape++ };
      if (operation === "release") {
        released.push(args.shape);
        if (args.shape === 1 && !failedOnce) {
          failedOnce = true;
          throw new Error("temporary release failure");
        }
      }
      return {};
    },
  }));
  const compat = new EngineCompatClient(engine);
  const scope = await compat.beginScope();
  await scope.makeBox([1, 1, 1]);
  await scope.makeBox([2, 2, 2]);

  await assert.rejects(scope.end(), /temporary release failure/);
  assert.deepEqual(released, [1, 2]);
  assert.equal((await compat.stats()).liveShapeHandles, 1);
  await scope.end();
  assert.equal((await compat.stats()).liveShapeHandles, 0);
  await compat.close();
});

test("compat scope converges after releaseAll partially succeeds", async () => {
  let nextShape = 1;
  let failedOnce = false;
  const released = [];
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "compat-fam", async () => ({
    request: async (operation, args) => {
      if (operation === "beginScope") return { scopeId: 1 };
      if (operation === "makeBox") return { shape: nextShape++ };
      if (operation === "release") {
        released.push(args.shape);
        if (args.shape === 1 && !failedOnce) {
          failedOnce = true;
          throw new Error("temporary release failure");
        }
      }
      return {};
    },
  }));
  const compat = new EngineCompatClient(engine);
  const scope = await compat.beginScope();
  await scope.makeBox([1, 1, 1]);
  await scope.makeBox([2, 2, 2]);

  await assert.rejects(compat.releaseAll(), /temporary release failure/);
  assert.deepEqual(released, [1, 2]);
  await scope.end();
  assert.deepEqual(released, [1, 2, 1]);
  assert.equal((await compat.stats()).liveShapeHandles, 0);
  await compat.close();
});

test("compat requests preserve runtime request options", async () => {
  let operationOptions;
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "compat-fam", async () => ({
    request: async (operation, _args, options) => {
      if (operation === "beginScope") return { scopeId: 1 };
      if (operation === "makeBox") {
        operationOptions = options;
        return { shape: 1 };
      }
      return {};
    },
  }));
  const compat = new EngineCompatClient(engine);
  const controller = new AbortController();
  const onProgress = () => undefined;
  const options = {
    timeoutMs: 250,
    signal: controller.signal,
    onProgress,
    outputBuffers: "shared",
  };

  await compat.request("makeBox", { size: [1, 1, 1] }, options);
  assert.strictEqual(operationOptions, options);
  await compat.close();
});

test("EngineCompatClient is a real BaseClient for ParametricModel sketch and extrusion", async () => {
  const calls = [];
  const runtimeClient = await DirectClient.create(wasm);
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", "compat-fam", async () => ({
    request: async (op, args) => {
      calls.push(op);
      return runtimeClient.request(op, args);
    },
  }));
  const compat = new EngineCompatClient(engine);
  const model = new ParametricModel(compat, {
    parameters: { depth: 5 },
    features: [
      {
        id: "profile",
        type: "sketch",
        entities: [
          { id: "a", type: "line", start: [0, 0], end: [10, 0] },
          { id: "b", type: "line", start: [10, 0], end: [10, 8] },
          { id: "c", type: "line", start: [10, 8], end: [0, 8] },
          { id: "d", type: "line", start: [0, 8], end: [0, 0] },
        ],
        constraints: [],
      },
      { id: "face", type: "face", outer: "profile" },
      { id: "solid", type: "extrude", input: "face", vector: [0, 0, "depth"] },
    ],
  });
  await model.recompute();
  assert.equal(await compat.shapeType(model.getShape("solid")), "solid");
  assert.ok(Math.abs((await compat.massProps(model.getShape("solid"))).mass - 400) < 1e-6);
  assert.ok(calls.includes("makeEdgeLine"));
  assert.ok(calls.includes("extrude"));
  await model.dispose();
  assert.equal((await compat.stats()).liveShapeHandles, 0);
  await compat.close();
});

test("EngineCompatClient drives the same ParametricModel chain through an isolated Worker profile", async () => {
  const descriptor = { name: "core-modeling.wasm", ...artifacts.artifacts["core-modeling.wasm"] };
  const engine = new GeometryEngine(modules);
  engine.registerProfile("core-modeling", descriptor.buildFamily, createWorkerProfileRuntime({
    artifact: descriptor,
    createWorker: createNodeWorker,
    loadArtifact: () => readFile(new URL("../../artifacts/core-modeling.wasm", import.meta.url))
      .then((bytes) => bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength)),
  }));
  const compat = new EngineCompatClient(engine);
  const model = new ParametricModel(compat, {
    parameters: { depth: 5 },
    features: [
      {
        id: "profile",
        type: "sketch",
        entities: [
          { id: "a", type: "line", start: [0, 0], end: [10, 0] },
          { id: "b", type: "line", start: [10, 0], end: [10, 8] },
          { id: "c", type: "line", start: [10, 8], end: [0, 8] },
          { id: "d", type: "line", start: [0, 8], end: [0, 0] },
        ],
        constraints: [],
      },
      { id: "face", type: "face", outer: "profile" },
      { id: "solid", type: "extrude", input: "face", vector: [0, 0, "depth"] },
    ],
  });

  try {
    await model.recompute();
    assert.equal(await compat.shapeType(model.getShape("solid")), "solid");
    assert.ok(Math.abs((await compat.massProps(model.getShape("solid"))).mass - 400) < 1e-6);
  } finally {
    await model.dispose();
    await compat.close();
  }
});
