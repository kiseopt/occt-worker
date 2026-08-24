# occt-worker

[中文说明](README.zh-CN.md) · [文档导航 / Documentation](docs/README.md)

OCCT geometry kernel for WebAssembly - narrow message protocol, reproducible workflows, browser and Node first.

This repository is independent of and is not affiliated with or endorsed by Open Cascade SAS. It does not use Open Cascade SAS trademarks.

## Status

v1.0.0 freezes the implemented modeling and query surface: a standalone reactor built from OCCT 8.0.1 and Emscripten 4.0.23, a synchronous message ABI, generation-checked shape arenas and scopes, primitives, construction, extrusion/revolution/loft/sweep, booleans/splitting, General Fuse cell selection and material merging, explicit glue/imprint, draft, local prism and rotational rib/slot features, cylindrical-hole and defeaturing features, fillet/chamfer/hollow/offset/healing, transforms, topology and mass queries, topology-aware projections/extrema/intersections, diagnostics, tessellation, BREP/STEP/IGES/STL/VRML/OBJ/PLY/glTF exchange, STEP XCAF assembly documents, and exchange-format probing. The TypeScript layer also includes a serializable parameter/expression and feature-DAG model with atomic recompute. OCCT is compiled with `OCC_CONVERT_SIGNALS` undefined: this kernel never installs signal handlers, and the setjmp-based `OCC_CATCH_SIGNALS` expansion makes the pinned wasm-EH toolchain emit invalid wasm (`br_table` label-arity mismatches) in the ShapeHealing/STEP code. The published operation set is always the `capabilities` response.

The output contract is deliberately narrow. Mesh vertices are face-isolated; `faceGroups` contains `(faceIndex, indexStart, indexCount)` u32 triples whose start and count address scalar index entries. `edgeGroups` uses `(edgeIndex, vertexStart, vertexCount)`. Geometry comparisons should use tolerances. Matrix order, transform composition, inertia order, defaults, and layouts are frozen in [docs/protocol.md](docs/protocol.md). BREP is a cache format bound by the caller to the exact wasm SHA-256, not a cross-version exchange format. The wasm SHA-256 and import table are recorded by the build verification command, while byte identity is not promised for outputs outside the protocol's explicitly normalized layouts.

The primary release wasm is verified in Node, Node workers, and Chromium, Firefox, and WebKit Web Workers. Wasmtime uses the separately published `wasm/occt-worker.wasmtime.wasm`, which preserves the same message ABI and C++ exception behavior while translating the pinned Emscripten release's legacy wasm-EH encoding to standardized exception references. The measured host decision and frozen import surface are documented in [docs/hosts.md](docs/hosts.md).

The repository implements a general CAD/OCCT WebAssembly platform across topology, modeling, curves and surfaces, parametric CAD, mesh processing, queries, and every exchange format family listed in the public capability matrix. It is a deliberately curated, coarse-grained API rather than a binding for every non-visual OCCT class or toolkit. The authoritative implemented surface and its explicit limits are maintained in [docs/capabilities.md](docs/capabilities.md). Visualization, pthread/TBB execution, generic OCAF authoring, and OCCT toolkits without a public operation remain outside this v1 surface.

## Node quick start

Install the published package with Node.js 20 or newer:

```sh
npm install occt-worker
```

The direct client loads the release wasm explicitly:

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

Use `await using scope = await kernel.beginScope()` in runtimes supporting `AsyncDisposable`.

For browser Worker usage, see [examples/browser](examples/browser) and [host support](docs/hosts.md).

## Build

The first build downloads the pinned CMake and Ninja archives into `.tools/`, installs the pinned Emscripten SDK submodule, builds the OCCT submodule, and emits `wasm/occt-worker.wasm`:

```powershell
npm install
npm test
.\scripts\build-wasm.ps1
```

`npm run build:wasm:debug` emits the assertion-enabled artifact at `build/debug/occt-worker.debug.wasm`; it is a CI diagnostic artifact and is not included in the npm package. Both profiles reserve a 5 MiB wasm stack. Release uses `-O3` and `wasm-opt -O3`; compiling all OCCT archives with `-flto` is intentionally disabled because the pinned toolchain's measured output traps while tessellating a fillet result.

The build is intentionally static and includes OCCT code. The published npm tarball carries `npm-shrinkwrap.json`, so the `SOURCE.md` rebuild sequence can use `npm ci`. See [LICENSES.md](LICENSES.md), [NOTICE](NOTICE), and [SOURCE.md](SOURCE.md) for the mixed-license distribution boundary and the corresponding-source/relinking materials that must accompany a binary distribution.

## Project documents

- [Changelog](CHANGELOG.md)
- [Security policy](SECURITY.md)
- [Code of conduct](CODE_OF_CONDUCT.md)
- [Contributing](CONTRIBUTING.md)
