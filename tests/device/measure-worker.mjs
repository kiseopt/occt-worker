const MEBIBYTE = 1024 * 1024;

self.addEventListener("message", async (event) => {
  try {
    const { mode = "measure", profile, candidateMb } = event.data;
    if (profile !== "preview" && profile !== "full") throw new Error(`Unknown profile '${profile}'`);
    if (mode === "limit" && ![1536, 1280, 1024, 768].includes(candidateMb)) {
      throw new Error(`Unknown memory candidate '${candidateMb}'`);
    }
    if (mode === "limit") {
      self.postMessage({ type: "limit-progress", stage: "加载候选产物" });
    }

    const stepUrl = new URL("../../occt/data/step/screw.step", self.location.href);
    const stepResponse = await fetch(stepUrl);
    if (!stepResponse.ok) throw new Error(`STEP fixture request failed: HTTP ${stepResponse.status}`);
    const stepBytes = new Uint8Array(await stepResponse.arrayBuffer());

    const wasmUrl = mode === "limit"
      ? new URL(`./candidates/${candidateMb}/${profile}.wasm`, self.location.href)
      : new URL(`../../artifacts/${profile}.wasm`, self.location.href);
    const compileStarted = performance.now();
    const module = await WebAssembly.compileStreaming(fetch(wasmUrl, { cache: "no-store" }));
    const compileMs = performance.now() - compileStarted;

    let memory;
    let clockNanoseconds = 1767225600000000000n;
    const stubs = {
      clock_time_get: (_clockId, _precision, timePointer) => {
        if (memory !== undefined) {
          new DataView(memory.buffer).setBigUint64(timePointer, clockNanoseconds, true);
          clockNanoseconds += 1000000n;
        }
        return 0;
      },
      fd_write: (_fd, iovPointer, iovCount, writtenPointer) => {
        if (memory !== undefined) {
          const view = new DataView(memory.buffer);
          let written = 0;
          for (let index = 0; index < iovCount; index++) {
            written += view.getUint32(iovPointer + index * 8 + 4, true);
          }
          view.setUint32(writtenPointer, written, true);
        }
        return 0;
      },
      fd_read: (_fd, _iovPointer, _iovCount, readPointer) => {
        if (memory !== undefined) new DataView(memory.buffer).setUint32(readPointer, 0, true);
        return 0;
      },
      environ_sizes_get: (countPointer, sizePointer) => {
        if (memory !== undefined) {
          const view = new DataView(memory.buffer);
          view.setUint32(countPointer, 0, true);
          view.setUint32(sizePointer, 1, true);
        }
        return 0;
      },
      environ_get: (_environPointer, bufferPointer) => {
        if (memory !== undefined) new Uint8Array(memory.buffer)[bufferPointer] = 0;
        return 0;
      },
      occt_worker_cancelled: () => 0,
      occt_worker_progress: () => {},
    };
    const wasi = new Proxy(stubs, {
      get: (target, property) => (property in target ? target[property] : () => 0),
    });
    const instance = await WebAssembly.instantiate(module, { wasi_snapshot_preview1: wasi, env: wasi });
    const exports = instance.exports;
    memory = exports.memory;
    if (
      exports.emscripten_stack_init !== undefined
      && exports.emscripten_stack_get_base !== undefined
      && exports.emscripten_stack_get_end !== undefined
      && exports.__set_stack_limits !== undefined
    ) {
      exports.emscripten_stack_init();
      exports.__set_stack_limits(exports.emscripten_stack_get_base(), exports.emscripten_stack_get_end());
    }
    exports._initialize?.();
    if (mode === "limit") {
      self.postMessage({
        type: "limit-progress",
        stage: "执行典型负载",
        memoryMb: memory.buffer.byteLength / MEBIBYTE,
      });
    }

    let requestId = 1;
    const call = (operation, args) => {
      const request = new TextEncoder().encode(JSON.stringify({ id: requestId++, op: operation, args }));
      const pointer = exports.k_alloc(request.byteLength);
      if (pointer === 0 && request.byteLength !== 0) throw new Error("kernel request allocation failed");
      let responseLength;
      try {
        new Uint8Array(memory.buffer).set(request, pointer);
        responseLength = exports.k_handle(pointer, request.byteLength);
      } finally {
        exports.k_free(pointer);
      }
      if (responseLength === 0) throw new Error("kernel did not return a protocol response");
      const responsePointer = exports.k_response_ptr();
      const response = JSON.parse(new TextDecoder().decode(
        new Uint8Array(memory.buffer).slice(responsePointer, responsePointer + responseLength),
      ));
      if (!response.ok) {
        throw new Error(`${response.error?.code ?? "KernelError"}: ${response.error?.message ?? "kernel request failed"}`);
      }
      return response.result;
    };

    const capabilities = call("capabilities", {});
    for (const operation of ["beginScope", "importSTEP", "tessellate"]) {
      if (!capabilities.ops.includes(operation)) throw new Error(`${profile} does not register '${operation}'`);
    }
    if (profile === "full" && !capabilities.ops.includes("booleanFuse")) {
      throw new Error("full does not register 'booleanFuse'");
    }

    const instantiateMb = memory.buffer.byteLength / MEBIBYTE;
    let maxMemoryBytes = memory.buffer.byteLength;
    const trackMemory = () => { maxMemoryBytes = Math.max(maxMemoryBytes, memory.buffer.byteLength); };
    const { scopeId } = call("beginScope", {});
    trackMemory();

    const input = call("createBuffer", { byteLength: stepBytes.byteLength });
    const inputPointer = exports.k_buffer_ptr(input.bufferId);
    if (exports.k_buffer_len(input.bufferId) !== stepBytes.byteLength || inputPointer === 0) {
      throw new Error("STEP input buffer is unavailable");
    }
    new Uint8Array(memory.buffer).set(stepBytes, inputPointer);
    let imported;
    try {
      imported = call("importSTEP", { scopeId, data: { bufferId: input.bufferId }, unit: "mm" });
    } finally {
      call("freeBuffer", { bufferId: input.bufferId });
    }
    trackMemory();

    const tessellateStarted = performance.now();
    const tessellation = call("tessellate", {
      shape: imported.shape,
      linearDeflection: 0.15,
      angularDeflection: 0.4,
    });
    const tessellateMs = performance.now() - tessellateStarted;
    trackMemory();
    const releasedBuffers = new Set();
    const releaseBuffers = (value) => {
      if (Array.isArray(value)) {
        for (const item of value) releaseBuffers(item);
      } else if (value !== null && typeof value === "object") {
        if (
          Number.isInteger(value.bufferId)
          && Number.isInteger(value.byteLength)
          && typeof value.layout === "string"
        ) {
          if (!releasedBuffers.has(value.bufferId)) {
            call("freeBuffer", { bufferId: value.bufferId });
            releasedBuffers.add(value.bufferId);
          }
        } else {
          for (const item of Object.values(value)) releaseBuffers(item);
        }
      }
    };
    releaseBuffers(tessellation);

    let booleanMs;
    if (profile === "full") {
      const base = call("makeBox", { scopeId, size: [40, 40, 40], origin: [0, 0, 0] }).shape;
      const tool = call("makeBox", { scopeId, size: [40, 40, 40], origin: [20, 20, 20] }).shape;
      const booleanStarted = performance.now();
      call("booleanFuse", { scopeId, base, tools: [tool] });
      booleanMs = performance.now() - booleanStarted;
      trackMemory();
    }

    call("endScope", { scopeId });
    if (mode === "limit") {
      const growPages = 256;
      while (true) {
        self.postMessage({
          type: "limit-progress",
          stage: "增长并写满 wasm 线性内存",
          memoryMb: memory.buffer.byteLength / MEBIBYTE,
        });
        await new Promise((resolve) => setTimeout(resolve, 50));
        try {
          const previousPages = memory.grow(growPages);
          new Uint8Array(memory.buffer, previousPages * 65536, growPages * 65536).fill(0xa5);
        } catch (error) {
          if (!(error instanceof RangeError)) throw error;
          self.postMessage({
            type: "limit-result",
            result: {
              outcome: "range-error",
              memoryMb: memory.buffer.byteLength / MEBIBYTE,
              detail: `${error.name}: ${error.message}`,
            },
          });
          return;
        }
      }
    }
    const jsHeapBytes = Number.isFinite(performance.memory?.usedJSHeapSize)
      ? performance.memory.usedJSHeapSize
      : undefined;
    self.postMessage({
      type: "result",
      result: {
        compileMs,
        instantiateMb,
        linmemMaxMb: maxMemoryBytes / MEBIBYTE,
        tessellateMs,
        ...(booleanMs === undefined ? {} : { booleanMs }),
        jsHeapBytes,
      },
    });
  } catch (error) {
    self.postMessage({
      type: "error",
      error: error instanceof Error ? `${error.name}: ${error.message}` : String(error),
    });
  }
});
