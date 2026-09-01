# TypeScript API 参考

[English reference / 英文完整规范](api.md)

## API 速览

| 使用场景 | 公开入口 | 参考 |
| --- | --- | --- |
| Node.js 进程内核 | `DirectClient` 和 `ShapeScope` | 本 API 参考 |
| Node.js 或浏览器 Worker 内核 | `WorkerClient` | [使用指南](getting-started.zh-CN.md)和本 API 参考 |
| 建模、查询、网格和交换操作 | `ShapeScope` 方法 | [能力矩阵](capabilities.zh-CN.md) |
| 底层消息、缓冲区、错误和默认值 | `request()` 与协议帧 | [协议规范](protocol.zh-CN.md) |
| Wasmtime 或自定义宿主集成 | 同步 WebAssembly ABI | [宿主支持](hosts.zh-CN.md) |
| 参数化特征和草图 | `ParametricModel` 与特征定义 | 本 API 参考 |
| shared Main/Side 高层兼容 | `SharedClient` + `EngineCompatClient` | 本 API 参考 |

这些客户端与 standalone full、shared Main/Side 和 isolated Profile 运行时的对应关系，包括形状所有权和生命周期，见[运行时架构](architecture.zh-CN.md)。

## 客户端与生命周期

`DirectClient.create(wasm, imports?)` 创建当前线程内核；`WorkerClient.create(factory, wasm)` 创建带硬超时恢复的可释放 Worker 内核。两者都按生成的协议版本初始化，并提供查询、三角化和导出方法。

### 作用域与句柄

- `beginScope()` 返回拥有其创建的所有形状的 `ShapeScope`。应使用 `await scope.end()` 显式结束作用域，或使用 `await using`。
- `ShapeHandle` 实例带有客户端 epoch 校验，不能由原始数字构造，也不能传递给另一个或重建后的 kernel 实例。
- `scope.batch()` 是类型化的批量执行入口，会将每个顶层 `shape` 或 `shapes` 结果转换为带来源与所有权检查的 `ShapeHandle`。

### 缓冲区与低层请求

- `DirectClient.createBuffer()` 暴露同线程可复用的输入 buffer；每次 wasm 请求后都必须重新调用 `view()`（线性内存增长会使旧视图失效），通过低层协议传入 `{ bufferId }`，并使用 `DirectClient.freeBuffer()` 释放。`releaseAll()` 会使所有活动 direct buffer 失效。
- 低层 `request()` 是明确标记为不安全的原始协议接口：它接受原始 `u32` 形状句柄和缓冲区引用，并绕过 TypeScript 的所有权与 epoch 来源检查。普通业务代码应使用类型化方法。

### 自定义 isolated 内核

替换 isolated kernel 时，将自定义 `artifact` 传给 `createWorkerProfileRuntime`，并通过 `resolve` 或 `baseUrl` 解析到实际 URL；当调用方未设置这两项覆盖时，`defaultBase` 提供由宿主确定的回退基址。该路径仍会启动 Worker 并执行 `capabilities` 握手，不兼容的协议会拒绝加载。若 descriptor 提供 `sha256`、`protocolVersion`、`abiVersion` 或 `buildFamily`，客户端会分别与字节或本次构建进行核对。

生成的 profile 客户端类型只适用于 `protocol/modules.json` 声明的官方 profile。自定义 artifact 没有对应的静态能力收窄；加载后应读取 `runtime.request("capabilities", {})` 返回的 `ops`，再选择操作。

## 构造、建模与局部特征

构造与建模方法包括：

- 基元，以及顶点、多边形、线框、面、壳、实体、复合体和复合实体构造；
- 有界解析面，以及有限的直线、圆锥曲线、偏移曲线、Bezier、BSpline 和螺旋线边；
- 拉伸、旋转、直纹、偏移、填充、裁剪、近似和延展曲面；
- 拉伸、旋转、放样、管道扫掠、布尔运算和 General Fuse；
- 拔模、局部棱柱/旋转、筋/槽、圆柱孔和去特征；
- 偏移、修复和变换。

N 边填充默认使用 C0 边界，并可添加 G1/G2 支持面约束。曲面裁剪支持在现有曲面上使用任意外边界线框和孔线框。

`dist/client.d.ts` 中的方法选项类型是规范 TypeScript 签名；操作名称和 history 级别来自 `protocol/operations.json`。

### Fillet 与 chamfer 选项

`fillet()` 和 `chamfer()` 支持标量尺寸、逐边数组和半径变化规则：

- **标量模式：** `scope.fillet(shape, edgeIndices, radius)` 或 `scope.chamfer(shape, edgeIndices, distance)`。
- **逐边数组：** 与 `edgeIndices` 对齐的 `radii` 或 `distances` 数组，允许构建具有独立边倒角尺寸的轮廓。
- **线性变半径圆角：** `radius2` 定义从轮廓起点半径到终点 `radius2` 的线性过渡。
- **多点半径变化规则：** `radiusLaw` 对所有选中边使用同一组控制点；`radiusLaws` 为每条边分别指定控制点。每组控制点由从 `0` 到 `1` 严格递增的相对参数和对应的正数半径组成。
- **双距离模式（Chamfer）：** `distance2` 或 `distances2`，要求提供与 `edgeIndices` 对齐的 `referenceFaceIndices`。每个参考面必须邻接对应边，并指定第一距离所在侧。

```ts
// 示例：变半径圆角与双距离倒角
const filleted = await scope.fillet(baseBox, [0, 1], 2.0, {
  radius2: 5.0,
  includeHistory: true,
});

const chamfered = await scope.chamfer(baseBox, [2], 3.0, {
  distance2: 6.0,
  referenceFaceIndices: [0],
});
```

### General Fuse 单元选择

`selectGeneralFuseCells()` 会从输入形状重新运行 General Fuse，然后按规则筛选单元（Cell）：

- `take`: 从零开始的输入形状索引数组，单元必须来源于此集合。
- `avoid`: 从零开始的输入形状索引数组，单元不得包含此集合中的输入。
- `material`: 分配给选中单元的非零整数材质标签。
- `removeInternalBoundaries`: 合并相邻且具有相同非零材质标签的单元。
- `makeContainers`: 根据单元维度创建 wire、shell 或 compsolid。

### 局部特征与去特征

- `draftAngle()`: 对从零开始的输入面索引应用一个方向、角度和中性面（neutral plane）。
- `localPrism()`: 使用属于 base shape 的选中面执行非破坏的 `BRepFeat_MakePrism` add 或 cut。数值参数选择 length 模式；options 参数选择 `until`、`fromUntil`、`untilEnd`、`fromEnd` 或 `thruAll` 并附带界限形状。
- `localRevolution()`: 对应的 `BRepFeat_MakeRevol` 特征，支持 axis、angle、until/from 或 thru-all 模式。
- `linearForm()`: 原生 `BRepFeat_MakeLinearForm` 平面筋/槽特征：输入 profile wire、所在平面、厚度方向及可选反向方向（`operation: "add"` 或 `"cut"`）。
- `revolutionForm()`: 原生 `BRepFeat_MakeRevolutionForm` 旋转筋/槽特征，接受非负的 `height1`/`height2`。
- `cylindricalHole()`: 沿显式轴切削，支持 through-all、through-next、until-end、blind-length 和 axis-interval 模式。
- `defeature()`: 从实体、compsolid 或实体复合体中移除选定面并重建周围几何。
- `glue()`: 暴露 `BRepFeat_Gluer` 的显式局部印贴（imprint），通过匹配的面绑定和边绑定缝合。

## 曲线与曲面

### 子形状与曲线

- `scope.getSubShape(shape, type, index)`：返回从 0 开始索引的子形状句柄；该句柄由当前作用域管理。
- `curveDomain(edge)`：通过 `first` 和 `last` 返回原生参数区间，并返回 `periodic` 和 `period`。
- `curveGeometry(edge)`：返回解析曲线类型，以及该类型对应的几何参数字段。
- `curveContinuity(edge)`：返回 `c0`、`g1`、`c1`、`g2`、`c2`、`c3` 或 `cn`。
- `evaluateCurve(edge, parameter)`：返回 `point`、`d1` 和 `d2`。仅当 `tangentDefined` 或 `curvatureDefined` 表示对应值存在时，结果才包含 `tangent` 或 `curvature`。
- `curveControlData(edge)`：返回 Bezier 或 BSpline 的全部控制点、权重、节点、重数和周期性数据。
- `updateCurvePole(edge, index, point)`：复制底层曲线，修改一个从 0 开始索引的控制点并返回新边，不修改输入。新边保持原参数域和拓扑方向。
- `trimCurve()`：创建有限子区间边。`convertCurveToBSpline()` 转换这条有限边，而不是可能无界的基准曲线。

`makeEdgeHyperbola()` 和 `makeEdgeParabola()` 要求有限且严格递增的参数边界，因此返回的边具有有限拓扑域。`makeEdgeOffset()` 保持基准边的参数域，并遵循 OCCT 的“切线向量 × 参考方向”叉乘约定。

### 曲面

- `surfaceDomain(face)`：返回 `uFirst`、`uLast`、`vFirst`、`vLast`，以及 U/V 方向的周期性标记和周期。
- `surfaceGeometry(face)`：返回解析曲面类型，以及该类型对应的几何参数字段。
- `surfaceContinuity(face)`：分别返回 U 和 V 方向的连续性级别。
- `evaluateSurface(face, u, v)`：返回点、一阶和二阶 U/V 偏导数及混合偏导数。`normalDefined` 和 `curvatureDefined` 表示定向法线与曲率字段是否存在；非脐点结果还可包含两个主方向。
- `makeSurfaceExtrusion()`、`makeSurfaceRevolution()` 和 `makeSurfaceRuled()`：根据边的参数域构造有限面。
- `makeSurfaceOffset()`：替换支撑曲面，并保留输入面的外边界线框和孔线框。
- `makeSurfaceBezier()` 和 `makeSurfaceBSpline()`：使用 U 优先的 `poles[u][v]` 控制网，并可指定有理权重、节点、重数和周期性。
- `makeSurfaceFilling()`：根据闭合线框构造 N 边填充面。边界默认为 C0，可添加 G1/G2 支持面和内部点约束。
- `makeFaceOnSurface()`：复制任意外边界线框和孔线框，投影缺失的 pcurve，并裁剪现有支撑曲面；输入保持不变。
- `surfaceControlData()`：返回完整控制网。`updateSurfacePole()` 复制并修改一个控制点；`trimSurface()` 创建有限的矩形 UV 区域；两者均不修改输入。
- `convertSurfaceToBSpline()`：转换有限的解析面、Bezier 面或 BSpline 面。
- `surfaceIsoCurve(face, "u" | "v", parameter)`：创建按面的外边界和孔裁剪的作用域内边。`"u"` 表示固定 U，`"v"` 表示固定 V。

### BSpline 编辑与边界规则

- `editCurveBSpline()`：执行原生节点插入/删除、升阶、周期性转换或完整控制网替换，并返回新 BSpline 边。
- `editSurfaceBSpline()`：提供对应的 U/V 节点、阶数、周期性、方向和控制网操作，同时保留面的线框和 pcurve。
- `reparameterizeCurve()` 和 `reparameterizeSurface()`：精确仿射变换节点参数域；曲面边界 pcurve 会按新参数域重建。
- `reduceCurveDegree()` 和 `reduceSurfaceDegree()`：近似到请求的较低阶数，返回 `{ shape, maxError }`，并保留输入参数域和拓扑。
- 近似方法可通过 `variationalSmoothing` 的长度、曲率和挠率权重启用 OCCT 光顺。此模式使用 `degreeMax`、连续性和容差，不使用参数化方式或 `degreeMin`。

`makeSurfaceFilling.constraints` 按线框顺序使用从 0 开始的边索引，并通过支持面指定 `g1` 或 `g2` 连续性。未指定的边保持 C0。重复或越界的索引，以及缺少对应边 pcurve 的支持面都会被拒绝。`trimSurface()` 只接受一个四边界区域；任意裁剪拓扑会被拒绝，不会被静默丢弃。

## 查询与诊断

- `scope.middlePath()`：提取管状形状中两个所属面或线框之间的 OCCT 中心路径。
- `inspectTolerances()`：返回稳定且从 0 开始索引的面、边和顶点容差。`scope.setTolerance()` 在深复制结果上修改一个或全部拓扑层级，不修改输入。
- 投影、极值和交运算只针对有限拓扑，并遵守面的孔和外边界。驻点解连续时，极值结果设置 `parallel`，并返回具有代表性的全局最近点对。
- `intersectCurveCurve()` 和 `intersectSurfaceSurface()`：为孤立结果或一维结果创建受作用域管理的顶点或边句柄。
- `distance()`：返回全部最小距离解、内部实体状态，以及每个解的顶点、边或面支持信息。适用时还返回边参数和面 UV 参数。若 OCCT 的内部解引用了不属于相应输入的子形状，该侧可以不返回支持索引。
- `scope.sectionAnalysis()`：返回受作用域管理的截面形状、截面边索引与长度、两个输入上的可选祖先面索引，以及孤立截面顶点索引。还可请求曲线近似和输入 pcurve，且不改变 `section()` 的行为。
- `scope.projectHLR()`：执行不经过三角化的精确 B-Rep 隐藏线消除。它接受从相机指向模型的 `direction`、`up` 方向以及平行或透视投影；透视投影还要求正的 `focus`。返回的 `visible` 和 `hidden` 边复合体使用视图坐标：X 向右，Y 沿投影后的上方向，Z 为 0。

## 网格处理

### 三角化与网格修复

- `weldTessellation()`：在位置容差内合并顶点，默认保留硬法线和 UV 接缝，删除合并后塌缩的三角形，并重写 `faceGroups`。
- `recalculateVertexNormals()`：计算面积加权顶点法线。
- `generateTangents()`：为带 UV 的三角化结果生成归一化 `f32x4` 切线和手性数据。
- `findBoundaryLoops()`：返回顺序确定、方向一致且带索引的边界环。
- `fillPlanarHoles()`：对选定的简单平面环进行三角化，追加调用方指定的面组，保留位置与 UV，并重建法线。非流形、分支、方向不一致、非平面、退化和自交边界都会被拒绝。该方法不会推断外边界；开放表面同时包含外边界和孔时，应传入 `loopIndices`。
- `FaceSelectionMap`：把渲染三角形索引映射到从 0 开始的 OCCT 面索引，并提供每个面的反向三角形范围。`EdgeSelectionMap` 提供边折线顶点到 OCCT 边索引的对应关系。两者都会校验范围单位、边界和重叠。
- `simplifyTessellation()`：按 `targetTriangles` 或 `ratio` 执行确定性的顶点聚类简化，删除塌缩三角形，保留面组范围，并重建面积加权法线。
- `subdivideTessellation()`：执行一级或多级中点细分，插值 UV，保留面组范围，并重建法线。

内置的焊接、简化和细分操作会按输出索引重建 `faceGroups`，因此选择映射仍可使用。

### 网格交换

- STL、OBJ、PLY 和 VRML 保留仅交换形状的导入导出路径；启用网格或文档选项时，还会返回索引缓冲区和格式元数据。
- OBJ 图元范围按 `indices` 中的标量条目计数。
- PLY 的 `ascii`、`binary_little_endian` 和 `binary_big_endian` 输出都可由对应导入路径读取。
- glTF 文档路径保留源 JSON、已解析资源和图元实例范围；合法的无三角形文档可以没有 `shape`。

各格式的元数据边界和拒绝规则见下方[交换格式](#交换格式)及[协议规范](protocol.zh-CN.md)。

## 建模历史

- `fillet()` 和 `chamfer()` 启用 `includeHistory: true` 后，返回形状及完整的局部面/边历史，包括保留的子形状、生成的面、修改后的替代面和删除的输入。生成与删除相互独立，因此一条被删除的边仍可能生成圆角面或倒角面。
- `extrude()`、`revolve()`、`loft()` 和 `sweepPipe()` 返回 OCCT 构造器提供的完整局部历史。
- `sweepPipeShell()` 映射主脊线和轮廓，但 OCCT 不提供辅助脊线的映射，因此整体历史支持为部分支持。
- `sew()`、`fixShape()` 和 `unifySameDomain()` 返回修复流程的完整历史。`fixShape()` 会组合复制与修复历史，使映射仍指向调用方的原始形状。

## 参数化 CAD

### 特征图与序列化

`ParametricModel` 在任意 `BaseClient` 之上提供可序列化特征层：

- 表达式支持命名参数和不依赖 `eval` 的算术运算；
- 特征依赖按拓扑排序，循环依赖会被拒绝；
- `recompute()` 在新作用域中构建，并原子发布结果；
- 并发重算和释放操作会串行执行；
- 序列化输出使用 `schemaVersion: 1`；`fromJSON()` 会迁移原始无版本格式，并拒绝未知的未来版本；
- 特征可标记为 `suppressed`；`getFeatureDiagnostics()` 返回上次重算的 `ok`、`suppressed` 或 `failed` 状态。

### 持久子形状引用

可选的 `subshapeReferences` 用于命名特征中的面或边：

- `initialIndex` 只在第一次成功重算时捕获持久签名。后续重算忽略该索引，改为按几何类型和父形状归一化后的子形状边界进行唯一匹配。
- `source: "id"` 沿唯一的特征历史路径跟随已有引用。布尔、圆柱孔、去特征、拉伸、旋转、放样、管道和 pipe-shell 扫掠、圆角、倒角、缝合、形状修复、同域统一及变换会自动提供可用的 OCCT 历史。
- OCCT 不提供辅助 pipe-shell 脊线的历史，因此该拓扑明确标记为未映射。删除或未映射的来源变为 `missing`，一对多映射变为 `ambiguous`。
- 对称候选确实需要按原始拓扑顺序选择时，可设置 `disambiguation: "initialIndex"`。默认行为仍是拒绝歧义。
- 操作有意替换曲线或曲面类型、且归一化边界仍唯一时，可设置 `allowGeometryReplacement: true`。未显式启用时，几何类型变化仍视为 `missing`。
- 圆角、倒角、抽壳、拔模、局部拉伸/旋转和去特征的选择字段接受 `{ reference: "id" }`。解析后的引用必须属于该特征输入，且拓扑类型符合要求。
- 引用缺失或存在歧义时，下游特征失败，不会发布部分重算结果。
- `getSubshapeReference()` 返回当前受作用域管理的句柄；`getSubshapeReferenceDiagnostics()` 返回 `resolved`、`missing` 或 `ambiguous` 状态及候选索引；`resolvePersistentSubshape(source, historyChain)` 用于显式组合历史。

签名匹配适用于保持归一化位置的尺寸变化和均匀拓扑变化，但不是缺少 OCCT 历史时的通用 B-Rep 命名算法。按初始索引消歧要求拓扑顺序稳定；允许几何类型替换则要求边界唯一。

### 特征定义与装配

- 基元特征包括长方体、圆柱、球体、圆锥、环面和楔体；轮廓特征包括多边形和面。
- 建模特征包括拉伸、旋转、放样、管道和 pipe-shell 扫掠、圆角、倒角、抽壳、偏移、拔模、局部拉伸、圆柱孔、去特征、平移、旋转、缩放、镜像、一般仿射变换，以及布尔切除/融合/求交。
- 可选的可序列化 `document` 定义带索引的零件和装配节点。重复零件节点可引用同一特征作为组件实例，并携带参数驱动变换、产品名称、RGBA 颜色、图层、可见性、物理与 PBR 材质及验证属性。
- `recompute()` 完成后，`toSTEPDocumentDefinition()` 将特征 ID 和变换表达式解析为 `exportSTEPDocument()` 与 `exportXCAF()` 接受的文档定义。

### 自由形几何与草图

- 参数化 `bezierCurve`、`bsplineCurve`、`bezierSurface` 和 `bsplineSurface` 特征可序列化自由形控制几何，并解析每个控制点坐标中的表达式。对应 `ShapeScope` 构造器支持时，也可使用有理权重、BSpline 次数、节点、重数、周期性标志和构造容差。
- `solveSketch()` 接受可序列化的二维直线、圆、圆弧和 BSpline 实体。几何约束包括 `coincident`、`fixed`、`horizontal`、`vertical`、`parallel`、`perpendicular`、`tangent`、`concentric`、`symmetry`、`equalLength` 和 `equalRadius`；尺寸约束包括 `distance`、`length` 和 `radius`。
- 结果包含收敛状态、残差、迭代次数、自由度和逐约束残差诊断。
- 相切约束适用于直线、圆和圆弧组合。BSpline 端点可以参与点引用约束，但 BSpline 相切不在此求解器范围内。未知约束和不支持的实体组合会被拒绝。
- 构造实体参与求解但不出现在结果中。参数化 `sketch` 会解析尺寸表达式，并把结果放置在任意三维平面上。
- 不提供 `profiles` 时，草图输出一条线框；有序轮廓会输出组合和稳定的 `sketchId.profileId` 线框引用，可直接用作 `face.outer` 和 `face.holes`。

特征图还支持缝合、形状修复、同域统一、截面和分割操作。

## Shape upgrade

`ShapeScope.shapeUpgrade()` 提供以下 OCCT ShapeUpgrade 模式：

- `continuity`：分割低连续性几何；
- `angle`：限制旋转类曲面的角跨度；
- `area`：按最大面积、近似分块数或固定 U/V 分割数拆分面；
- `closedFaces` 和 `closedEdges`：打开周期拓扑；
- `convertToBezier`：转换选定的 2D/3D 曲线和曲面类型；
- `removeInternalWires`：移除面积低于指定值的孔；
- `removeLocations`：把选定拓扑的位置变换固化到几何中。

分割和转换模式接受精度范围、曲面分段和适用的边模式（`0` 表示自由边，`1` 表示共享边，`2` 表示全部边）。这些操作可能改变拓扑，不提供持久拓扑历史。

## 网格与缓冲区边界

- 大型 BSpline 控制点数组可使用按 `f64x3` 紧密排列的 `Float64Array`。
- `tessellate()` 返回按面隔离的 `Float32Array` 位置和法线、`Uint32Array` 索引和分组，以及可选 UV。
- `triangulationData()` 读取现有面网格，不重新三角化；`validateTriangulation()` 报告属性和三角形问题。
- `replaceTriangulation()` 复制输入面，应用调用方提供的位置、索引、法线和 UV，并返回新面；输入保持不变。
- `repairTriangulation()` 复制输入形状，使用 OCCT BRepMesh 重建面网格并生成法线。
- 二进制导入接受 `ArrayBuffer` 和 `SharedArrayBuffer`。`WorkerClient` 会传输并分离普通输入缓冲区；共享输入仍由调用方持有，跨 Worker 边界时不会克隆。
- `WorkerClient.requestShared()` 为每个内核输出缓冲区新建 `SharedArrayBuffer`，并从 WASM 线性内存复制数据。共享缓冲区不会被传输；该过程仍有一次复制，并非直接零拷贝访问 WASM 内存。
- 普通导出和三角化缓冲区会在 Promise 完成前复制到宿主并释放内核缓冲区。

## 交换格式

所有交换方法均使用内存流。

### STEP 与 IGES 形状交换

- STEP 和 IGES 接受 `mm`、`cm`、`m`、`inch` 或 `foot` 作为模型单位。
- STEP 形状和文档导出接受 `schema: "AP203" | "AP214" | "AP242"`，默认值为 `AP242`。导出使用调用方传入的时间戳或协议默认值，不读取系统时钟。
- `exportSTEP()` 和 `exportIGES()` 接受单个形状或由多个独立根组成的数组。导入返回 `{ shape, shapes, rootCount }`：`shape` 是 OCCT 合并后的结果，`shapes` 包含每个传输根对应的受作用域管理句柄。
- 仅交换形状时，IGES 还接受 `mode: "faces"`（默认）或 `mode: "brep"`。
- `Matrix12` 是公开变换类型，使用协议定义的行主序 3x4 仿射布局。

### XCAF 文档

- `exportSTEPDocument()` 和 `ShapeScope.importSTEPDocument()` 往返传输带索引的零件/装配树。节点可包含局部变换、名称、RGBA 颜色、图层、可见性、物理材料元数据、验证属性和受作用域管理的形状句柄。
- 材料密度不得为负。省略密度元数据时，单位和类型分别默认为 `kg/m^3` 和 `mass density`。原生 XCAF 保留全部材料字段。STEP 保留材料名称、描述、密度和密度名称，但可能省略 `densityValueType`；PBR 视觉材质也只能通过显示颜色表示。
- 原生 `exportXCAF()` 和 `importXCAF()` 在二进制文件及 Worker 生成的 XML 中保留完整的公开 PBR 字段。节点 RGBA 颜色、子形状 RGBA 样式、带颜色的 SHUO 路径及第三方 XmlXCAF 输入必须使用 XCAF 二进制格式；XML 导出会直接拒绝这些字段，避免静默降低颜色精度。
- `subshapeStyles` 按 `getSubShape()` 的同一顺序，使用从 0 开始的 `face` 或 `edge` 索引。面样式映射为 XCAF 表面颜色，边样式映射为曲线颜色。这些样式属于零件定义；重复 STEP 实例的逐实例覆盖需要使用 SHUO。
- 原生 XCAF `views` 保留命名相机的投影方式、方向、缩放/窗口、可选视体裁剪、关联节点索引，以及带原点、法向量和封盖状态的裁剪平面标签。
- 原生 XCAF `shuo` 把组件路径保存为从上层用法到下一层用法的节点索引序列。可选的逐实例 RGBA `color` 要求使用二进制格式。
- 尺寸、基准和几何公差可包含 `presentation: { shape, name? }`。导入时，图形标注拓扑以受作用域管理的 `ShapeHandle` 返回。
- 基准记录保留名称、描述、标识、可选语义名称和所引用的零件节点。几何公差记录保留类型、数值、可选数值类型、语义名称、关联的 `datumIndices`、`materialRequirement`、`zoneModifier`/`zoneModifierValue`、全部公开 OCCT `modifiers` 和 `maxValueModifier`。
- STEP 会拒绝无法保留的链接，包含 GDT 的 STEP 文档必须只有一个根节点。这里提供的是有边界的元数据接口，不是通用 XCAF 编辑 API；XML 的详细边界见[协议规范](protocol.zh-CN.md)。
- `exportIGESDocument()` 和 `ShapeScope.importIGESDocument()` 通过 OCCT IGESCAF 桥接保留根层级、名称、RGBA 颜色、图层和实例变换。IGES 无法保留 `gdt`、`datums`、`geometricTolerances`、`views` 和 `shuo`，文档导出会明确拒绝这些字段。

### STL、OBJ、PLY 与 VRML

- `ShapeScope.importSTL(data)` 默认返回形状。传入 `{ includeMesh: true }` 时，还会返回索引几何、逐面片法线，以及 ASCII `solidName` 或精确的 80 字节二进制 `binaryHeader`。`exportSTL()` 接受相应元数据和 `encoding: "binary" | "ascii"`；形状导出还接受 `relative`。STL 不保留顶点共享、UV、材质或层级，导出的面片法线由三角形绕序决定。
- `ShapeScope.importOBJ(data)` 默认返回形状。传入 `{ includeDocument: true, resources }` 时，还会返回索引几何、逐角点法线和 UV、对象/组/材质/平滑范围、材质库引用和解析后的 MTL 数据。`exportOBJ(mesh, { materialLibrary })` 返回 OBJ 数据及 URI 到 `ArrayBuffer` 的配套资源；形状重载仍然可用。
- `importPLY()` 和 `exportPLY()` 支持 PLY 1.0 ASCII 及两种二进制字节序。形状路径保留顶点法线和常见 UV 属性；`{ includeMesh: true }` 还会返回索引几何、u8 RGBA 顶点颜色及有序的 `comments`/`objectInfo`。网格导出会拒绝无效索引和退化三角形。
- `importVRML()` 和 `exportVRML()` 支持 VRML 1/2。传入 `{ includeMesh: true }` 时返回索引几何、法线、可选 UV 和不透明的 u8 RGBA 颜色。VRML 只能存储 RGB，因此网格导出会拒绝非不透明 alpha。

### glTF 与 GLB

- `importGLTF()` 自动识别 glTF 2.0 JSON 和 GLB。导入会保留浮点或归一化整数形式的 `NORMAL` 和 `TEXCOORD_0`，并解析位置、法线、UV、索引和变形目标的稀疏访问器。
- 传入 `{ includeDocument: true, resources }` 时，导入会保留源文档，按完整 URI 解析缓冲区和图像，并返回活动场景的网格缓冲区及图元实例范围。合法的无三角形文档可以没有 `shape`。文档保持 glTF Y-up 坐标，返回的网格缓冲区和形状则使用平台 Z-up 坐标。
- `exportGLTF()` 默认返回 GLB。形状导出使用 `format: "gltf"` 时，返回带内嵌缓冲区的 glTF JSON；三角化导出包含位置、法线和 UV。网格文档重载接受导入所得的 `document`、已解析 `buffers` 和 `resources`；解析后的源缓冲区是权威输入，计算所得的 Z-up 数组不会重新编码。
- JSON 导出将外部缓冲区 URI 保留为配套资源，并重新生成数据 URI。GLB 导出把缓冲区合并进 BIN 块，外部图像仍作为配套资源返回。标准场景节点、实例、材质、纹理、动画、骨骼、相机、扩展和附加数据保留在源文档中。
- `animationIndex` 和 `animationTime` 必须同时提供。平移、旋转、缩放和权重通道支持 `STEP`、`LINEAR` 和 `CUBICSPLINE`；超出关键帧范围的时间会钳制到端点。求值前会校验引用、布局、数量、目标路径、插值模式和有限数值。
- 显式 `morphWeights` 必须匹配每个导入图元的目标数量，其优先级依次高于采样节点权重、静态节点权重和网格权重。变形访问器可以是稀疏的。
- 蒙皮通过 `JOINTS_0`、`WEIGHTS_0`、关节世界变换和可选逆绑定矩阵支持四影响混合。关节可使用无符号 8 位或 16 位整数，权重可使用浮点数或归一化的无符号 8 位/16 位整数；导入法线会随蒙皮几何一起变换。
- 采样不会修改返回的源 `document`。
- TypeScript 导入层使用固定版本的 Draco 和 meshoptimizer 运行时解码 `KHR_draco_mesh_compression` 与 `EXT_meshopt_compression`，再将几何传入内核。文档导入会保留原始压缩 JSON 和缓冲区，以便往返导出。

### BREP 与格式探测

- BREP 是绑定到当前 WASM 精确身份的缓存数据。应用必须同时保存该身份，并在调用 `importBREP()` 前拒绝不匹配的数据；standalone、isolated 和 shared 运行时的身份规则见[协议规范](protocol.zh-CN.md#交换与缓存)。
- `probeFormat()` 识别 BREP、STEP、IGES、STL、OBJ、PLY、glTF/GLB、VRML 和原生 XCAF 二进制/XML，报告文本或二进制编码，并指出 STEP 是否包含装配或显示元数据。

## 错误、取消与限制

- 错误是带稳定 `code` 的 `KernelError` 实例；`details` 是可选字段。
- 已解析操作派发的 `KernelFailure` 包含 `details.operation`。STEP、XCAF、IGES、STL、OBJ、PLY、glTF、VRML 和 BREP 失败还会在 `details.format` 中提供稳定的小写格式标识。
- 格式错误的协议帧和运行时失败可以省略 `details`。
- 底层 `request()` 接受数值超时，或 `{ timeoutMs, signal, onProgress }`。进度回调接收 `{ operation, fraction }`，其中 `fraction` 的范围为 0 到 1。
- 形状与边三角化、VRML、STL、OBJ、PLY、glTF，以及 STEP/IGES 形状或 XCAF 文档交换会把进度回调接入 OCCT `Message_ProgressIndicator`。
- 网格交换通过 `WorkerClient` 的共享标志观察 `AbortSignal`，并在解析、序列化和复制输出缓冲区时检查取消。OCCT 操作在下一个进度检查点停止，Worker 和已有句柄仍然有效。
- 取消排队中的请求只会移除该请求。取消其他正在执行的同步 Worker 操作会触发硬恢复：客户端终止并重建 Worker、拒绝旧队列，并使原有句柄失效。超时使用相同的恢复路径。

`DirectClient` 调用是同步的，不保证通过硬恢复处理超时。不受信任的 CAD 输入应使用 `WorkerClient`，使超时或非协作取消能够通过重建 Worker 隔离。
