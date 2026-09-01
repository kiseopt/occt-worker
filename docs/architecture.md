# Runtime and WASM architecture

[中文说明 / Chinese translation](architecture.zh-CN.md)

This page defines the public architecture of the shipped runtime: how protocol
operations become modules, build units, profiles, artifacts, and runtime instances. It
also defines shape ownership, loading, failure, and memory boundaries. Exact operation
signatures belong in the [protocol specification](protocol.md); the generated
[profile capability table](profile-capabilities.generated.md) is the authority for the
current profile-to-operation mapping.

## Architecture model

The word "layer" is not one binary hierarchy. The implementation separates these
concepts so API organization, linker boundaries, release files, and application choices
can evolve independently:

```text
operation catalog
       |
       v
semantic modules
       |
       v
implementation units and dependency closure
       |
       v
profiles                         shared Side mapping
       |                                  |
       v                                  v
isolated Profile artifacts       shared Main + Side artifacts
       \                                  /
        +---------- runtime instances ----+
                          |
                          v
               application runtime choice
```

| Concept | Meaning | Authoritative source |
| --- | --- | --- |
| Operation | One public protocol command with stable request, result, error, and history semantics. | `protocol/operations.json` and `protocol/operation-contracts.json` |
| Semantic module | A capability category assigning each operation to one semantic owner. It describes what an operation does, not necessarily a file boundary. | `protocol/modules.json` under `semanticModules` |
| Implementation unit | Sources, OCCT toolkits, and dependencies that form one linker-level build unit. Units form an acyclic dependency graph. | `protocol/modules.json` under `implementationUnits` |
| Profile | A named, tested combination of implementation units for an isolated runtime. | `protocol/modules.json` under `profiles` |
| Artifact | A concrete release file such as a standalone Profile WASM, shared Main, Side WASM, or Main JavaScript glue. | `protocol/artifacts.json` |
| Backend/runtime form | The way artifacts are instantiated and shapes are owned: standalone, shared, or isolated. | Public clients and the generated runtime manifest |
| Product preset | An application-level choice such as switching between preview and full functionality. It may reuse a Profile, but it is not a new kernel abstraction. | Application configuration |

`protocol/modules.json` is the source of truth for topology. The profile generator derives
CMake source/toolkit closures, profile client types, runtime routing data, the shared Side
mapping, CI size budgets, and the profile capability table. Generated files must not be
edited directly.

## Profiles

Profiles are capability sets, not quality levels and not a strict smallest-to-largest
chain. The current official profiles are:

| Profile ID | Purpose | Important boundary |
| --- | --- | --- |
| `preview` | Import and export STEP or BREP shapes, probe every documented format, inspect topology and bounds, and produce face/edge tessellation for viewing. | No primitives, general geometry construction, booleans, feature modeling, mesh repair, or XCAF/CAD-document exchange. |
| `core-modeling` | Geometry and topology construction, feature modeling, booleans, analysis, and repair. | No tessellation, mesh editing, or general exchange formats. |
| `mesh` | `core-modeling` plus tessellation and triangulation inspection, replacement, validation, and repair. | No general exchange formats. |
| `modeling-viewer` | Product-facing name for the same capability set and artifact as `mesh`. | It is an alias, not a separately built WASM. |
| `exchange` | Shape/document and mesh-format exchange without the modeling stack. | It is an orthogonal exchange preset, not a larger or smaller modeling tier. |
| `full-profile` | Every public operation in one isolated Profile runtime. | The ID is `full-profile`; its artifact file is `full.wasm`. |

The generated capability table contains the exact semantic modules, artifact aliases, and
operation counts. Runtime `capabilities()` remains the final authority for the bytes that
were actually loaded, especially for a user-supplied artifact.

Avoid the ambiguous labels "minimal core" and "full core":

- `preview` is the smallest official viewing Profile, not a general CAD core.
- `core-modeling` is the smallest official modeling Profile, not necessarily the smallest
  possible custom binary.
- `full-profile` is the complete isolated Profile. It is distinct from the default
  standalone full artifact and from a shared runtime after all Sides are loaded.

Application presets should use the official Profile IDs directly. A product preset can
change when a runtime is started or stopped, but it does not change the Profile's
operation set.

## Runtime forms

The public operation contracts are shared across runtime forms, but loading, object
identity, cleanup, and failure impact differ.

| Runtime form | Public entry points | Artifacts and loading | Shape ownership | Reclamation and failure impact |
| --- | --- | --- | --- | --- |
| Standalone full | `DirectClient`, `WorkerClient` | Loads `wasm/occt-worker.wasm` as one complete static module. Wasmtime uses its separately translated standalone artifact. | One kernel instance, one arena, and local `ShapeHandle` values. | Close or rebuild the instance. All handles from that instance expire. |
| Shared Main/Side | `SharedClient`; optionally `EngineCompatClient` for the high-level `BaseClient` surface | Instantiates Main and its JavaScript glue, then loads the Side mapped to an operation before the first request that needs it. Concurrent first loads share one promise. | Main and every loaded Side share one memory, table, allocator, kernel arena, and handle namespace. | A Side remains resident for the Main epoch. Close the shared client; if it is hosted in a dedicated Worker, terminate that Worker. A failed/rebuilt Main invalidates the whole shared epoch. |
| Isolated Profiles | `createProfileClient`, or `createWorkerProfileRuntime` registered with `GeometryEngine` | Each Profile is a static WASM in its own Worker/runtime. `createProfileClient()` starts it immediately; the factory returned by `createWorkerProfileRuntime()` starts it when invoked. | Handles are local to one Profile instance. `GeometryEngine` exposes logical `EngineShapeRef` values and can maintain placements in multiple Profiles. | An invalidated Profile epoch invalidates its placements. Other Profile runtimes can remain usable, subject to the browser process boundary. |

Select the concrete entry point above. `GeometryEngine` coordinates isolated Profile runtimes;
`SharedClient` is the direct shared Main/Side client. A shared client can be adapted to the
legacy high-level API with `EngineCompatClient`, but that does not turn shared Main/Side
into multiple isolated Profile Workers.

### What "full" can mean

Use precise names when referring to complete capability:

| Name | Meaning |
| --- | --- |
| Standalone full | The default static `wasm/occt-worker.wasm` used by `DirectClient`, `WorkerClient`, and the source for the Wasmtime translation. |
| `full-profile` | The isolated Profile ID whose release artifact is `full.wasm`. |
| Complete shared runtime | One shared Main epoch after every required Side has been loaded. There is no separate artifact named "shared full." |

These forms can expose the same protocol operation set without having the same binary,
startup path, memory layout, or failure scope.

## Choosing a runtime

| Requirement | Runtime choice |
| --- | --- |
| Simplest deployment, npm-bundled WASM, in-process Node.js, or Wasmtime | Standalone full |
| One modeling session that should keep native shape identity while capabilities load by operation | Shared Main/Side |
| Start only a scenario-specific capability set, run Profiles separately, or reclaim one Profile runtime | Isolated Profiles |
| An application with mutually exclusive preview/full modes that must never keep both runtimes alive | `SingleRuntimeSession` with isolated Profile factories |

An application-level preview-to-full switch should close the old runtime before creating
the replacement. `SingleRuntimeSession.switchProfile()` rejects pending work, waits for
the underlying calls to settle, closes the old runtime, and only then creates the new one.
If replacement creation fails, the session is closed; it does not silently revive the old
runtime. The application must reload its model or start a new session. Old handles cannot
cross that boundary.

`GeometryEngine` serves a different workflow: it may keep multiple isolated Profiles
running and route operations among them. Do not use it as a substitute for an exclusive
runtime switch when the product requirement is to release the previous Profile first.

## Shape ownership and transfer

### Standalone and shared handles

A `ShapeHandle` belongs to the client, scope, and epoch that created it. Shared Main and
Sides use one arena, so a shape produced by one loaded Side can be consumed directly by
another loaded Side. The numeric handle is never portable to another standalone instance,
Profile Worker, or rebuilt epoch.

Ending a scope releases its remaining shapes. Closing or rebuilding a client invalidates
all handles from its previous epoch.

### Isolated logical shapes

`GeometryEngine` represents a logical shape with an opaque `EngineShapeRef`. A logical
shape can have one local placement in each Profile that needs it. When an operation must
run in another Profile, the engine:

1. exports the source placement through the internal BREP transfer path;
2. wraps the payload with transfer version, build-family identity, length, and checksum;
3. imports it into a target Profile scope; and
4. records a new placement while leaving the source placement live.

Transfer is clone semantics, not move semantics. It can increase transient and retained
memory because source and target placements may coexist. Reconstructed BREP creates a new
OCCT object: logical identity is retained by `EngineShapeRef`, but native `TShape`
identity, subshape object identity, and operation-history references are not guaranteed to
survive transfer.

All shape inputs for one operation are co-located in one eligible Profile. A `batch` is
never silently split across Profile runtimes; cross-Profile batches are rejected. Release
an individual placement with `EngineShapeRef.releasePlacement()`, release every placement
for one logical shape with `GeometryEngine.releaseShape(ref.logicalId)`, or close the
engine to close all started runtimes.

The internal Profile transfer envelope must not be confused with the public BREP cache
contract. Public BREP bytes remain bound to the exact running WASM identity defined by the
[protocol](protocol.md#exchange-and-cache); they are not a general cross-version exchange format.

## Loading, integrity, and lifecycle

Profile and shared artifacts are described by the generated runtime configuration. The
artifact resolver chooses a caller-supplied resolver or base URL first, then the descriptor
URL/default release base. Official descriptors carry integrity and build identity data.

- Isolated official Profiles are hashed before Worker startup and their runtime
  `capabilities()` result is checked against the generated Profile operation set.
- A custom isolated descriptor may assert a hash, protocol version, ABI version, and build
  family. Static Profile types no longer describe arbitrary custom bytes; use the returned
  runtime capabilities.
- Shared Main and Sides must come from the same build family. A Side is loaded before its
  operation is dispatched, registered atomically, and checked against the declared loaded
  capability set.
- Shared Sides use nodelete semantics. Loading a Side is not an unloadable feature toggle;
  destroy the Main epoch to reclaim it.
- The npm package contains the standalone full artifact. Shared and isolated artifacts are
  resolved from the release manifest or a caller-controlled mirror; runtime lazy loading
  does not reduce npm installation size.
- The default browser loader for isolated Profiles caches bytes by artifact name and hash
  and verifies every cache hit. Custom artifact loaders and shared Main/Side loading bypass
  that cache. Compiled `WebAssembly.Module` objects are not persisted by this library.

Never combine Main, Side, or Profile files from different releases or build families.
Artifact hashes, sizes, URLs, toolchain identities, and build families belong in
`protocol/artifacts.json`; long-lived prose should point there rather than duplicate their
current values.

## Memory and mobile boundaries

Three measurements answer different questions:

| Measurement | What it means | What it does not prove |
| --- | --- | --- |
| Artifact byte size | Bytes fetched, cached, and passed to WebAssembly compilation. | Compiled-code size, live OCCT heap use, or device memory pressure. |
| WASM linear-memory capacity | `WebAssembly.Memory.buffer.byteLength`, or `stats().wasmMemorySize`, for one live instance. | Bytes currently occupied by useful objects, JavaScript heap size, compiled-code memory, or total process memory. |
| Browser/process memory pressure | Combined compiled code, linear memories, JavaScript, graphics, files, and browser overhead under a concrete workload. | A portable per-device limit inferred from one caught exception. |

Linear memory grows in WebAssembly pages and does not normally shrink during an instance's
lifetime. Closing the owning runtime makes its memory eligible for reclamation, but the
time at which the browser returns memory is host-controlled. Keeping preview and full
runtimes alive together therefore defeats an exclusive low-memory switch even if neither
reports many live shape handles.

A successful `Memory.grow` to a declared maximum proves only that the WebAssembly memory
object reached that capacity. A `RangeError` for growth beyond a candidate's declared
maximum classifies that declaration, not the device's physical limit. At the wasm32
address-space ceiling, a request for another page is intrinsically invalid and likewise
does not measure available device RAM. Conversely, a page or Worker can be terminated by
the host without a catchable JavaScript error.

For a candidate-capacity test, reaching the candidate's declared maximum and failing only
on the intentional extra-page probe is a pass for that candidate. The device upper limit
remains indeterminate; the result must not be reported as the highest supported capacity.

Dedicated Workers isolate queues and handle epochs, not necessarily operating-system
memory failure. In iOS WebKit a Worker and page can share one WebContent process, so a hard
memory termination may remove both. An unfinished-load marker can report only that the
previous attempt did not complete; it cannot identify out-of-memory as the cause.

Profile choice can reduce code and capability footprint, but a smaller artifact does not
guarantee a proportionally smaller process peak. Measure the real model and operation on
the target host, and report artifact bytes, linear-memory capacity, and observed host
behavior separately.

## Capabilities and generated contracts

Official Profile client types are generated as method subsets. For example, a Preview
client does not statically expose modeling methods. This compile-time narrowing is a
developer aid; the loaded kernel capability check is still the enforcement boundary.
Code using untyped JavaScript, unsafe low-level requests, or a custom artifact must rely on
runtime `capabilities()`.

The following files have distinct responsibilities:

| Source | Responsibility |
| --- | --- |
| `protocol/operations.json` | Operation catalog and history classification. |
| `protocol/operation-contracts.json` | Request/result schemas and frozen protocol semantics. |
| `protocol/modules.json` | Semantic modules, implementation units, dependencies, Profiles, and transfer operations. |
| `protocol/artifacts.json` | Published artifact descriptors, identities, hashes, sizes, and locations. |
| `protocol/build-flags.json` | Build features reported by capabilities. |
| `docs/profile-capabilities.generated.md` | Generated human-readable Profile capability summary. |
| `ts/src/profile-clients.generated.ts` | Generated compile-time Profile client subsets. |
| `ts/src/runtime-manifest.generated.ts` | Generated runtime routing and artifact configuration. |

Change an authoritative input and run `npm run generate`. Do not hand-edit generated
outputs or preserve a second capability list in architecture prose.
