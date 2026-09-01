# 运行时与 WASM 架构

[English reference / 英文规范](architecture.md)

本页定义已发布运行时的公开架构：协议操作如何组成语义模块、实现单元、Profile、产物和
运行时实例，并定义形状所有权、加载、失败与内存边界。精确操作签名由[协议规范](protocol.zh-CN.md)
负责；当前 Profile 到 operation 的映射以生成的
[Profile 能力表](profile-capabilities.generated.zh-CN.md)为准。

## 架构模型

“分层”不是一条二进制层级链。实现将下列概念分开，使 API 组织、链接边界、发布文件和
应用选择可以独立演进：

```text
operation 目录
       |
       v
语义模块
       |
       v
实现单元及依赖闭包
       |
       v
Profile                          shared Side 映射
       |                                  |
       v                                  v
isolated Profile 产物            shared Main + Side 产物
       \                                  /
        +---------- 运行时实例 ------------+
                          |
                          v
                    应用运行方式
```

| 概念 | 含义 | 权威来源 |
| --- | --- | --- |
| Operation | 一条公开协议命令，具有稳定的请求、结果、错误和 history 语义。 | `protocol/operations.json` 和 `protocol/operation-contracts.json` |
| 语义模块 | 将每个 operation 分配给唯一语义所有者的能力分类。它描述操作做什么，不一定对应文件边界。 | `protocol/modules.json` 的 `semanticModules` |
| 实现单元 | 组成链接级构建单元的源码、OCCT toolkit 和依赖。实现单元构成无环依赖图。 | `protocol/modules.json` 的 `implementationUnits` |
| Profile | 为 isolated 运行时定义的、经过命名和测试的实现单元组合。 | `protocol/modules.json` 的 `profiles` |
| Artifact / 产物 | 具体发布文件，例如 standalone Profile WASM、shared Main、Side WASM 或 Main JavaScript glue。 | `protocol/artifacts.json` |
| Backend / 运行形式 | 产物的实例化和形状所有权方式：standalone、shared 或 isolated。 | 公开客户端和生成的运行时 manifest |
| 产品预设 | 应用层选择，例如在预览与完整功能之间切换。它可以复用某个 Profile，但不是新的内核抽象。 | 应用配置 |

`protocol/modules.json` 是拓扑定义的单一事实源。Profile 生成器据此派生 CMake 源码/toolkit
闭包、Profile 客户端类型、运行时路由数据、shared Side 映射、CI 体积预算和 Profile 能力表。
不得直接编辑生成文件。

## Profile

Profile 是能力集合，不是质量等级，也不构成严格的从小到大链。当前正式 Profile 为：

| Profile ID | 用途 | 重要边界 |
| --- | --- | --- |
| `preview` | 导入和导出 STEP/BREP shape、探测全部文档化格式、查询拓扑和包围盒，并生成用于显示的面/边三角化数据。 | 不含基元、一般几何构造、布尔、特征建模、网格修复或 XCAF/CAD document exchange。 |
| `core-modeling` | 几何和拓扑构造、特征建模、布尔、分析与修复。 | 不含三角化、网格编辑或一般交换格式。 |
| `mesh` | `core-modeling` 加三角化，以及 triangulation 的检查、替换、验证和修复。 | 不含一般交换格式。 |
| `modeling-viewer` | 与 `mesh` 使用相同能力集合和产物的产品侧名称。 | 它是别名，不是单独构建的 WASM。 |
| `exchange` | 不含建模栈的 shape/document 与网格格式交换能力。 | 它是正交的交换预设，不是更大或更小的建模层。 |
| `full-profile` | 在一个 isolated Profile 运行时中包含全部公开 operation。 | ID 是 `full-profile`，产物文件是 `full.wasm`。 |

生成能力表包含精确语义模块、artifact 别名和 operation 数量。对于实际加载的字节，运行时
`capabilities()` 仍是最终依据；用户自定义产物尤其如此。

避免使用含糊的“最小核心”和“完全核心”：

- `preview` 是最小的正式查看 Profile，不是通用 CAD 核心。
- `core-modeling` 是最小的正式建模 Profile，不一定是理论上最小的自定义二进制。
- `full-profile` 是完整 isolated Profile；它不同于默认 standalone 完整产物，也不同于
  加载全部 Side 后的 shared 运行时。

应用预设应直接使用正式 Profile ID。产品预设可以改变运行时的启动或关闭时机，但不会改变
Profile 的 operation 集。

## 运行形式

各运行形式共享公开 operation 契约，但加载、对象身份、清理和失败影响不同。

| 运行形式 | 公开入口 | 产物与加载 | Shape 所有权 | 回收与失败影响 |
| --- | --- | --- | --- | --- |
| Standalone full | `DirectClient`、`WorkerClient` | 将 `wasm/occt-worker.wasm` 作为一个完整静态模块加载。Wasmtime 使用单独转换的 standalone 产物。 | 一个内核实例、一个 arena 和本地 `ShapeHandle`。 | 关闭或重建实例；该实例的全部句柄失效。 |
| Shared Main/Side | `SharedClient`；需要高层 `BaseClient` 调用面时可配合 `EngineCompatClient` | 实例化 Main 和 JavaScript glue，在某 operation 首次需要前加载其映射的 Side；并发首次加载共用一个 Promise。 | Main 与所有已加载 Side 共用一个 memory、table、allocator、kernel arena 和 handle 命名空间。 | Side 在 Main epoch 内常驻。关闭 shared client；若它运行在 dedicated Worker 中，再终止该 Worker。Main 失败或重建会使整个 shared epoch 失效。 |
| Isolated Profile | `createProfileClient`，或将 `createWorkerProfileRuntime` 注册到 `GeometryEngine` | 每个 Profile 是独立 Worker/运行时中的静态 WASM。`createProfileClient()` 会立即启动；`createWorkerProfileRuntime()` 返回的 factory 在调用时启动。 | 句柄只属于一个 Profile 实例。`GeometryEngine` 暴露逻辑 `EngineShapeRef`，并可在多个 Profile 中维护 placement。 | Profile epoch 失效会使该 Profile 的 placement 失效；在浏览器进程边界允许的前提下，其他 Profile 仍可继续。 |

应选择上表中的具体入口。`GeometryEngine` 协调 isolated Profile 运行时；`SharedClient` 是直接的 shared Main/Side
客户端。可以通过 `EngineCompatClient` 将 shared client 适配到旧高层 API，但这不会把
shared Main/Side 变成多个 isolated Profile Worker。

### “full”的三种含义

描述完整能力时应使用精确名称：

| 名称 | 含义 |
| --- | --- |
| Standalone full | `DirectClient`、`WorkerClient` 使用的默认静态 `wasm/occt-worker.wasm`，也是 Wasmtime 转换的源产物。 |
| `full-profile` | isolated Profile ID，其发布产物为 `full.wasm`。 |
| 完整 shared 运行时 | 一个 shared Main epoch 已加载全部所需 Side 后的状态；不存在名为“shared full”的单独产物。 |

这些形式可以暴露相同的协议 operation 集，但二进制、启动路径、内存布局和失败范围并不相同。

## 选择运行方式

| 需求 | 运行方式 |
| --- | --- |
| 最简单部署、npm 内置 WASM、Node.js 进程内执行或 Wasmtime | Standalone full |
| 同一建模会话需要保留原生 shape identity，并按 operation 加载能力 | Shared Main/Side |
| 只启动场景所需的能力集合、分别运行 Profile，或回收某个 Profile 运行时 | Isolated Profile |
| 应用的 preview/full 模式互斥，绝不能同时保留两个运行时 | 用 isolated Profile factory 创建 `SingleRuntimeSession` |

应用层从 preview 切换到 full 时，应先关闭旧运行时，再创建替代运行时。
`SingleRuntimeSession.switchProfile()` 会拒绝待处理工作，等待底层调用结束，关闭旧运行时，
然后才创建新运行时。如果替代运行时创建失败，会话进入关闭状态，不会静默恢复旧运行时；
应用必须重新加载模型或启动新会话。旧句柄不能跨过该边界。

`GeometryEngine` 服务于不同工作流：它可以让多个 isolated Profile 同时运行，并在它们之间
路由操作。当产品要求先释放旧 Profile 时，不要用它替代互斥运行时切换。

## Shape 所有权与迁移

### Standalone 与 shared 句柄

`ShapeHandle` 属于创建它的客户端、scope 和 epoch。Shared Main 与 Side 使用同一个 arena，
因此一个已加载 Side 生成的 shape 可由另一个已加载 Side 直接消费。数值句柄永远不能移植到
另一个 standalone 实例、Profile Worker 或重建后的 epoch。

结束 scope 会释放其中剩余 shape；关闭或重建客户端会使上一 epoch 的全部句柄失效。

### Isolated 逻辑 shape

`GeometryEngine` 使用不透明 `EngineShapeRef` 表示逻辑 shape。一个逻辑 shape 可以在每个
需要它的 Profile 中拥有一个本地 placement。当 operation 必须在另一个 Profile 中运行时，
engine 会：

1. 通过内部 BREP 迁移路径导出源 placement；
2. 用迁移版本、build-family 身份、长度和 checksum 包装 payload；
3. 将其导入目标 Profile 的 scope；
4. 记录新 placement，并让源 placement 继续有效。

迁移采用 clone 语义，不是 move 语义。源与目标 placement 可同时存在，因此迁移可能增加瞬时
和保留内存。由 BREP 重建会产生新的 OCCT 对象：`EngineShapeRef` 保留逻辑身份，但不保证原生
`TShape` 身份、子形状对象身份或 operation history 引用跨迁移保持不变。

一次 operation 的全部 shape 输入会共置到一个可用 Profile。`batch` 绝不会被静默拆到多个
Profile 运行时；跨 Profile batch 会被拒绝。使用 `EngineShapeRef.releasePlacement()` 释放
一个 placement，使用 `GeometryEngine.releaseShape(ref.logicalId)` 释放一个逻辑 shape 的全部
placement，或关闭 engine 以关闭全部已启动运行时。

内部 Profile 迁移 envelope 不能与公开 BREP 缓存契约混淆。公开 BREP 字节仍绑定
[协议](protocol.zh-CN.md#交换与缓存)定义的精确运行中 WASM 身份，不是通用跨版本交换格式。

## 加载、完整性与生命周期

Profile 和 shared 产物由生成的运行时配置描述。Artifact resolver 优先使用调用方提供的
resolver 或 base URL，否则使用 descriptor URL 或默认 release base。正式 descriptor
带有完整性和构建身份数据。

- 正式 isolated Profile 在 Worker 启动前校验哈希，并将运行时 `capabilities()` 与生成的
  Profile operation 集核对。
- 自定义 isolated descriptor 可以声明 hash、协议版本、ABI 版本和 build family。静态
  Profile 类型不再描述任意自定义字节；应使用实际返回的运行时 capabilities。
- Shared Main 与 Side 必须来自同一 build family。Side 会在 operation 派发前加载，原子
  注册，并与声明的已加载能力集合核对。
- Shared Side 使用 nodelete 语义。加载 Side 不是可卸载的功能开关；必须销毁 Main epoch
  才能回收。
- npm 包包含 standalone 完整产物。Shared 和 isolated 产物从 release manifest 或调用方
  镜像解析；运行时懒加载不会减少 npm 安装体积。
- 浏览器中的默认 isolated Profile loader 以 artifact 名称和 hash 为键缓存字节，并在每次
  命中时重新校验；自定义 artifact loader 和 shared Main/Side 加载不使用该缓存。本库不会
  持久化编译后的 `WebAssembly.Module` 对象。

不得混用不同 release 或 build family 的 Main、Side 或 Profile 文件。Artifact hash、大小、
URL、工具链身份和 build family 属于 `protocol/artifacts.json`；长期文档应指向该文件，
而不是复制当前数值。

## 内存与移动端边界

三个指标回答不同问题：

| 指标 | 含义 | 不能证明什么 |
| --- | --- | --- |
| Artifact 字节大小 | 获取、缓存并交给 WebAssembly 编译的字节数。 | 编译代码大小、活动 OCCT heap 使用或设备内存压力。 |
| WASM 线性内存容量 | 一个活动实例的 `WebAssembly.Memory.buffer.byteLength` 或 `stats().wasmMemorySize`。 | 有效对象实际占用字节、JavaScript heap、编译代码内存或进程总内存。 |
| 浏览器/进程内存压力 | 具体负载下编译代码、线性内存、JavaScript、图形、文件和浏览器开销之和。 | 从一次可捕获异常推导出的通用设备上限。 |

线性内存按 WebAssembly page 增长，在实例生命周期内通常不会缩小。关闭所属运行时后，该内存
才具备被回收的条件，但浏览器何时归还内存由宿主控制。因此，即使两个实例都没有很多活动
shape handle，同时保留 preview 与 full 运行时也违背互斥低内存切换的目标。

`Memory.grow` 成功达到声明 maximum，只证明 WebAssembly memory object 达到了该容量。
增长超过候选声明 maximum 时的 `RangeError` 只分类该声明，不代表设备物理上限。在 wasm32
地址空间上限处，再请求一页本身就是无效操作，同样不能测量可用设备 RAM。反过来，页面或
Worker 可能被宿主终止，而不给出可捕获 JavaScript 错误。

对于候选容量测试，若已达到候选声明 maximum，仅在故意多增长一页时失败，该候选应判定为
通过。设备上限仍然无法确定；不得把该结果报告为设备支持的最高容量。

Dedicated Worker 隔离队列和 handle epoch，不一定隔离操作系统内存失败。在 iOS WebKit 中，
Worker 与页面可能共享同一个 WebContent 进程，因此硬内存终止可以同时移除两者。未完成加载
标记只能说明上次尝试没有完成，不能将原因确定为内存不足。

Profile 选择可以降低代码和能力 footprint，但更小的 artifact 不保证进程峰值按比例降低。
应在目标宿主上用真实模型和 operation 测量，并分别报告 artifact 字节、线性内存容量和宿主
行为观测。

## Capabilities 与生成契约

正式 Profile 客户端类型生成为方法子集。例如 Preview client 在静态类型上不暴露建模方法。
这种编译期收窄是开发辅助；已加载 kernel 的 capability 检查仍是强制边界。未类型化
JavaScript、不安全低层 request 或自定义 artifact 必须依赖运行时 `capabilities()`。

下列文件各自承担不同职责：

| 来源 | 职责 |
| --- | --- |
| `protocol/operations.json` | Operation 目录和 history 分类。 |
| `protocol/operation-contracts.json` | 请求/结果 schema 和冻结的协议语义。 |
| `protocol/modules.json` | 语义模块、实现单元、依赖、Profile 和迁移 operation。 |
| `protocol/artifacts.json` | 发布 artifact descriptor、身份、hash、大小和位置。 |
| `protocol/build-flags.json` | capabilities 报告的构建特性。 |
| `docs/profile-capabilities.generated.zh-CN.md` | 生成的 Profile 能力摘要。 |
| `ts/src/profile-clients.generated.ts` | 生成的编译期 Profile 客户端子集。 |
| `ts/src/runtime-manifest.generated.ts` | 生成的运行时路由与 artifact 配置。 |

修改权威输入后运行 `npm run generate`。不得手改生成输出，也不要在架构文档中保存第二份
能力列表。
