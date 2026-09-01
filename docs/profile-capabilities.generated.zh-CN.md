<!-- Generated from protocol/modules.json. Do not edit. -->

# Profile capabilities / Profile 能力

[English reference / 英文规范](profile-capabilities.generated.md)

Profile 与实现单元、产物和运行时实例的关系见[运行时架构](architecture.zh-CN.md)。

编译期能力收窄只适用于下表由官方定义的 profile。传给 `createWorkerProfileRuntime({ artifact })` 的自定义 wasm 产物可能具有不同能力，因此必须用运行时 `capabilities()` 检查自定义产物。

类型收窄不会从底层运行时对象移除方法。绕过 TypeScript 的代码仍可能尝试不支持的调用；内核能力检查仍是运行时的强制边界。

每个 Profile 还包含 runtime operation，以及 isolated 路由内部使用的 BREP 迁移 operation。Semantic modules / 语义模块列只列出选定的非 runtime 模块；Operations / 操作的计数包含这两组公共 operation 在内的全部可调用 Profile operation。

| Profile / Profile | Artifact / 产物 | Alias of / 别名 | Semantic modules / 语义模块 | Operations / 操作 |
| --- | --- | --- | --- | ---: |
| `core-modeling` | `core-modeling.wasm` | - | `geometry-topology`, `topology-query`, `modeling`, `algorithms` | 134 |
| `mesh` | `mesh.wasm` | - | `geometry-topology`, `topology-query`, `modeling`, `algorithms`, `tessellation`, `mesh` | 140 |
| `exchange` | `exchange.wasm` | - | `step-shape-exchange`, `cad-document-exchange`, `exchange-mesh` | 32 |
| `preview` | `preview.wasm` | - | `topology-query`, `tessellation`, `step-shape-exchange` | 25 |
| `modeling-viewer` | `mesh.wasm` | `mesh` | `geometry-topology`, `topology-query`, `modeling`, `algorithms`, `tessellation`, `mesh` | 140 |
| `full-profile` | `full.wasm` | - | `geometry-topology`, `topology-query`, `modeling`, `algorithms`, `tessellation`, `mesh`, `exchange-mesh`, `step-shape-exchange`, `cad-document-exchange` | 161 |
