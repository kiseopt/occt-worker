import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { Worker } from "node:worker_threads";
import test from "node:test";
import { GeometryEngine } from "../../dist/engine.js";
import { createWorkerProfileRuntime } from "../../dist/isolated-runtime.js";
import { BUILD_IDENTITY, PROFILE_OPERATIONS, RUNTIME_CONFIG } from "../../dist/index.js";

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

test("custom artifact URL loads the real full wasm and reports capabilities", async () => {
  const official = RUNTIME_CONFIG.profiles["full-profile"].artifact;
  let resolvedUrl;
  const runtimeFactory = createWorkerProfileRuntime({
    artifact: {
      name: "vendor-full.wasm",
      sha256: official.sha256,
      protocolVersion: BUILD_IDENTITY.protocolVersion,
      abiVersion: BUILD_IDENTITY.pluginAbiVersion,
      buildFamily: BUILD_IDENTITY.buildFamilies.isolated,
    },
    resolve: () => {
      resolvedUrl = new URL("../../artifacts/full.wasm", import.meta.url);
      return resolvedUrl;
    },
    createWorker: createNodeWorker,
    loadArtifact: (url) => readFile(new URL(url))
      .then((bytes) => bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength)),
  });

  const runtime = await runtimeFactory();
  try {
    const capabilities = await runtime.request("capabilities", {});
    assert.equal(capabilities.protocolVersion, BUILD_IDENTITY.protocolVersion);
    assert.ok(capabilities.ops.includes("makeBox"));
    assert.ok(capabilities.ops.includes("importSTEP"));
  } finally {
    await runtime.close();
  }
  assert.match(resolvedUrl.href, /artifacts\/full\.wasm$/);
});

test("custom artifact descriptor rejects declared identity mismatches", async () => {
  const cases = [
    ["protocolVersion", "0.0.0", /declares protocol version/],
    ["abiVersion", BUILD_IDENTITY.pluginAbiVersion + 1, /declares ABI version/],
    ["buildFamily", "vendor-family", /declares build family/],
  ];
  let workerCreated = false;
  for (const [field, value, message] of cases) {
    const runtimeFactory = createWorkerProfileRuntime({
      artifact: { name: `invalid-${field}.wasm`, [field]: value },
      resolve: () => "https://example.test/custom.wasm",
      createWorker: () => {
        workerCreated = true;
        return createCapabilityWorker([]);
      },
      loadArtifact: async () => new ArrayBuffer(0),
    });
    await assert.rejects(runtimeFactory(), message);
  }
  assert.equal(workerCreated, false);
});

test("custom artifact descriptor verifies supplied hash before starting a worker", async () => {
  let workerCreated = false;
  const runtimeFactory = createWorkerProfileRuntime({
    artifact: { name: "invalid-hash.wasm", sha256: "00".repeat(32) },
    resolve: () => "https://example.test/custom.wasm",
    createWorker: () => {
      workerCreated = true;
      return createCapabilityWorker([]);
    },
    loadArtifact: async () => new TextEncoder().encode("not wasm").buffer,
  });
  await assert.rejects(runtimeFactory(), /SHA-256 mismatch/);
  assert.equal(workerCreated, false);
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
