import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { Worker } from "node:worker_threads";
import test from "node:test";
import { GeometryEngine } from "../../dist/engine.js";
import { createWorkerProfileRuntime } from "../../dist/isolated-runtime.js";
import { PROFILE_OPERATIONS, RUNTIME_CONFIG } from "../../dist/index.js";

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

function createCapabilityWorker(ops) {
  const listeners = { message: [], error: [] };
  let terminated = false;
  const emit = (data) => {
    for (const listener of listeners.message) listener({ data });
  };
  return {
    postMessage: (message) => {
      if (message.type === "init") {
        queueMicrotask(() => emit({ type: "ready" }));
        return;
      }
      if (message.op === "capabilities") {
        queueMicrotask(() => emit({
          type: "response",
          id: message.id,
          ok: true,
          result: {
            protocolVersion: "1.2.0",
            kernelVersion: "test",
            occtVersion: "test",
            ops,
            historySupport: Object.fromEntries(ops.map((operation) => [operation, "unsupported"])),
            buildFlags: { threads: false, simd: false, wasmExceptions: true },
          },
        }));
      }
    },
    terminate: () => { terminated = true; },
    addEventListener: (type, listener) => { listeners[type].push(listener); },
    get terminated() { return terminated; },
  };
}

test("isolated profile initialization rejects both capability difference directions", async () => {
  const bytes = await readFile(new URL("../../artifacts/preview.wasm", import.meta.url));
  const operations = [
    ...PROFILE_OPERATIONS.preview.filter((operation) => operation !== "bbox"),
    "fillet",
  ];
  const workers = [];
  const runtimeFactory = createWorkerProfileRuntime({
    profile: "preview",
    createWorker: () => {
      const worker = createCapabilityWorker(operations);
      workers.push(worker);
      return worker;
    },
    loadArtifact: async () => bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength),
  });

  await assert.rejects(
    runtimeFactory(),
    (error) => /declared but not registered: bbox/.test(error.message)
      && /registered but not declared: fillet/.test(error.message),
  );
  assert.equal(workers[0].terminated, true);
});

test("createWorkerProfileRuntime drives a real isolated profile worker", async () => {
  const profile = RUNTIME_CONFIG.profiles["core-modeling"];
  const runtimeFactory = createWorkerProfileRuntime({
    profile: "core-modeling",
    createWorker: createNodeWorker,
    loadArtifact: () => readFile(new URL("../../artifacts/core-modeling.wasm", import.meta.url))
      .then((bytes) => bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength)),
  });
  const engine = new GeometryEngine();
  engine.registerProfile(profile.profileId, profile.buildFamily, runtimeFactory);

  try {
    const box = await engine.create("core-modeling", "makeBox", { size: [2, 3, 4] });
    const bbox = await engine.request("bbox", { shape: box });
    assert.deepEqual(bbox.max.map(Math.round), [2, 3, 4]);
    assert.deepEqual(engine.stats().startedProfiles, ["core-modeling"]);
  } finally {
    await engine.close();
  }
});

test("modeling-viewer profile loads and runs the mesh artifact", async () => {
  const profile = RUNTIME_CONFIG.profiles["modeling-viewer"];
  let loadedArtifact;
  const runtimeFactory = createWorkerProfileRuntime({
    profile: "modeling-viewer",
    createWorker: createNodeWorker,
    loadArtifact: (url) => {
      loadedArtifact = url;
      return readFile(new URL("../../artifacts/mesh.wasm", import.meta.url))
        .then((bytes) => bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength));
    },
  });
  const engine = new GeometryEngine(RUNTIME_CONFIG.modules);
  engine.registerProfile(profile.profileId, profile.buildFamily, runtimeFactory);

  try {
    const box = await engine.create("modeling-viewer", "makeBox", { size: [2, 3, 4] });
    assert.deepEqual((await engine.request("bbox", { shape: box })).max.map(Math.round), [2, 3, 4]);
    assert.match(loadedArtifact, /mesh\.wasm$/);
    assert.equal(profile.artifact.name, "mesh.wasm");
  } finally {
    await engine.close();
  }
});

test("real isolated profile workers support clone transfer and source survival", async () => {
  const engine = new GeometryEngine();
  const register = (profileId) => {
    const profile = RUNTIME_CONFIG.profiles[profileId];
    engine.registerProfile(profile.profileId, profile.buildFamily, createWorkerProfileRuntime({
      profile: profileId,
      createWorker: createNodeWorker,
      loadArtifact: () => readFile(new URL(`../../artifacts/${profile.artifact.name}`, import.meta.url))
        .then((bytes) => bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength)),
    }));
  };
  register("core-modeling");
  register("mesh");

  try {
    const source = await engine.create("core-modeling", "makeBox", { size: [2, 3, 4] });
    const target = await source.cloneTo("mesh");
    assert.deepEqual((await engine.request("bbox", { shape: source })).max.map(Math.round), [2, 3, 4]);
    assert.deepEqual((await engine.request("bbox", { shape: target })).max.map(Math.round), [2, 3, 4]);
    assert.deepEqual(engine.stats().startedProfiles.sort(), ["core-modeling", "mesh"]);
  } finally {
    await engine.close();
  }
});
