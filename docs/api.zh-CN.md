# TypeScript API 参考

英文完整规范：[api.md](api.md)。本文件按主题说明公开 API；协议字段、默认值和精确类型以英文规范及 `protocol/` 生成类型为准。

## 客户端与生命周期

- `DirectClient.create(wasm)` 在当前线程加载 wasm，适合 Node 或受控同步调用。
- `WorkerClient.create(factory, wasm)` 在 Web Worker/Node Worker 中异步执行，支持超时、进度、`AbortSignal` 和实例重建。
- `initialize()` 调用 `capabilities` 并校验协议版本。
- `beginScope()` 创建 `ShapeScope`；`end()`/`endScope` 批量释放形状。`release` 和 `releaseAll` 用于显式释放。
- `stats()` 返回 `liveShapeHandles`、`liveBufferBytes` 和 `wasmMemorySize`。
- `ShapeHandle` 携带客户端 epoch，不能伪造，也不能跨重建后的实例使用。

## 基元与拓扑

`ShapeScope` 提供 box、cylinder、sphere、cone、torus、wedge、half-space、vertex、polygon、wire、face、shell、solid、compound 和 compsolid。还提供按类型获取子形、邻接、形状类型、拓扑计数、相同形状判断和批处理。

## 曲线与曲面

支持 line、arc、circle、ellipse、hyperbola、parabola、offset、Bezier、BSpline、helix，以及 plane、cylinder、cone、sphere、torus、extrusion、revolution、ruled、offset、Bezier/BSpline surface。公开求值、导数、切向、曲率、连续性、参数域、解析几何、控制点/权重/节点读取与 copy-on-write 编辑。

编辑操作包含节点插入/删除、升阶、周期性转换、U/V 交换和反向、控制网编辑、裁剪、扩展、重参数化、曲线/曲面转 BSpline、点集近似、曲率/长度/扭率 fairing、N-side filling 和带 pcurve 的曲面裁剪。

## 建模与几何运算

提供 extrude、revolve、loft、pipe、pipe shell、boolean fuse/cut/common、section、split、general fuse、glue/imprint、draft、local prism/revolution、cylindrical hole、defeature、fillet、chamfer、hollow、offset、sew、fixShape、unifySameDomain、shapeUpgrade 和刚体/镜像/缩放/仿射变换。

支持 `includeHistory` 的操作会返回 retained/generated/modified/deleted 映射。历史是单次操作的局部拓扑历史，不是跨任意操作的通用 Persistent Topological Naming；辅助脊线的 pipe-shell 历史明确为 partial。

## 查询与诊断

`bbox`、`obb`、`massProps`、`distance`、`classifyPoint`、`isValid`、`diagnoseShape`、`inspectTolerances`、`setTolerance`、曲线/曲面投影、极值、交运算、`sectionAnalysis`、`middlePath` 和 `projectHLR` 均通过消息协议公开。

## 网格处理

`tessellate` 返回逐面隔离的 positions、normals、indices、faceGroups 和可选 UV；`tessellateEdges` 返回 edgeGroups。`triangulationData`、`replaceTriangulation`、`validateTriangulation` 和 `repairTriangulation` 访问或修复 OCCT 面网格。

TypeScript 侧还提供 weld、法线/切线生成、simplify、subdivide、boundary loop、planar hole filling，以及面/边选择映射。网格 API 会校验非有限值、越界索引、重复索引、退化三角形和非流形边界。

## 参数化 CAD

`ParametricModel` 支持命名参数和安全算术表达式、特征 DAG、循环检测、序列化 schema v1、原子 recompute、失败回滚、suppressed 特征、每特征诊断、子形引用和可用 OCCT 历史组合。

可序列化特征包括基元、polygon/face、extrude、revolve、loft、pipe、fillet、chamfer、hollow、offset、draft、local feature、hole、defeature、修复、section、split、sew、变换和 boolean。还支持 Bezier/BSpline 曲线曲面、2D sketch 约束求解、多 profile 草图和参数化装配定义。

## 交换格式

支持内存流 BREP、STEP/AP203/AP214/AP242、STEP XCAF、IGES、IGES XCAF、STL、OBJ、PLY、VRML、glTF/GLB 和原生 XCAF binary/XML。公开能力包括多根、单位、装配实例、颜色、层、材料、验证属性、视图、SHUO、datum、GDT、OBJ/MTL、PLY 元数据、glTF 资源/动画/蒙皮/morph 和 Draco/meshopt 压缩预处理，具体格式限制见 [capabilities.zh-CN.md](capabilities.zh-CN.md)。

所有二进制交换通过内核 buffer；导入支持 `ArrayBuffer`/`SharedArrayBuffer`，Worker 会在普通传输后释放临时内核缓冲。

## 错误、取消与限制

错误使用稳定 `KernelError.code`，包括 `InvalidArgs`、`InvalidHandle`、`ConstructionFailed`、`BooleanFailed`、`FilletFailed`、`TessellationFailed`、`ImportExportFailed`、`HealingFailed`、`Cancelled`、`OutOfMemory`、`KernelError` 和 `ProtocolError`。错误详情包含操作名，交换错误还包含格式名。

DirectClient 的同步调用不承诺硬超时；不受信 CAD 文件应交给 WorkerClient。可视化、pthread/TBB、通用 OCAF 和未列入能力矩阵的 OCCT toolkit 不属于 v1。
