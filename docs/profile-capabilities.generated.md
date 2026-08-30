<!-- Generated from protocol/modules.json. Do not edit. -->

# Profile capabilities

Compile-time capability narrowing applies only to the official profiles below. A custom wasm artifact supplied through the LGPL replacement path can differ from these types, so custom artifacts must be checked with runtime `capabilities()`.

Type narrowing does not remove methods from the underlying runtime object. Code that bypasses TypeScript can still attempt unsupported calls; the kernel capability check remains the runtime enforcement boundary.

| Profile | Artifact | Alias of | Semantic modules | Operations |
| --- | --- | --- | --- | ---: |
| `core-modeling` | `core-modeling.wasm` | - | `geometry-topology`, `topology-query`, `modeling`, `algorithms` | 134 |
| `mesh` | `mesh.wasm` | - | `geometry-topology`, `topology-query`, `modeling`, `algorithms`, `tessellation`, `mesh` | 140 |
| `exchange` | `exchange.wasm` | - | `step-shape-exchange`, `cad-document-exchange`, `exchange-mesh` | 32 |
| `preview` | `preview.wasm` | - | `topology-query`, `tessellation`, `step-shape-exchange` | 25 |
| `modeling-viewer` | `mesh.wasm` | `mesh` | `geometry-topology`, `topology-query`, `modeling`, `algorithms`, `tessellation`, `mesh` | 140 |
| `full-profile` | `full.wasm` | - | `geometry-topology`, `topology-query`, `modeling`, `algorithms`, `tessellation`, `mesh`, `exchange-mesh`, `step-shape-exchange`, `cad-document-exchange` | 161 |
