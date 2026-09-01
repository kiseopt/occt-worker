<!-- Generated from protocol/modules.json. Do not edit. -->

# Profile capabilities

[中文说明 / Chinese translation](profile-capabilities.generated.zh-CN.md)

For how Profiles relate to implementation units, artifacts, and runtime instances, see the [runtime architecture](architecture.md).

Compile-time capability narrowing applies only to the official profiles below. A custom wasm artifact passed to `createWorkerProfileRuntime({ artifact })` can differ from these types, so custom artifacts must be checked with runtime `capabilities()`.

Type narrowing does not remove methods from the underlying runtime object. Code that bypasses TypeScript can still attempt unsupported calls; the kernel capability check remains the runtime enforcement boundary.

Every Profile also includes the runtime operations and the BREP transfer operations used internally by isolated routing. The Semantic modules column lists only the selected non-runtime modules; Operations counts all callable Profile operations, including both common groups.

| Profile | Artifact | Alias of | Semantic modules | Operations |
| --- | --- | --- | --- | ---: |
| `core-modeling` | `core-modeling.wasm` | - | `geometry-topology`, `topology-query`, `modeling`, `algorithms` | 134 |
| `mesh` | `mesh.wasm` | - | `geometry-topology`, `topology-query`, `modeling`, `algorithms`, `tessellation`, `mesh` | 140 |
| `exchange` | `exchange.wasm` | - | `step-shape-exchange`, `cad-document-exchange`, `exchange-mesh` | 32 |
| `preview` | `preview.wasm` | - | `topology-query`, `tessellation`, `step-shape-exchange` | 25 |
| `modeling-viewer` | `mesh.wasm` | `mesh` | `geometry-topology`, `topology-query`, `modeling`, `algorithms`, `tessellation`, `mesh` | 140 |
| `full-profile` | `full.wasm` | - | `geometry-topology`, `topology-query`, `modeling`, `algorithms`, `tessellation`, `mesh`, `exchange-mesh`, `step-shape-exchange`, `cad-document-exchange` | 161 |
