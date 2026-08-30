# Protocol v1.2.0 / 协议 v1.2.0

[English reference / 英文规范](protocol.md)

BSpline 编辑操作 `editCurveBSpline` 和 `editSurfaceBSpline` 使用显式 action
值执行节点插入/删除、升阶、周期性转换、方向交换/反向和完整控制网替换。协议边界
中的索引从零开始；每个操作都返回新的作用域形状，输入保持不变。

协议 schema、操作目录、逐操作契约和错误列表是 `protocol/` 中的单一事实源。
`operation-contracts.json` 定义每个操作的参数和结果结构；如果它或
`historySupport` 遗漏任何操作，生成会失败；`protocol.schema.json` 包含生成的
请求/结果定义。请求是 `{ id, op, args }`，成功响应是
`{ id, ok: true, result }`，失败响应是
`{ id, ok: false, error: { code, message, details? } }`。ID 由客户端负责并单调递增。
`message` 面向阅读者，`code` 和文档化的 `details` 用于稳定分支。
协作式内核取消使用稳定的 `Cancelled`，而不是算法、三角化或导入导出失败码。
每个派发失败都包含 `details.operation`；交换操作还包含
`details.format`（`step`、`xcaf`、`iges`、`stl`、
`obj`、`ply`、`gltf`、`vrml` 或 `brep`）。
`probeFormat()` 通过 `BINFILE` 头识别原生 XCAF binary，通过 `XmlXCAF`
文档格式识别 XML。

## 请求与响应

WebAssembly 边界是同步且串行的。宿主只调用一次 `_initialize`，用 `k_alloc`
分配 UTF-8 请求字节，调用 `k_handle(ptr, len)`，从 `k_response_ptr()` 读取
响应，最后调用 `k_free(ptr)`。`k_handle` 返回零表示实例已失效。
公共消息 ABI 包含初始化、消息入口、分配/释放和 buffer 访问器；完整必需的 WebAssembly
导入导出面（包括运行时支持导出）冻结在 `wasm-surface.json` 中。

## 句柄、作用域和缓冲

形状句柄是只在一个内核实例内有效的 u32。TypeScript API 将其包装成带 epoch 检查的
`ShapeHandle`。公开的低层 `request()` 逃生口明确标记为不安全：原始 u32
值会绕过 TypeScript 的所有者、epoch 和作用域来源检查，猜测句柄时甚至可能访问另一个
活动作用域；但内核 generation 检查仍会拒绝过期句柄或来自其他实例的值。每个由作用域
创建的形状都由其 `scopeId` 所有；`endScope` 会释放该作用域的剩余形状，
`release` 会拒绝过期或未知句柄。

Buffer 使用 `{ bufferId, byteLength, layout }` 描述。普通 `DirectClient` 请求
会把输出复制到宿主内存，并立即释放内核描述符。可复用的同线程输入通过
`DirectClient.createBuffer()` 获得显式拥有的 `DirectBuffer`；每次 wasm 调用
后都必须重新调用 `view()`，通过低层协议传入 `{ bufferId }`，并使用
`DirectClient.freeBuffer()` 释放。`releaseAll()` 也会使所有活动 direct
buffer 失效。Worker 客户端不暴露内核 buffer ID：普通请求物化为可传输的
`ArrayBuffer`，`WorkerClient.requestShared()` 将每个输出描述符物化为
`SharedArrayBuffer`；共享输入跨 Worker 边界时不会传输或 detach。两种 Worker 模式
仍会在 wasm 线性内存与宿主内存之间复制一次。

## Batch

`batch` 按顺序执行操作，遇到第一个失败就停止。`{"$ref": 0}` 引用此前
操作的 `shape` 结果。Batch 不是事务：失败前创建的形状仍归其作用域所有。嵌套失败
包含 `error.details.operation: "batch"`、`nestedOperation`，交换操作还会
包含 `format`。

## 网格布局与矩阵

v1.2.0 三角化布局为 `f32x3` positions、`f32x3` normals、`u32`
三角形索引和 `u32x3` face groups。每个面使用隔离的顶点范围，消费者不能假设
不同面之间共享顶点。Face group 是 `(faceIndex, indexStart, indexCount)`；start
和 count 指向 `indices` 中的标量条目，因此一个三角形贡献三个条目。Edge
tessellation 使用 `(edgeIndex, vertexStart, vertexCount)`；start 和 count 指向
`positions` 中的 `f32x3` 顶点。面、边、顶点和 batch 索引都从零开始。
传入 `includeUV: true` 时还会返回与 positions 对齐的 `f32x2` `uvs`。

`matrix12` 是行主序
__[m11,m12,m13,m14,m21,m22,m23,m24,m31,m32,m33,m34]`，将点映射为
`(m11*x+m12*y+m13*z+m14, m21*x+m22*y+m23*z+m24, m31*x+m32*y+m33*z+m34)`。
对 `transform` 而言，`matrix` 优先，其他变换部分会被忽略。没有 matrix
时，合成矩阵是 `T*R*S*M`，点依次经历镜像、缩放、旋转和位移。
`massProps.inertia` 是返回中心处、全局模型坐标中的行主序
__[Ixx,Ixy,Ixz,Iyx,Iyy,Iyz,Izx,Izy,Izz]`。

## 能力、版本和默认值

所有公共协议默认值冻结在 `operation-contracts.json` 的
`semantics.defaults` 中，并生成到 `PROTOCOL_SEMANTICS` 和
`protocol.schema.json` 的 `x-protocolSemantics`。主要数值默认值为：
分类容差 `1e-7`，建模/修复容差 `1e-6`，三角化/STL 偏差为线性
`0.1`、角度 `0.5`，STEP 单位 `mm`，完整旋转角
`2*pi`。原点默认为 `(0,0,0)`，轴方向默认为 `(0,0,1)`，
history 和 UV 输出默认为 false，polygon 闭合和 transform copy 默认为 true，省略的
transform 部分默认为单位变换。

交换默认值也按操作冻结：STEP/IGES 使用 `mm`，STEP 时间戳使用
`2026-01-01T00:00:00`，STL 导出 binary，PLY 导出 ASCII，glTF 导出 GLB，
原生 XCAF 使用 binary persistence，网格/document 导入结果默认关闭。基于 shape 的
STL、OBJ、PLY、glTF 和 VRML 导出器共享线性/角度偏差及 relative 选项；基于 buffer
的重载按契约忽略三角化选项。

## 历史和 ShapeUpgrade

History 是单次操作的局部映射，不是持久拓扑命名；跨操作命名和特征树引用属于更高层
参数化 SDK。`historySupport` 明确列出目录中每个操作的级别：
三个 boolean 操作、`cylindricalHole`、`defeature`、`extrude`、
`revolve`、`loft`、`sweepPipe`、`fillet`、
`chamfer`、`sew`、`fixShape`、`unifySameDomain`、
`transform`、`generalTransform`、`translate`、
`rotate`、`scale` 和 `mirror` 为 `full`。
`sweepPipeShell` 为 `partial`：OCCT 映射脊线和 profile，但不提供
辅助脊线的拓扑历史。没有可靠 OCCT 历史来源的操作保持 `unsupported`，不会
被宣传成通用拓扑命名。

`shapeUpgrade` 包装 shape-level OCCT ShapeUpgrade 算法。模式为 `continuity`、
`angle`、`area`、`closedFaces`、`closedEdges`、
`convertToBezier`、`removeInternalWires` 和 `removeLocations`。
连续性条件为 `c0` 到 `c3` 或 `cn`；角度以弧度表示；area
division 只能选择 maximum-area、approximate-part-count 或 fixed U/V-count 语义之一；
internal-wire removal 使用投影轮廓面积；location removal 可从 shape、compound、solid、
shell 或 face 层级开始。分割和转换模式共享显式精度边界、surface-segment 模式及适用的
edge 选择（`0` free、`1` shared、`2` all）。ShapeUpgrade 会
改变拓扑，输入的面/边索引不会沿用，也不报告操作历史。

## 操作目录

v1.2.0 还包含多边形和曲线构造（`makeVertex`、`makePolygon`、
`makeWire`、`makeFace`、`makeFaceOnSurface`、
`makeShell`、`makeSolidFromShell`、`makeCompound`、
`makeCompSolid`、`makeEdgeLine`、`makeEdgeArc`、
`makeEdgeCircle`、`makeEdgeEllipse`、`makeEdgeHyperbola`、
`makeEdgeParabola`、`makeEdgeOffset`、`makeEdgeBezier`、
`makeEdgeBSpline`、`makeEdgeHelix`），点集 BSpline 近似
（`approximateCurveBSpline`、`approximateSurfaceBSpline`）、有界曲线/
曲面扩展（`extendCurve`、`extendSurface`）、曲线控制网编辑
（`curveControlData`、`updateCurvePole`）、有界和 filling 曲面构造
（`makeSurfaceBezier`、`makeSurfaceBSpline`、`makeSurfaceFace`、
`makeSurfaceExtrusion`、`makeSurfaceRevolution`、
`makeSurfaceRuled`、`makeSurfaceOffset`、`makeSurfaceFilling`）、
曲面控制网编辑和转换（`surfaceControlData`、`updateSurfacePole`、
`trimSurface`、`convertSurfaceToBSpline`）、基元、建模、查询、网格、
交换、STEP/IGES XCAF 文档、原生 XCAF binary/XML 以及 `probeFormat`。
曲线控制数组使用从零开始的索引；编辑返回保持输入参数域的新 edge。近似使用 OCCT
弦长、centripetal 或 uniform 参数化以及请求的 degree/continuity/tolerance 范围。
`makeSurfaceFilling` 使用 wire 顺序的一个闭合边界 wire，可选内部 3D 点约束。
`makeFaceOnSurface` 会投影缺失 pcurve，并接受任意 outer/hole wire。
`selectGeneralFuseCells` 使用从零开始的 take/avoid 输入索引规则，可合并相同的
非零 material 或创建按维度分类的容器。`draftAngle` 对从零开始的输入面索引
应用一个方向、角度和 neutral plane。`localPrism` 对属于 base shape 的面执行
长度模式的 `BRepFeat_MakePrism` add/cut。曲面扩展增大矩形外边界并保留 hole
wire；曲面控制网使用从零开始的 U-major `poles[u][v]` 和 `weights[u][v]`。
`transform.copy` 默认 true，控制 OCCT 是否复制变换后的拓扑。
`batchTransformCopy` 创建不包含输入本身的 count 个变换副本；线性副本使用每副本
translation，圆形副本围绕 origin/direction 将 total angle 等分。`sweepPipeShell`
还支持 `fixedAxis` 和可选的 `auxiliarySpine`/`curvilinearEquivalence`。
`capabilities` 是某个构建中精确可用操作集的最终依据。

## 查询、截面和 HLR

`distance` 返回所有 OCCT 最小距离解、`innerSolution` 以及两个输入各自
的 vertex/edge/face 支持分类。支持记录在对应输入中存在时使用稳定的从零开始拓扑索引，
并在适用时包含 edge 参数或 face UV 参数。`middlePath` 提取一个 pipe-like shape
两个 face 或 wire 截面之间的 OCCT 中心路径。`sectionAnalysis` 返回带索引的
截面边、长度、两个输入的 ancestor face、孤立截面顶点、可选交线近似和可选输入 pcurve。

`projectHLR` 使用精确的 `TKHLR` B-Rep 隐藏线算法，返回带作用域的
`visible` 和 `hidden` edge compound。`direction` 从相机指向
模型，`up` 固定视图方向。默认是平行投影；透视投影要求从 Z=0 投影平面到
眼睛的正 `focus` 距离。结果位于投影坐标中，每条边的 Z 都为零。

`getSubShape` 将从零开始索引的子形状物化到调用方作用域，使导入或生成的实体
能够暴露单独的边和面供解析运算。曲线求值使用原生 OCCT edge 参数域，并返回点、一阶/
二阶导数、切线可用性、切线和曲率；曲面求值使用受限 face UV 域，并返回点/导数、拓扑
定向法线以及有定义时的有符号主/平均/高斯曲率。反转面会反转法线和有符号主/平均曲率，
高斯曲率不变。STL 导入导出都走内存流；导入的 STL 变成三角化 TopoDS compound，不会
恢复解析 CAD 曲面。

## 曲线、曲面和网格操作

`makeEdgeArc` 接受三点或圆定义（`center`、`normal`、
`radius`、`startAngle`、`endAngle` 以及可选 `xDirection`）。
双曲线和抛物线必须提供有限参数界限；`makeEdgeOffset` 保留基 edge 域，并使用
OCCT 的 tangent/reference-direction cross-product 约定。`makeEdgeBSpline` 接受
JSON 点或 packed `f64x3` buffer：默认插值，`mode: "controlPoints"` 接受
degree、knots、multiplicities、weights 和 periodicity。派生曲面构造使用有限输入 edge/
face 拓扑域，surface offset 保留 outer/hole wire。Bezier/BSpline 曲面使用 U-major
控制网构造有限 face；`makeSurfaceFilling` 默认 C0 边界，可选 G1/G2 支持面和
内部点约束；`makeFaceOnSurface` 不修改输入。`surfaceIsoCurve(face, "u" | "v",
parameter)` 返回按 outer boundary 和 hole 裁剪的 scoped edge。未定义的切线、法线和
曲率通过对应 boolean 字段报告，缺失值不会输出。

`inspectTolerances()` 返回稳定的从零开始 face、edge、vertex 容差条目。
`ShapeScope.setTolerance()` 在深复制上编辑一个拓扑层级或全部层级，不修改输入。
投影、极值和交运算针对有限拓扑而非无界基准几何，遵循 outer/hole 边界。极值在驻点
解连续时设置 `parallel`，此时返回代表性的全局最近点对；
`intersectCurveCurve()` 和 `intersectSurfaceSurface()` 为孤立或一维结果
创建 scoped vertex/edge 句柄。

`weldTessellation()` 在位置容差内合并顶点，默认保留硬法线和 UV seam，删除合并
后塌缩的三角形并重写 `faceGroups`。`recalculateVertexNormals()` 计算
面积加权法线，`generateTangents()` 为含 UV 的三角化返回规范化 `f32x4`
切线和 handedness。`findBoundaryLoops()` 返回确定性排序且方向一致的索引边界环。
`fillPlanarHoles()` 对选中的简单平面环执行 ear clipping，追加调用方选择的 face
group，保留 positions/UV 并重建法线；它明确拒绝非流形、分支、方向不一致、非平面、退化
或自交边界，不会推断外边界和孔，存在两者时调用方应传入 `loopIndices`。

STL、OBJ、PLY、VRML 和 glTF 的 mesh/document 导入导出保留各格式文档化的 metadata。
STL 可返回 facet normals、ASCII solid name 和精确 binary header；OBJ 可返回 indexed
positions/indices、独立 corner UV/normal、object/group/material/smoothing 状态和 MTL
资源；PLY 可返回 positions、indices、normals、UV、u8 RGBA vertex colors 以及有序
`comments`/`objectInfo`；VRML 可返回 positions、indices、normals、UV 和
不透明 u8 RGBA colors。索引 buffer 的 primitive range 使用标量 index 条目。
`exportPLY` 接受 `ascii`、`binary_little_endian` 和
`binary_big_endian`；所有输出都可由对应的 `importPLY` 读取。
STL、PLY、OBJ 和 glTF 的 mesh 导出会拒绝越界、重复索引、非有限属性和几何退化三角形。

glTF 2.0 JSON/GLB 支持 data URI 或 URI 匹配的 companion buffer、list/strip/fan 三角形、
scene 节点矩阵或 TRS、float/normalized integer NORMAL/TEXCOORD_0、sparse accessor、
POSITION/NORMAL morph、四影响 skin、选定动画采样以及 OCCT Z-up 与 glTF Y-up 转换。
`animationIndex` 和 `animationTime` 必须同时省略或同时提供；动画在 active
scene 世界变换、morph 和 skinning 前采样，时间超出关键帧范围时钳制到端点。节点的
translation、rotation、scale、weights 支持 STEP/LINEAR/CUBICSPLINE；线性旋转使用归一化
最短路径四元数插值。显式 `morphWeights` 覆盖采样的 node weights，采样值覆盖
static node weights，后者覆盖 mesh weights，缺失权重按零处理。skinning 使用
`JOINTS_0`、`WEIGHTS_0`、joint world transform 和可选 inverse bind matrix。
`includeDocument: true` 时，导入保留未修改的源 JSON、资源和 primitive ranges；
有效的无三角形文档可以没有 shape。JSON 导出写 companion resources，GLB 将 buffer 合入
BIN chunk，外部图像仍作为资源返回；materials、textures、animations、skins、cameras、
extensions 和 extras 保持在标准 JSON 中。未知必需的几何扩展会被拒绝。

## 参数化 CAD

`ParametricModel` 在 `BaseClient` 之上提供可序列化特征层。表达式支持命名
参数和不使用 `eval` 的算术；特征依赖按拓扑排序，循环会被拒绝。
`recompute()` 在新作用域中构建并原子发布结果，并串行化并发重算和 disposal。
序列化输出带 `schemaVersion: 1`；`fromJSON()` 会将原始无版本表示迁移
到 v1，并拒绝未知的未来版本。特征可以在定义中或通过 `setFeatureSuppressed()`
标记 `suppressed`；`getFeatureDiagnostics()` 报告上次重算的 `ok`、
`suppressed` 或 `failed`。

可选的 `subshapeReferences` 可以命名 feature face 或 edge。`initialIndex`
只在第一次成功重算时捕获持久签名，之后按 geometry family 加 parent-normalized bounds
唯一匹配。`source: "id"` 会沿唯一特征历史路径跟随之前的引用。boolean、
cylindrical hole、defeature、extrude、revolve、loft、pipe/pipe-shell sweep、fillet、
chamfer、sew、shape repair、same-domain unification、translate、rotate、scale、mirror
和 general affine transform 会自动贡献可用的 OCCT history，包括非零 multi-input 和多阶段
链。辅助 pipe-shell spine 明确标记为 unmapped；deleted/unmapped source 变为 `missing`，
一对多映射变为 `ambiguous`。对有意且稳定的对称候选可设置
`disambiguation: "initialIndex"`；默认仍拒绝歧义。若操作有意替换 curve/surface
family 并保留唯一 normalized boundary，可显式设置 `allowGeometryReplacement: true`；
否则 family 变化保持 missing。fillet、chamfer、hollow、draft、local prism/revolution
和 defeature 的索引字段还接受 `{ reference: "id" }`。缺失或歧义引用会使下游
特征失败，不发布部分重算结果。`getSubshapeReference()` 返回当前 scoped handle，
`getSubshapeReferenceDiagnostics()` 报告 resolved/missing/ambiguous 及候选索引；
`resolvePersistentSubshape(source, historyChain)` 保留供显式历史组合使用。

可序列化特征覆盖基元、polygon/face、extrusion、revolution、loft、pipe/pipe-shell、
fillet、chamfer、hollow、offset、draft、local prism、cylindrical holes、defeature、
translation、rotation、scale、mirror、general affine transform 和 boolean cut/fuse/common。
可选的 `document` 定义带索引的 part/assembly 节点；重复 part 节点可作为 component
instance，带参数驱动变换、product name、RGBA colors、layers、visibility、physical/PBR
materials 和 validation properties。重算后 `toSTEPDocumentDefinition()` 将 feature
ID 和 transform expression 解析成 `exportSTEPDocument()` 与 `exportXCAF()`
接受的文档定义。`solveSketch()` 支持二维 line/circle/arc/BSpline、construction
geometry、几何和尺寸约束、收敛/残差/迭代/自由度诊断及任意 3D sketch plane；BSpline
端点参与点引用约束，但 BSpline tangency 不属于有界 v1 solver。无 `profiles` 时
sketch 输出一个 wire；有序 profiles 输出 compound 和稳定的
`sketchId.profileId` wire 引用，可直接作为 `face.outer` 或
`face.holes`。特征图还支持 sewing、shape repair、same-domain unification、section
和 split。

## 交换、缓存和错误

STEP 与 IGES 通过内存流工作并接受 `mm`、`cm`、`m`、
`inch`、`foot` 单位。STEP shape/document 支持
`AP203`、`AP214` 和 `AP242`，默认 `AP242`；STEP
时间戳使用调用方提供的字符串或固定的 `2026-01-01T00:00:00`。多根 shape-only
导出接受一个 shape 或独立 shape 数组；导入返回组合的 `shape`、每个独立结果的
`shapes` 和 `rootCount`。XCAF 文档支持根树、实例变换、名称、RGBA
颜色、层、可见性、physical/PBR material、验证属性、subshape styles、views、SHUO、datum
和 GDT 的文档化子集。STEP 无法保留的链接会拒绝；带 GDT 的 STEP 文档必须只有一个根。
原生 XCAF binary 保留完整的公共字段；worker 生成的 XML 带经验证的 binary recovery marker，
标准 XML 无法无损保留 node color、非空 subshapeStyles 和带 color 的 SHUO，导出会拒绝，
这些组合应直接使用 binary。`probeFormat()` 识别 BREP、STEP、IGES、STL、OBJ、PLY、
glTF/GLB、VRML 和 XCAF binary/XML，并报告文本/二进制编码及 STEP 文档元数据。

BREP 是绑定精确 wasm SHA-256 的缓存格式，不是跨版本交换格式。内核只输出裸 BREP 字节，
不会嵌入或验证哈希；调用方必须将 `docs/g0-build.json` 中的哈希与缓存一起保存，
在调用 `importBREP` 前拒绝不匹配项。

`fillet()` 和 `chamfer()` 使用输入 shape 稳定 edge map 的从零开始
`edgeIndices`。两者保留标量 size 兼容参数，并接受对齐的 `radii` 或
`distances` 数组；fillet 的 `radius2` 是轮廓起点到终点的线性演化，
`radiusLaw` 或 `radiusLaws` 使用从 0 到 1 严格递增的多点 OCCT law。
Chamfer 的双距离模式使用 `distance2` 或 `distances2`，并要求对齐的
`referenceFaceIndices`；每个参考面必须邻接对应边并指定第一距离所在侧。
`history` 始终是单次操作的局部映射；没有可靠 history 的操作在 capabilities
中标记 unsupported，请求其 history 会得到 `InvalidArgs`。

`cylindricalHole` 使用显式 axis，支持 `throughAll`、`throughNext`、
`untilEnd`、`blind` 和 `between` limit。blind 的 length 和
between 的 from/to 都是从 origin 沿轴测量的模型单位。`defeature` 接受 solid、
compsolid 或 solid compound 上从零开始的 face 索引；两者都能提供完整的局部 face/edge history。

曲线/曲面编辑使用原生节点、升阶、周期性和控制网操作；degree reduction 使用有界 OCCT
BSpline 近似，返回新 shape 与 `maxError`，保留源参数域和曲面边界拓扑。
`triangulationData` 不重新三角化，`validateTriangulation` 检查 finite
attribute、索引和退化约束，`replaceTriangulation` 返回独立 face copy，
`repairTriangulation` 使用 OCCT BRepMesh 复制、重网格并计算法线。
`makeSurfaceFilling.constraints` 使用 wire 顺序的从零开始 boundary edge index 和
face handle，连续性为 `g1` 或 `g2`；未指定边使用 C0，support face
必须有该 edge 的 OCCT pcurve。`localPrism` 默认 length 模式，其他 limit 映射
到 `BRepFeat_MakePrism`；`localRevolution` 映射到
`BRepFeat_MakeRevol`，`linearForm` 和 `revolutionForm` 分别
构造平面和旋转 rib/slot，`glue` 使用 `BRepFeat_Gluer` 执行显式 imprint。

压缩 glTF 在 TypeScript 导入边界用固定的 Draco 和 meshoptimizer runtime 解码后再进入
内核；文档导入仍保留原始压缩 JSON 和 source buffers。VRML、STL、OBJ、PLY、glTF 导入/
导出以及 edge tessellation 报告归一化进度；只有宿主适配器列出的协作取消操作会在派发后
观察 `AbortSignal`，网格交换会在解析、序列化和 buffer 物化阶段确认取消。
