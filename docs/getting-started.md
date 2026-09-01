# Getting started

[中文说明 / Chinese guide](getting-started.zh-CN.md)

This page covers first use and common workflows: choosing a runtime entry point,
running the published kernel, building from source, understanding the boundary of
generated code, and handling common errors. Complete API parameters and protocol
fields belong in the [TypeScript API](api.md). Runtime composition, Profile boundaries,
and shape and memory ownership are defined in the [architecture](architecture.md).

## Choose a runtime

| Your scenario | Choose | What to do |
| --- | --- | --- |
| Node.js scripts, services, or batch jobs | `DirectClient` | Read the full WASM, create a client and scope, and call `ShapeScope`. |
| Do not block the Node.js or browser main thread | `WorkerClient` | Provide a Worker factory and WASM; operations run through the message queue. |
| Load a smaller kernel module by capability | `SharedClient` or `GeometryEngine` | Resolve Main, Side, or isolated-profile artifacts from the release manifest. |
| Parametric parts and assemblies | `ParametricModel` | Manage parameters, the feature DAG, recompute, and subshape references on a client. |
| A custom host or Wasmtime | Synchronous WASM ABI | Provide the imports and call the protocol ABI described in [host support](hosts.md). |

To verify that the project works without rebuilding OCCT, run the Node.js example:

```
node examples/node-batch.mjs
```

## Install and run

Install the published npm package:

```
npm install occt-worker
```

Minimal Node.js program:

```js
import { readFile } from "node:fs/promises";
import { DirectClient } from "occt-worker";

const wasm = await readFile(new URL(import.meta.resolve("occt-worker/wasm")));
const kernel = await DirectClient.create(wasm);
const scope = await kernel.beginScope();

const base = await scope.makeBox([100, 60, 4]);
const tool = await scope.makeCylinder(8, 4, { origin: [50, 30, 0] });
const cut = await scope.booleanCut(base, [tool]);

console.log(await kernel.bbox(cut.shape));
await scope.end();
```

Use the `engines` field in `package.json` as the Node.js version requirement. Shapes
belong to the scope and client that created them; ending a scope releases its shapes,
and rebuilding a Worker invalidates its old handles.

## Use a browser Worker

### In an application using the npm package

When consuming `occt-worker` in a web application:

```js
import { WorkerClient } from "occt-worker";

// Fetch the release WASM binary
const wasm = await (await fetch("/path/to/occt-worker.wasm")).arrayBuffer();

// Resolve the package and worker subpath through the application's bundler or import map.
const workerUrl = import.meta.resolve("occt-worker/worker");
const factory = () => new Worker(workerUrl, { type: "module" });

const kernel = await WorkerClient.create(factory, wasm);
const scope = await kernel.beginScope();
const box = await scope.makeBox([10, 20, 30]);

console.log(await kernel.massProps(box));
await scope.end();
kernel.close();
```

> **Repository checkouts and local examples:** Inside this repository (such as [`examples/browser`](../examples/browser)), scripts reference local build artifacts using relative paths such as `../../dist/worker-entry.js` and `../../wasm/occt-worker.wasm`.

> **Without an import map:** use a URL emitted by the application's bundler or a server-visible URL to the installed file, such as `/node_modules/occt-worker/dist/worker-entry.js`. The package export is a Node/bundler resolution rule, not a native browser URL resolution rule.

### Browser environment requirements

Browsers must load ES modules, Workers, and WASM over HTTP(S); do not open the HTML directly with `file://`.

Features that rely on `SharedArrayBuffer` (shared input/output buffers and cooperative cancellation) require cross-origin isolation response headers:

```http
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

### Node.js `worker_threads`

Node.js `worker_threads` uses the same `WorkerClient`, but requires a host adapter that bridges Node's `parentPort` messages to the `WorkerLike` interface expected by `WorkerClient`. The browser-only `dist/worker-entry.js` is not a Node Worker entry point. See [Host support](hosts.md) for details on host adapters.

## Discover available operations

For standalone and isolated runtimes, the runtime `capabilities` response is the final
authority for the operations the loaded kernel actually exposes:

```js
const capabilities = await kernel.initialize();
console.log(capabilities.ops);
```

`SharedClient.initialize()` instead reports the configured operation surface, including
operations whose Side module has not loaded yet; the matching Side loads on first dispatch.

Constants such as `OPERATIONS` and `HISTORY_SUPPORT` are generated from the protocol
and are useful for static typing and routing. Runtime capability decisions must use
the returned `capabilities` value. The complete operation contract is defined by:

- [`protocol/operations.json`](../protocol/operations.json)
- [`protocol/operation-contracts.json`](../protocol/operation-contracts.json)
- [`protocol/modules.json`](../protocol/modules.json)

## Build from source

Build from source when changing the kernel, protocol, or TypeScript client. Clone the
repository with its pinned submodules:

```powershell
git clone --recurse-submodules https://github.com/kiseopt/occt-worker.git
cd occt-worker
npm ci
```

For protocol or TypeScript-only changes:

```powershell
npm run build
```

For OCCT C++, kernel handlers, or build-configuration changes:

```powershell
npm run build:wasm
```

The script prepares the pinned CMake, Ninja, and Emscripten SDK versions, builds the
OCCT static libraries, and then builds the project WASM kernel. For a debug artifact:

```powershell
npm run build:wasm:debug
```

After a build, run checks relevant to the change:

```powershell
npm test
npm run test:browser
```

The build requires a clean `occt` submodule worktree. Do not edit generated protocol
files directly; update the authoritative definitions under `protocol/` and run
`npm run generate`. New C++ code must be connected to a `kernel/` handler and the
CMake build graph; placing a `.cpp` file in the repository is not enough.

## Runtime boundary of generated code

| Generated result | Runs directly | Correct path |
| --- | --- | --- |
| JavaScript API calls | Yes | Install dependencies and load the published WASM with `DirectClient` or `WorkerClient`. |
| TypeScript API calls | Yes | Compile with the TypeScript toolchain, then use the JavaScript path. |
| Protocol JSON or `batch` sequences | Yes | Pass them to the existing WASM kernel through `request()` or `batch()`. |
| New OCCT C++ source | No | Compile it into the full kernel or a Side WASM, register protocol operations, and call them. |

This project executes compiled WebAssembly; it is not a C++ source interpreter. A
generator that emits new C++ requires a new kernel build each time. To avoid repeated
compilation, generate API calls, parameters, or protocol JSON that use algorithms
already compiled into the kernel.

## Resource lifetime

- Pair every `beginScope()` with `scope.end()`.
- Release an individual shape with `kernel.release(shape)`; use `releaseAll()` for a batch.
- Binary import and export use kernel buffers. After obtaining a `DirectBuffer`, reacquire
  `view()` after every WASM call.
- A Worker timeout or an active non-cooperative cancellation rebuilds the Worker; old
  shape handles cannot be reused.
- Use `stats()` to inspect `liveShapeHandles`, `liveBufferBytes`, and WASM memory usage.

## Troubleshooting

| Symptom | Cause | Action |
| --- | --- | --- |
| `Cannot find module` or no `dist` directory | Running from source before generating TypeScript output | Run `npm run build`, or import from the published package. |
| `Unknown operation` | The client, protocol, and WASM came from different generated outputs | Change `protocol/`, run `npm run generate`, and rebuild the WASM. |
| `BuildFamilyMismatch` or Side loading fails | Main and Side artifacts came from different build families or versions | Resolve artifacts from the same `protocol/artifacts.json` release. |
| `InvalidHandle` after a Worker rebuild | The handle belongs to an expired Worker epoch | Create a new scope and shapes; do not cache handles across Worker rebuilds. |
| WASM or Worker loading fails in a browser | `file://` or an HTTP server with the wrong MIME type or headers | Serve over HTTP(S); enable cross-origin isolation for shared memory. |
| New C++ code fails to link | The toolkit is disabled or the source is missing from the CMake target | Check `scripts/build-config.ps1`, `kernel/CMakeLists.txt`, and the target toolkit. |

## Next reading

- [TypeScript API](api.md): find public entry points, methods, types, errors, and exchange details.
- [Runtime and WASM architecture](architecture.md): understand layering, Profiles, artifacts, ownership, and lifecycle.
- [Protocol specification](protocol.md): read frames, handles, buffers, and determinism.
- [Capability matrix](capabilities.md): see implemented capabilities and explicit limits.
- [Host support](hosts.md): see browser, Worker, Node.js, and Wasmtime boundaries.
- [Browser example](../examples/browser): run a complete Worker, shared-buffer, and mesh flow.
