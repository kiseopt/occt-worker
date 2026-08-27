import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { Worker } from "node:worker_threads";
import test from "node:test";
import { GeometryEngine } from "../../dist/engine.js";
import { createWorkerProfileRuntime } from "../../dist/isolated-runtime.js";
import { RUNTIME_CONFIG } from "../../dist/index.js";

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
