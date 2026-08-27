// Shared loader semantics tests, driven against the dylink spike artifacts so
// the concurrency/failure/epoch rules are validated on real dynamic linking
// without requiring the PIC OCCT build.
//
// One Main instantiation per process: Emscripten keeps some dylink state
// global to the JS realm, mirroring the production rule of one shared kernel
// per Worker.

import assert from "node:assert/strict";
import test from "node:test";
import { SharedKernelLoader, SharedKernelLoadError } from "../../dist/shared-loader.js";

test("shared loader deduplicates concurrent side loads and survives failures", async () => {
  let registrations = 0;
  let loadCalls = 0;
  let pluginName = "";
  const module = {
    UTF8ToString: (pointer) => pointer === 1 ? "spike-dylink-v1" : pluginName,
    _occt_host_build_family: () => 1,
    _occt_host_plugin_count: () => registrations,
    _occt_host_plugin_error: () => 0,
    _occt_host_plugin_name: () => 2,
    loadDynamicLibrary: async (url) => {
      loadCalls += 1;
      if (url === "missing-side.wasm") throw new Error("missing side");
      await Promise.resolve();
      pluginName = url.replace(/\.wasm$/, "");
      registrations += 1;
      return true;
    },
  };
  const loader = new SharedKernelLoader({
    importFactory: async () => async () => module,
    mainWasmUrl: "dylink-main.wasm",
  });

  await loader.instantiate("dylink-main.mjs");
  const epochAfterInstantiate = loader.epoch;
  await assert.rejects(
    loader.instantiate("dylink-main.mjs"),
    (error) => error instanceof SharedKernelLoadError && error.code === "MainAlreadyInstantiated",
  );

  // A failed load surfaces a structured error and clears the reserved slot
  // before any successful load happens.
  await assert.rejects(
    loader.ensureSide({ name: "missing", url: "missing-side.wasm" }, "spike-dylink-v1"),
    (error) => error instanceof SharedKernelLoadError && error.code === "SideLoadFailed" && error.sideName === "missing",
  );
  assert.deepEqual(loader.loadedSides, []);

  // Concurrent first loads share one in-flight promise.
  const spike = { name: "dylink-side", url: "dylink-side.wasm" };
  await Promise.all([
    loader.ensureSide(spike, "spike-dylink-v1"),
    loader.ensureSide(spike, "spike-dylink-v1"),
  ]);
  assert.deepEqual(loader.loadedSides, ["dylink-side"]);
  assert.equal(loadCalls, 2, "one failed load and one deduplicated successful load");
  await assert.rejects(
    loader.ensureSide({ name: "dylink-side", url: "different-side.wasm" }, "spike-dylink-v1"),
    (error) => error instanceof SharedKernelLoadError && error.code === "SideLoadFailed",
  );
  assert.equal(loader.epoch, epochAfterInstantiate);

  // Dispose bumps the epoch and forgets every side.
  loader.dispose();
  assert.equal(loader.epoch, epochAfterInstantiate + 1);
  assert.deepEqual(loader.loadedSides, []);
});

test("shared loader serializes different side registrations", async () => {
  let registrations = 0;
  let loadCalls = 0;
  let pluginName = "";
  const module = {
    UTF8ToString: (pointer) => pointer === 1 ? "family-a" : pluginName,
    _occt_host_build_family: () => 1,
    _occt_host_plugin_count: () => registrations,
    _occt_host_plugin_error: () => 0,
    _occt_host_plugin_name: () => 2,
    loadDynamicLibrary: async (url) => {
      loadCalls += 1;
      await Promise.resolve();
      pluginName = url.replace(/\.wasm$/, "");
      registrations += 1;
      return true;
    },
  };
  const loader = new SharedKernelLoader({
    importFactory: async () => async () => module,
    mainWasmUrl: "main.wasm",
  });
  await loader.instantiate("main.mjs");

  await Promise.all([
    loader.ensureSide({ name: "first", url: "first.wasm" }, "family-a"),
    loader.ensureSide({ name: "second", url: "second.wasm" }, "family-a"),
  ]);

  assert.equal(loadCalls, 2);
  assert.deepEqual(loader.loadedSides, ["first", "second"]);
});

test("shared loader rejects build-family and registration failures", async () => {
  const makeLoader = (module) => new SharedKernelLoader({
    importFactory: async () => async () => module,
    mainWasmUrl: "main.wasm",
  });
  const baseModule = {
    UTF8ToString: (pointer) => pointer === 1 ? "family-a" : "side",
    _occt_host_build_family: () => 1,
    _occt_host_plugin_count: () => 0,
    _occt_host_plugin_error: () => 0,
    _occt_host_plugin_name: () => 2,
    loadDynamicLibrary: async () => true,
  };

  const mismatch = makeLoader(baseModule);
  await mismatch.instantiate("main.mjs");
  await assert.rejects(
    mismatch.ensureSide({ name: "side", url: "side.wasm" }, "family-b"),
    (error) => error instanceof SharedKernelLoadError && error.code === "BuildFamilyMismatch",
  );

  const registration = makeLoader({ ...baseModule, _occt_host_plugin_error: () => 1 });
  await registration.instantiate("main.mjs");
  await assert.rejects(
    registration.ensureSide({ name: "side", url: "side.wasm" }, "family-a"),
    (error) => error instanceof SharedKernelLoadError && error.code === "RegistrationFailed",
  );

  const failedLoad = makeLoader({ ...baseModule, loadDynamicLibrary: async () => false });
  await failedLoad.instantiate("main.mjs");
  await assert.rejects(
    failedLoad.ensureSide({ name: "side", url: "side.wasm" }, "family-a"),
    (error) => error instanceof SharedKernelLoadError && error.code === "SideLoadFailed",
  );

  let wrongSideRegistrations = 0;
  const wrongSide = makeLoader({
    ...baseModule,
    _occt_host_plugin_count: () => wrongSideRegistrations,
    UTF8ToString: (pointer) => pointer === 1 ? "family-a" : "other-side",
    loadDynamicLibrary: async () => {
      wrongSideRegistrations += 1;
      return true;
    },
  });
  await wrongSide.instantiate("main.mjs");
  await assert.rejects(
    wrongSide.ensureSide({ name: "side", url: "side.wasm" }, "family-a"),
    (error) => error instanceof SharedKernelLoadError
      && error.code === "RegistrationFailed"
      && error.message.includes("check the side URL"),
  );
});

test("shared loader retries synchronous validation failures and rejects stale epoch loads", async () => {
  const makeLoader = (module) => new SharedKernelLoader({
    importFactory: async () => async () => module,
    mainWasmUrl: "main.wasm",
  });
  let family = "wrong";
  let resolveLoad;
  const module = {
    UTF8ToString: (pointer) => pointer === 1 ? family : "retry",
    _occt_host_build_family: () => 1,
    _occt_host_plugin_count: () => family === "family-a" ? 1 : 0,
    _occt_host_plugin_error: () => 0,
    _occt_host_plugin_name: () => 2,
    loadDynamicLibrary: () => new Promise((resolve) => { resolveLoad = resolve; }),
  };
  const loader = makeLoader(module);
  await loader.instantiate("main.mjs");
  await assert.rejects(
    loader.ensureSide({ name: "retry", url: "retry.wasm" }, "family-a"),
    (error) => error instanceof SharedKernelLoadError && error.code === "BuildFamilyMismatch",
  );
  assert.deepEqual(loader.loadedSides, []);

  family = "family-a";
  const loading = loader.ensureSide({ name: "retry", url: "retry.wasm" }, "family-a");
  await Promise.resolve();
  loader.dispose();
  resolveLoad(true);
  await assert.rejects(
    loading,
    (error) => error instanceof SharedKernelLoadError && error.code === "SideLoadFailed",
  );
  assert.deepEqual(loader.loadedSides, []);
});
