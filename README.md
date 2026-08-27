# occt-worker

Run OCCT CAD modeling and geometry operations in Node.js and browser Workers without a
native CAD installation.

This WebAssembly geometry kernel uses a versioned message protocol and TypeScript clients
for Node.js and browser Workers.

[![CI](https://github.com/kiseopt/occt-worker/actions/workflows/ci.yml/badge.svg)](https://github.com/kiseopt/occt-worker/actions/workflows/ci.yml)
[![npm version](https://img.shields.io/npm/v/occt-worker.svg)](https://www.npmjs.com/package/occt-worker)
[![Node.js](https://img.shields.io/node/v/occt-worker.svg)](https://www.npmjs.com/package/occt-worker)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE-APACHE-2.0.txt)

[中文说明](README.zh-CN.md) · [Documentation](docs/README.md) · [API reference](docs/api.md) · [Capability matrix](docs/capabilities.md)

**Current release:** `v1.2.0` · **Runtime:** Node.js `>=20` · **License:** Apache-2.0 for
project code; see the distribution notices for OCCT and other third-party components.

This is an independent project. It is not affiliated with, endorsed by, or sponsored
by Open Cascade SAS, and it does not use Open Cascade SAS trademarks.

## Highlights

- **Modeling:** primitives, wires, faces, solids, sweeps, lofts, revolutions, booleans,
  splitting, local features, fillets, chamfers, shells, offsets, healing, and transforms.
- **Geometry and topology:** mass properties, bounding boxes, topology traversal,
  classification, distance, intersections, projections, extrema, curves, and surfaces.
- **CAD exchange:** BREP, STEP, IGES, STL, VRML, OBJ, PLY, glTF/GLB, and XCAF document
  import/export, with format probing and documented metadata behavior.
- **Mesh processing:** deterministic tessellation, edge tessellation, triangulation
  inspection and repair, indexed mesh import/export, and progress-aware cancellation.
- **Application layer:** serializable parameters, expressions, feature DAGs, sketches,
  subshape references, and assembly definitions in the TypeScript package.

The public operation set is the `capabilities` response. This is a deliberately curated,
coarse-grained API rather than a binding for every non-visual OCCT class or toolkit.

## Installation

Node.js 20 or newer is required:

```sh
npm install occt-worker
```

> **Package size vs. download size.** The npm package ships the full kernel WASM, the JS
> runtime, types, and protocol manifests; installing it does not benefit from runtime
> lazy-loading. Browser first-use downloads are a different metric: Side/Profile
> artifacts (stage-4 shared/isolated architecture) resolve lazily from Release/CDN via
> `resolveArtifact({ name })` with an optional `baseUrl` or fully custom resolver for
> private deployments.

## Quick start

`DirectClient` loads the release WebAssembly artifact explicitly and exposes scoped shape
handles:

```js
import { readFile } from "node:fs/promises";
import { DirectClient } from "occt-worker";

const wasm = await readFile(new URL(import.meta.resolve("occt-worker/wasm")));
const kernel = await DirectClient.create(wasm);
const scope = await kernel.beginScope();

const plate = await scope.makeBox([100, 60, 4]);
const hole = await scope.makeCylinder(8, 4, { origin: [50, 30, 0] });
const result = await scope.booleanCut(plate, [hole]);

console.log(await kernel.bbox(result.shape));
await scope.end();
```

This creates a 100 x 60 x 4 plate, cuts a centered cylindrical hole, and prints the
resulting bounding-box data.

Runtimes that implement `AsyncDisposable` can use
`await using scope = await kernel.beginScope()`. For browser Workers, start with
[the browser example](examples/browser) and read [host support](docs/hosts.md).

## API at a glance

| Use case | Public entry point | Reference |
| --- | --- | --- |
| In-process Node.js kernel | `DirectClient` and `ShapeScope` | [TypeScript API](docs/api.md) |
| Worker-backed Node.js or browser kernel | `WorkerClient` | [TypeScript API](docs/api.md) · [browser example](examples/browser) |
| Modeling, queries, mesh, and exchange operations | `ShapeScope` methods | [Capability matrix](docs/capabilities.md) |
| Low-level messages, buffers, errors, and defaults | `request()` and protocol frames | [Protocol specification](docs/protocol.md) |
| Wasmtime or custom host integration | Synchronous WebAssembly ABI | [Host support](docs/hosts.md) |
| Parametric features and sketches | `ParametricModel` and feature definitions | [TypeScript API](docs/api.md) |
| Shared Main/Side high-level compatibility | `SharedClient` + `EngineCompatClient` | [TypeScript API](docs/api.md) |

For an isolated profile, resolve its manifest descriptor and provide a Worker factory to
`createWorkerProfileRuntime`. The helper verifies the artifact before starting the worker;
register the resulting factory with `GeometryEngine` for lazy profile startup. Profile and
shared Side binaries are release artifacts, not files in the npm tarball. The tag-based release
workflow builds and uploads them to the matching GitHub Release and updates `protocol/artifacts.json`
with versioned URLs and SHA-256 values. Use `SharedClient` with `EngineCompatClient` when the legacy
high-level API should run on Main/Side; private mirrors can use `baseUrl`, `defaultBase`, or `resolve`
without maintaining a project CDN.

## Support matrix

| Environment or feature | Release position |
| --- | --- |
| Node.js | `>=20`; DirectClient, WorkerClient, npm consumer tests |
| Desktop browser Workers | Chromium, Firefox, and WebKit are covered by the release test matrix |
| Node `worker_threads` | Supported with the same primary `wasm/occt-worker.wasm` artifact |
| Wasmtime | Use the separately published `wasm/occt-worker.wasmtime.wasm`; CI uses Wasmtime 47.0.3 |
| Shared memory | `SharedArrayBuffer` features require browser cross-origin isolation |
| Mobile browsers and WebViews | Not part of the release certification matrix |
| Visualization and parallel OCCT toolkits | Visualization, pthread/TBB execution, and generic OCAF authoring are outside v1 |
| BREP | Cache format only; bind every entry to the exact wasm SHA-256 in `docs/g0-build.json` |

For the exact operation set, defaults, buffer layouts, cancellation rules, and host details,
see [the protocol](docs/protocol.md), [capabilities](docs/capabilities.md), and
[host support](docs/hosts.md).

## Known issues and limits

- Active synchronous operations cancelled through a `WorkerClient` use hard recovery:
  the worker is rebuilt and all handles owned by the previous worker expire.
- `SharedArrayBuffer` transport still performs one copy between WebAssembly linear memory
  and host memory; it is not zero-copy.
- BREP is a cache format, not a portable exchange format. Cache entries must be rejected
  when the running wasm SHA-256 differs from the recorded value.
- Mobile browsers and WebViews are not covered by the release test matrix, including their
  memory limits.
- Visualization, pthread/TBB execution, and generic OCAF authoring are outside the v1 API.
- STEP/IGES/XCAF metadata support follows the documented format-specific subset; data that
  a format cannot preserve is rejected or represented through the documented extension path.

Report reproducible defects with the input format, host runtime, version, and a minimal
sanitized reproducer. See [Security](SECURITY.md) for private vulnerability reporting and
[Issues](https://github.com/kiseopt/occt-worker/issues) for ordinary bugs and requests.

## Build from source

Clone the repository with its pinned submodules, then build the WebAssembly artifact:

```powershell
git clone --recurse-submodules https://github.com/kiseopt/occt-worker.git
cd occt-worker
npm ci
.\scripts\build-wasm.ps1
npm test
```

The build script bootstraps the pinned CMake, Ninja, and Emscripten SDK and emits
`wasm/occt-worker.wasm`. Useful checks include:

```powershell
npm run build:wasm:debug
npm run test:browser:all
npm run test:wasmtime -- -WasmtimePath C:\path\to\wasmtime.exe
```

Release builds, generated protocol files, and wasm hashes are verified by the repository's
scripts and CI. Do not edit generated protocol output directly; update the authoritative
files under `protocol/` first.

## Releases and corresponding source

Published npm packages include the TypeScript distribution, protocol metadata, documentation,
and release WebAssembly artifacts. Each binary release must be accompanied by the
corresponding-source and relinking materials described in [SOURCE.md](SOURCE.md), together
with the notices in [LICENSES.md](LICENSES.md), [NOTICE](NOTICE), and
`THIRD-PARTY-NOTICES.txt`.

## Documentation

- [Documentation index](docs/README.md)
- [TypeScript API](docs/api.md)
- [Protocol specification](docs/protocol.md)
- [Capability matrix](docs/capabilities.md)
- [Host support and Wasmtime](docs/hosts.md)
- [Changelog](CHANGELOG.md)

## Community

- [Report a bug or request a feature](https://github.com/kiseopt/occt-worker/issues)
- [Contributing guide](CONTRIBUTING.md)
- [Security policy](SECURITY.md)
- [Code of conduct](CODE_OF_CONDUCT.md)
- [Releases](https://github.com/kiseopt/occt-worker/releases)

## License

Project code and documentation are licensed under [Apache-2.0](LICENSE-APACHE-2.0.txt).
The distribution also contains OCCT under LGPL-2.1 with the Open CASCADE exception,
Emscripten runtime/system code, nlohmann/json, Draco, and meshoptimizer. See
[LICENSES.md](LICENSES.md) and `THIRD-PARTY-NOTICES.txt` for the applicable notices and
redistribution boundary.
