# Host support

[中文说明 / Chinese guide](hosts.zh-CN.md)

Runtime composition, Profile boundaries, shape ownership, and memory terminology are
defined in the [runtime architecture](architecture.md). This page defines host support
and integration requirements.

## Verified hosts

The release artifact `wasm/occt-worker.wasm` is validated in Node, Node `worker_threads`, and Chromium, Firefox, and WebKit Web Workers. Those hosts run the same SHA-256 artifact and call `_initialize` exactly once per instance. The required import and export surface is frozen in `protocol/wasm-surface.json`; `scripts/verify-wasm.mjs` rejects any drift.

The browser matrix uses the desktop Playwright engines. It does not certify iOS Safari, Android WebView, or mobile memory limits; a release must name separately any real-device result it intends to claim.

## Version and build matrix

| Environment | Declared or pinned version | Coverage |
| --- | --- | --- |
| Node.js | `>=20`; CI uses Node 22 | Direct client, worker client, package consumer, and wasm verification |
| Chromium, Firefox, WebKit | Desktop engines installed by Playwright 1.62.1 | Browser Worker and packed npm consumer smoke tests |
| Wasmtime | 47.0.3 in CI | Separate translated artifact and protocol smoke test |
| Source build host | `windows-latest` in CI; PowerShell build scripts | Release wasm rebuild and generated-file verification |

The package runtime is intended for Node.js 20+ and the desktop browser Worker
environments listed above. Other operating systems and browser versions are not
release-certified unless a release note states otherwise. Source builds on other
platforms may require adapting the PowerShell bootstrap and build commands.

## Wasmtime

### Artifact conversion

Wasmtime uses the separate `wasm/occt-worker.wasmtime.wasm` artifact. Emscripten 4.0.23 emits the Phase 3 form of WebAssembly exception handling for `-fwasm-exceptions`; current Wasmtime rejects that encoding with `legacy_exceptions feature required for try instruction`.

`npm run build:wasm:wasmtime` uses Binaryen to translate the built module to standardized `try_table`/`exnref` encoding while preserving C++ exception behavior. It does not overwrite `wasm/occt-worker.wasm`, because Node and the supported browsers do not all accept that exception-reference form.

### Verification and embedding

Run `npm run test:wasmtime -- -WasmtimePath /path/to/wasmtime` to validate the alternate artifact. The test preloads deterministic `env` stubs, lets Wasmtime supply WASI Preview 1, initializes the reactor, and covers scope creation, primitives, booleans, bounding boxes, tessellation, structured errors, and resource cleanup through `k_handle`.

CI and the release workflow use `scripts/bootstrap-wasmtime.ps1` to install the pinned Wasmtime version, rebuild the translated artifact, run the smoke test, and publish the artifact with its checksum. Release evidence is recorded in `docs/wasmtime-build.json`.

Embedders must provide the imports frozen in `protocol/wasm-surface.json`, call `_initialize` once, and then use the same message ABI as browser and Node hosts.

### Deterministic imports

The allowlisted WASI `fd_*`, environment, and clock imports do not expose protocol filesystem or wall-clock behavior. `DirectClient` supplies deterministic stubs: writes are discarded while reporting the consumed byte count, reads and environment enumeration are empty, and clock values advance from a fixed epoch.

The non-WASI `env` imports are Emscripten/OCCT runtime hooks implemented by the browser and Node adapters. `occt_worker_progress` and `occt_worker_cancelled` connect supported OCCT algorithms to host progress and cancellation state.

## Worker, cancellation, and shared memory

### Cooperative cancellation vs hard recovery

`WorkerClient.request()` accepts `{ signal, onProgress }`. A queued call is removed without disturbing the active worker.

- **Cooperative cancellation:** Tessellation and edge tessellation, VRML and mesh (STL/OBJ/PLY/glTF) exchange, and STEP/IGES shape and document import/export use `SharedArrayBuffer` for cooperative cancellation. When OCCT acknowledges the cancellation signal, the worker, its shape handles, and subsequent queued calls remain valid. Mesh exchange checks the cancellation flag while parsing, serializing, and copying output buffers.
- **Hard recovery:** For active synchronous operations that do not support cooperative cancellation, or environments without `SharedArrayBuffer`, cancellation terminates and rebuilds the worker. All previous handles and pending queued requests are invalidated.

### SharedArrayBuffer and cross-origin isolation

For bulk data transfer, binary imports accept `SharedArrayBuffer`. Unlike standard `ArrayBuffer` inputs, shared buffers cross the worker boundary without transfer or detachment.

`WorkerClient.requestShared(op, args, options)` allocates a `SharedArrayBuffer` for each output buffer descriptor and copies the data from WebAssembly linear memory. The shared buffer is neither cloned nor detached at the Worker boundary, but this path still performs one copy into host memory.

Browsers require cross-origin isolation headers for both bulk buffer sharing and cooperative cancellation:

```http
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

### Node.js `worker_threads` integration

`WorkerClient` expects a `WorkerLike` interface compatible with browser Web Workers. When hosting `occt-worker` in Node.js `worker_threads`, the host adapter must implement:

```ts
export interface WorkerLike {
  postMessage(message: unknown, transfer?: Transferable[]): void;
  addEventListener(type: "message", listener: (event: MessageEvent) => void): void;
  addEventListener(type: "error", listener: (event: ErrorEvent) => void): void;
  terminate(): void;
}
```

### Process isolation and mobile limits

Dedicated Worker isolation lets the main page handle JavaScript-visible failures such as `abort()` and `RangeError`. However, it does not isolate against operating-system memory terminations: on iOS WebKit, the page and Worker share the same WebContent process, and memory pressure causes Jetsam to terminate the entire process.

`ArtifactLoadAttemptTracker` provides heuristic tracking for full-profile loading. When passed as `loadAttempt`, `createWorkerProfileRuntime` records progression through `fetching`, `compiling`, `instantiating`, and `ready`. Applications can call `unfinished()` after a reload to detect that the previous load attempt did not complete.

### Artifact caching in browsers

In browser environments, the default isolated Profile loader caches official artifacts with a declared SHA-256 as raw WASM bytes via the Cache API. The cache key combines the artifact name and hash. Every cache hit is re-verified by checksum; invalid entries are purged and re-fetched. A custom `loadArtifact` callback and the shared Main/Side loader bypass this cache. Compiled `WebAssembly.Module` objects are intentionally not persisted.
