# Host support

[中文说明 / Chinese guide](hosts.zh-CN.md)

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

Wasmtime uses the separate `wasm/occt-worker.wasmtime.wasm` artifact. Emscripten 4.0.23 emits the Phase 3 form of WebAssembly exception handling for `-fwasm-exceptions`; current Wasmtime rejects that encoding with `legacy_exceptions feature required for try instruction`. `npm run build:wasm:wasmtime` preserves C++ exception behavior while translating the already-built module to the standardized `try_table`/`exnref` encoding with Binaryen. It never overwrites `wasm/occt-worker.wasm`, because Node and the supported browsers do not yet all accept the standardized exception-reference form.

Run `npm run test:wasmtime -- -WasmtimePath /path/to/wasmtime` to validate the alternate artifact. The test preloads the deterministic `env` stubs, lets Wasmtime supply WASI Preview 1, initializes the reactor, and exercises scope creation, primitive construction, boolean modeling, bounding-box queries, tessellation, structured errors, and resource cleanup through `k_handle`. CI and the release workflow install the pinned Wasmtime 47.0.3 binary with `scripts/bootstrap-wasmtime.ps1`, rebuild the translated artifact, run this smoke, and publish the artifact with its checksum. Release evidence is recorded in `docs/wasmtime-build.json`. Embedders must provide the imports frozen in `protocol/wasm-surface.json`, call `_initialize` once, and then use the same message ABI as browser and Node hosts.

The allowlisted WASI `fd_*`, environment, and clock imports do not expose protocol filesystem or wall-clock behavior. DirectClient supplies deterministic stubs: writes are discarded while reporting the consumed byte count, reads and environment enumeration are empty, and clock values advance from a fixed epoch. The non-WASI `env` imports are Emscripten/OCCT runtime hooks and are likewise implemented by the browser/Node adapter. `occt_worker_progress` and `occt_worker_cancelled` connect supported OCCT algorithms to host progress and cancellation state.

## Worker, cancellation, and shared memory

`WorkerClient.request()` accepts `{ signal, onProgress }`. A queued call is removed without disturbing the active worker. Tessellation, VRML, mesh (STL/OBJ/PLY/glTF), and shape-only STEP/IGES import/export use `SharedArrayBuffer` for cooperative cancellation and preserve the worker, handles, and following queue when OCCT acknowledges the break. Mesh exchange cancellation is checked at parser, serialization, and buffer-materialization phase boundaries. This requires a host where `SharedArrayBuffer` is available; browsers therefore need cross-origin isolation. Other active synchronous calls, and supported calls without shared memory support, retain hard cancellation: the worker is terminated and rebuilt and all old handles and queued calls become invalid.

Dedicated Worker isolation lets the main page report JavaScript-visible soft failures such as `abort()` and `RangeError`. It does not protect an iOS page from an out-of-memory termination: WebKit runs the Worker and page in the same WebContent process, and Jetsam terminates that entire process. After a reload, an unfinished-attempt marker can show only that the previous attempt did not complete; it cannot prove why.

`ArtifactLoadAttemptTracker` provides that heuristic for full-profile loading. When passed as `loadAttempt`, `createWorkerProfileRuntime` records `fetching`, `compiling`, `instantiating`, and `ready`; successful loads and catchable failures clear the active marker and store their result separately. Applications can use `unfinished()` after a reload to report that the previous full-feature load did not complete, without claiming a cause. The device measurement page uses the same tracker for memory-limit candidates.

In browsers, official artifacts with a declared SHA-256 are cached as wasm bytes through the Cache API. The cache key includes the artifact name and hash. Every cache hit is hashed again; invalid bytes are deleted and downloaded again. Compiled `WebAssembly.Module` objects are not persisted.

For bulk data, binary imports also accept `SharedArrayBuffer`; unlike ordinary `ArrayBuffer` inputs, these are shared with the worker and are not detached. `WorkerClient.requestShared(op, args, options)` requests `SharedArrayBuffer` materialization for all output buffer descriptors in a low-level protocol result. This avoids cloning or detaching data at the worker boundary, but the host still copies bytes once between wasm linear memory and the shared buffer. Browsers require cross-origin isolation for both bulk sharing and cooperative cancellation.
