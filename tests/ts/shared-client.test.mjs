import assert from "node:assert/strict";
import test from "node:test";
import { SharedClient } from "../../dist/shared-client.js";
import { SharedKernelLoadError } from "../../dist/shared-loader.js";

function fakeMain({ family = "family-a", protocolVersion = "1.2.0", onLoad, onHandle } = {}) {
  const memory = new WebAssembly.Memory({ initial: 1 });
  const heap = new Uint8Array(memory.buffer);
  const responsePointer = 8192;
  let registrations = 0;
  let pluginName = "";
  return {
    memory,
    HEAPU8: heap,
    UTF8ToString: (pointer) => pointer === 1 ? family : pluginName,
    loadDynamicLibrary: async (url) => {
      onLoad?.(url);
      pluginName = url.replace(/\.side\.wasm$/, "");
      registrations++;
      return true;
    },
    _k_alloc: () => 1024,
    _k_free: () => undefined,
    _k_handle: (requestPointer, requestLength) => {
      const request = JSON.parse(new TextDecoder().decode(
        heap.subarray(requestPointer, requestPointer + requestLength),
      ));
      onHandle?.(request);
      const result = request.op === "capabilities"
        ? {
            protocolVersion,
            kernelVersion: "main-kernel",
            occtVersion: "main-occt",
            ops: ["capabilities", "beginScope"],
            historySupport: { capabilities: "unsupported", beginScope: "unsupported" },
            buildFlags: { wasmExceptions: true, source: "main" },
          }
        : {};
      const response = new TextEncoder().encode(JSON.stringify({ id: request.id, ok: true, result }));
      heap.set(response, responsePointer);
      return response.byteLength;
    },
    _k_response_ptr: () => responsePointer,
    _k_buffer_ptr: () => 0,
    _k_buffer_len: () => 0,
    _occt_host_build_family: () => 1,
    _occt_host_plugin_count: () => registrations,
    _occt_host_plugin_error: () => 0,
    _occt_host_plugin_name: () => 2,
  };
}

function options(module, overrides = {}) {
  const descriptor = (name, kind) => ({
    name,
    kind,
    url: name,
    sha256: "0".repeat(64),
    sizeBytes: 0,
    buildFamily: "family-a",
  });
  return {
    mainJsUrl: "main.mjs",
    mainWasmUrl: "main.wasm",
    resolve: (artifact) => artifact.name,
    importFactory: async () => async () => module,
    config: {
      buildFamily: "family-a",
      mainJs: descriptor("main.mjs", "shared-glue"),
      mainWasm: descriptor("main.wasm", "shared-main"),
      sides: [descriptor("modeling.side.wasm", "shared-side")],
      operationSides: { makeBox: "modeling.side.wasm" },
    },
    ...overrides,
  };
}

test("shared client derives capabilities from Main and validates its boundary", async () => {
  const loaded = [];
  const client = await SharedClient.create(options(fakeMain({ onLoad: (url) => loaded.push(url) })));
  const capabilities = await client.initialize();
  assert.equal(capabilities.kernelVersion, "main-kernel");
  assert.equal(capabilities.occtVersion, "main-occt");
  assert.deepEqual(capabilities.buildFlags, { wasmExceptions: true, source: "main" });
  assert.deepEqual(capabilities.ops, ["beginScope", "capabilities", "makeBox"]);
  assert.equal(capabilities.historySupport.makeBox, "unsupported");
  await client.request("batch", {
    ops: [{ op: "batch", args: { ops: [{ op: "makeBox", args: {} }] } }],
  });
  assert.deepEqual(loaded, ["modeling.side.wasm"]);
  client.close();

  await assert.rejects(
    SharedClient.create(options(fakeMain({ family: "other" }))),
    (error) => error instanceof SharedKernelLoadError && error.code === "BuildFamilyMismatch",
  );
  await assert.rejects(
    SharedClient.create(options(fakeMain({ protocolVersion: "0.9.0" }))),
    /Unsupported kernel protocol 0.9.0/,
  );
  await assert.rejects(
    SharedClient.create(options(fakeMain(), {
      config: {
        ...options(fakeMain()).config,
        operationSides: { makeBox: "missing.side.wasm" },
      },
    })),
    /maps to unconfigured side 'missing.side.wasm'/,
  );
});

test("shared client connects RequestOptions cancellation and progress to Main imports", async () => {
  let factoryOptions;
  const controller = new AbortController();
  const progress = [];
  const module = fakeMain({
    onHandle: (request) => {
      if (request.op !== "stats") return;
      factoryOptions.occtWorkerProgress(0.4);
      assert.equal(factoryOptions.occtWorkerCancelled(), true);
    },
  });
  const client = await SharedClient.create(options(module, {
    importFactory: async () => async (received) => {
      factoryOptions = received;
      return module;
    },
  }));
  try {
    await assert.rejects(
      client.request("stats", {}, {
        signal: controller.signal,
        onProgress: (event) => {
          progress.push(event);
          if (event.fraction === 0.4) controller.abort(new Error("shared request cancelled"));
        },
      }),
      /shared request cancelled/,
    );
    assert.deepEqual(progress, [
      { operation: "stats", fraction: 0 },
      { operation: "stats", fraction: 0.4 },
    ]);
    await assert.rejects(client.request("stats", {}, { timeoutMs: 0 }), { name: "TimeoutError" });
  } finally {
    client.close();
  }
});
