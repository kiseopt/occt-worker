# TypeScript API 参考

[English reference / 英文完整规范](api.md)

## API 速览

| 使用场景 | 公开入口 | 参考 |
| --- | --- | --- |
| Node.js 进程内核 | DirectClient 和 ShapeScope | 本 API 参考 |
| Node.js 或浏览器 Worker 内核 | WorkerClient | [使用指南](getting-started.zh-CN.md)和本 API 参考 |
| 建模、查询、网格和交换操作 | ShapeScope 方法 | [能力矩阵](capabilities.zh-CN.md) |
| 底层消息、缓冲区、错误和默认值 | request() 与协议帧 | [协议规范](protocol.zh-CN.md) |
| Wasmtime 或自定义宿主集成 | 同步 WebAssembly ABI | [宿主支持](hosts.zh-CN.md) |
| 参数化特征和草图 | ParametricModel 与特征定义 | 本 API 参考 |
| shared Main/Side 高层兼容 | SharedClient + EngineCompatClient | 本 API 参考 |

## 客户端与生命周期

DirectClient.create(wasm, imports?) 创建当前线程内核；WorkerClient.create(factory, wasm)
创建带硬超时恢复的可释放 Worker 内核。两者都按生成的协议版本初始化，并提供查询、
三角化和导出方法。DirectClient.createBuffer() 暴露同线程可复用输入 buffer；每次 wasm
请求后都要重新调用 view()，通过低层协议传入 { bufferId }，并使用
DirectClient.freeBuffer() 释放。releaseAll() 也会使所有活动 direct buffer 失效。低层
request() 是明确不安全的协议逃生口：它接受原始 u32 句柄和 buffer 引用，绕过
TypeScript 所有者和 epoch 来源检查，猜测句柄时可能访问另一个活动 scope，但不会绕过
内核 generation 检查，也不会让句柄在另一个 wasm 实例中有效。普通应用应使用类型化方法。

beginScope() 返回拥有其创建的每个形状的 ShapeScope。应显式结束 scope，或使用
await using。形状句柄带客户端 epoch，不能由原始数字构造，也不能发送到另一个或重建
后的 kernel 实例。scope.batch() 是类型化 batch 入口，会把每个顶层 shape 或 shapes
结果转换为带来源检查的 ShapeHandle。

### 自定义 isolated 内核

替换 isolated kernel 时，将自定义 artifact 传给 createWorkerProfileRuntime，并通过
resolve 或 baseUrl 解析到实际 URL。该路径仍会启动 Worker 并执行 capabilities 握手，
不兼容的协议会拒绝加载。若 descriptor 提供 sha256、protocolVersion、abiVersion 或
buildFamily，客户端会分别与字节或本次构建核对；这些字段是调用方断言，任意 wasm
不能自行证明 buildFamily。

C3 生成的 profile 客户端类型只适用于 protocol/modules.json 声明的官方 profile。自定义
artifact 没有对应的静态能力收窄；加载后应读取 runtime.request("capabilities", {})
返回的 ops，再选择操作。

### 交换选项类型

交换选项导出为命名 TypeScript contract：STLExportOptions/STLImportOptions、
IGESExportOptions/IGESImportOptions、STEPExportOptions/STEPImportOptions、
GLTFExportOptions/GLTFImportOptions、OBJExportOptions/OBJImportOptions、
PLYExportOptions/PLYImportOptions 和 VRMLExportOptions/VRMLImportOptions。基于 shape
的三角化导出共享 TessellatedShapeExportOptions（linearDeflection、angularDeflection、
relative）；indexed mesh 重载接受 positions/indices（STL 还接受 normals，VRML 还接受
normals/UV）并忽略三角化选项。mesh-document 重载只接受不需要三角化的选项。
LengthUnit 是 STEP/IGES 通用单位联合类型。省略 format 时，类型化和低层协议入口的
原生 XCAF 默认都使用 binary。

## 构造、建模与局部特征

构造方法包含基元、vertex、polygon/wire/face/shell/solid/compound/compsolid、有限
plane/cylinder/cone/sphere/torus face、已有曲面上的 wire-trimmed face、带 C0 边界和
可选 G1/G2 support-face 约束的 N-side filling、有限 line/arc/ellipse/hyperbola/
parabola/offset/Bezier/BSpline/helix edge、有限 extrusion/revolution/ruled/offset
surface、点集 BSpline 曲线/曲面近似、有界曲线/曲面扩展、extrusion、revolution、loft、
pipe sweep、boolean、带 source mapping 和 take/avoid/material 选择的 General Fuse、
multi-face draft、local prism、revolution-form rib/slot、cylindrical hole、defeature、
offset、repair 和 transform。dist/client.d.ts 中的方法选项类型是规范 TypeScript 签名；
操作名称和 history 级别来自 protocol/operations.json。

selectGeneralFuseCells() 会从输入形状重新运行 General Fuse，然后加入符合每条规则从
零开始 take 和 avoid 输入集合的 cell。非零且相同的 material 可以在
removeInternalBoundaries 下合并；makeContainers 按 cell 维度创建 wire、shell 或
compsolid。draftAngle() 对从零开始的输入面索引应用一个方向、角度和 neutral plane。
localPrism() 使用已经属于 base shape 的选中面，执行非破坏的 BRepFeat_MakePrism add/cut。
现有数值参数选择 length mode；options 参数选择 until、fromUntil、untilEnd、fromEnd
或 thruAll，并携带所需的 until/from limiting shape。localRevolution() 是对应的
BRepFeat_MakeRevol，支持 axis、angle、until/from limit 或 thru-all。
linearForm() 暴露原生 BRepFeat_MakeLinearForm 平面 rib/slot：输入 open/closed profile
wire、所在 plane、一个 thickness direction 和可选相反方向。operation 为 add 创建 rib，
为 cut 创建 slot；第二方向必须为零或与第一方向相反。

cylindricalHole() 沿显式 axis 切削，支持 through-all、through-next、until-end、
blind-length 和 axis-interval limit。blind 的 length 及 between 的 from/to 都是从
origin 沿轴测量的模型单位。defeature() 从 solid、compsolid 或 solid compound
移除选定的从零开始面并重建周围几何；启用 includeHistory 时两者都返回局部 face/edge
history。

revolutionForm() 暴露原生 BRepFeat_MakeRevolutionForm 旋转 rib/slot：在 plane 上接受
open/closed profile wire、revolution axis 和非负的 height1/height2（不能同时为零）。
add 创建 rib，cut 创建 slot；该特征没有 limit-shape mode。glue() 暴露
BRepFeat_Gluer 的显式局部 imprint：输入 new shape、basis shape 以及至少一个从零开始
的重合 face binding；可直接提供 edge binding，绑定面中的边也会由 OCCT 原生 edge
finder 匹配。结果是新 glued shape，输入不修改。

## 曲线与曲面

ShapeScope.getSubShape(shape, type, index) 将一个从零开始的子形状物化到调用方 scope。
curveDomain、curveGeometry、curveContinuity 和 evaluateCurve 暴露 edge 的原生参数范围、
解析类型和参数、全局连续性、导数、切线和曲率。makeEdgeHyperbola 和 makeEdgeParabola
要求有限递增参数界限；makeEdgeOffset 保留基 edge 域并使用 OCCT tangent/reference-
direction cross-product 约定。curveControlData 返回完整 Bezier/BSpline poles、weights、
knots、multiplicities 和 periodicity；updateCurvePole 复制并编辑一个从零开始的 pole，
同时保持输入 edge 参数域和拓扑方向。trimCurve 创建子范围 edge，convertCurveToBSpline
转换有限 edge，而不是可能无界的基准曲线。

surfaceDomain、surfaceGeometry、surfaceContinuity 和 evaluateSurface 暴露 face 的受限
UV 范围、解析类型和参数、U/V 连续性、导数、定向法线及局部主/平均/高斯曲率。
makeSurfaceExtrusion、makeSurfaceRevolution 和 makeSurfaceRuled 从 edge 域构造有限 face；
makeSurfaceOffset 保留输入 face 的 outer/hole wire 并替换 support surface。
makeSurfaceBezier 和 makeSurfaceBSpline 使用 U-major poles[u][v] 控制网构造有限 face，
可选 rational weights、knot vectors、multiplicities 和 periodicity。
makeSurfaceFilling 从闭合 wire 构造 N-side filling，默认 C0 边界，可选 G1/G2 support-
face 和内部点约束。makeFaceOnSurface 复制任意 outer/hole wire，为缺失 pcurve 进行投影，
并裁剪现有 support surface，不修改输入。surfaceControlData 读取完整控制网，
updateSurfacePole 复制编辑单个 pole，trimSurface 创建有限矩形 UV patch，
convertSurfaceToBSpline 转换有限 analytic/Bezier/BSpline face。
surfaceIsoCurve(face, "u" | "v", parameter) 创建按 face outer boundary 和 holes 裁剪的
scoped edge；"u" 表示固定 U，"v" 表示固定 V。未定义切线、法线和曲率通过对应
boolean 字段报告，并省略未定义值。

BSpline 编辑操作使用原生节点插入/删除、升阶、周期性转换、方向交换/反向和完整控制网
替换，并返回新 edge/face。reparameterizeCurve 和 reparameterizeSurface 执行精确仿射
knot-domain 变化；曲面边界 pcurve 按新参数域重建。reduceCurveDegree 和
reduceSurfaceDegree 执行有界 OCCT BSpline 近似到请求的较低 degree，保留输入域/拓扑并
返回 { shape, maxError }。approximateCurveBSpline 和 approximateSurfaceBSpline 可以用
variationalSmoothing 的 length/curvature/torsion weights 执行 OCCT variational fairing；
该模式使用 degreeMax、continuity 和 tolerance，不使用 parameterization 或 degreeMin。

## 查询与诊断

ShapeScope.middlePath() 提取 pipe-like shape 两个属于该 shape 的 face 或 wire subshape
之间的 OCCT 中心路径。inspectTolerances() 返回稳定的从零开始 face、edge、vertex 容差
条目；ShapeScope.setTolerance() 在深复制上编辑一个拓扑层级或全部层级，不修改输入。

投影、极值和交运算针对有限拓扑而非无界基准几何，并遵循 face holes 和 outer boundary。
极值在驻点解连续时设置 parallel，此时返回代表性的全局最近点对。distance() 返回所有
OCCT 最小距离解、输入之间的 inner-solid 状态和每个解的 vertex/edge/face support。
支持索引使用与其他 API 相同的稳定从零开始拓扑 map；适用时返回 edge 参数和 face UV
参数。若 inner solution 的 support 不是相应输入的子形状，该侧可以省略索引。
intersectCurveCurve() 和 intersectSurfaceSurface() 为孤立或一维结果创建 scoped handle。

ShapeScope.sectionAnalysis() 返回 scoped section shape、每条 section edge 的稳定索引和
长度、两个输入上的可选 ancestor-face 索引，以及孤立 section vertex 的稳定索引；可
请求 OCCT 曲线近似和任一输入上的 pcurve，而不改变 section() 行为。
ShapeScope.projectHLR() 运行不经过三角化的精确 B-Rep hidden-line removal，接受
camera-to-model direction、up 和平行/透视投影；透视还要求正的 focus 距离。它在 view
coordinates 返回可复用的 scoped visible 和 hidden edge compound，X 向右、Y 沿投影后的
up、Z 为零。

## 网格处理

weldTessellation() 在位置容差内合并顶点，默认保留硬法线和 UV seam，删除 weld 后塌缩的
三角形并重写 faceGroups。recalculateVertexNormals() 计算面积加权法线；
generateTangents() 为包含 UV 的三角化返回规范化 f32x4 tangent 和 handedness。
findBoundaryLoops() 返回确定性排序且方向一致的 indexed boundary loop。
fillPlanarHoles() 对选定的简单平面环执行 ear clipping，追加调用方选择的 face group，
保留 positions/UV 并重建法线；明确拒绝非流形、分支、方向不一致、非平面、退化和自交
边界。它不会推断外边界和孔，开放表面同时有两者时调用方应传入 loopIndices。

FaceSelectionMap 将渲染 triangle index 解析为从零开始的 OCCT face index，并提供每个 face
的反向 triangle range。EdgeSelectionMap 提供 edge-polyline vertex 与 OCCT edge index 的
对应映射。两者校验 group range 单位、边界和重叠；内置 weld、simplify 和 subdivision
会重建带输出 index 的 faceGroups，因此映射仍可使用。
simplifyTessellation() 执行确定性的 vertex-cluster 简化，可设置 targetTriangles 或 ratio，
删除塌缩三角形、保留 face-group range 并重建面积加权法线。
subdivideTessellation() 执行一至多级 midpoint triangle 细分，插值 UV、保留 face-group
range 并重建法线。

## 交换格式

所有交换均通过内存流。STEP 和 IGES 接受 mm、cm、m、inch、foot 单位；STEP shape/document
schema 为 AP203、AP214 或 AP242，默认 AP242。STEP 时间戳使用调用方提供的 timestamp
或固定的 2026-01-01T00:00:00，不读取时钟。exportSTEP/exportIGES 接受一个 shape 或
独立 shape 数组；import 返回组合的 shape、每个独立根的 scoped shapes 和 rootCount。

exportSTEPDocument 接受带 indexed part/assembly 节点的根树；子节点有局部 matrix12，节点
可带名称、RGBA 颜色、层、可见性和 XCAF physical material。material density 必须非负，
省略元数据时使用 kg/m^3 和 mass density。原生 exportXCAF/importXCAF 在 binary 及
worker 生成 XML 中保留 geometric tolerances、datum links、views、SHUO、subshape styles
和公共 PBR 字段。XML 是标准 XmlXCAF 并带经验证的 binary recovery marker；内存导入只
接受 exportXCAF() 生成的 XML，第三方 XmlXCAF 必须先转为 XCAF binary。XML 导出会拒绝
node.color、非空 subshapeStyles 和带 color 的 SHUO，以免标准 XML 部分静默丢失 RGBA；
需要这些字段时使用 XCAF binary。STEP 无法保留的 datums 或 geometricTolerances 链接会
拒绝，带 GDT 的 STEP 必须只有一个文档根。原生 XCAF views 保留带裁剪平面标签的 named
camera；dimensions、datums 和 geometric tolerances 可以带 presentation: { shape, name? }，
导入时物化为 scoped ShapeHandle。

exportIGESDocument 和 importIGESDocument 通过受验证的尾部 metadata marker 保留根
part/assembly 层级、名称、RGBA、层和 occurrence transform。IGES 不能保留 gdt、datums、
geometricTolerances、views 或 shuo，文档导出会明确拒绝。shape-only IGES 额外支持
mode: "faces"（默认）或 "brep"，并返回独立 roots。

ShapeScope.importSTL(data) 默认返回 shape-only；includeMesh: true 时还返回 indexed
positions、indices、每 facet 复制的 normals 以及 ASCII solidName 或精确 binary 80-byte
binaryHeader。exportSTL() 对 shape 和 indexed mesh 都接受相应 metadata，支持
encoding: "binary" | "ascii"（默认 binary）；shape export 还支持 relative: false。
STL 不保留 vertex sharing、UV、material 或 hierarchy，facet normals 来自三角形绕序。

ShapeScope.importOBJ(data) 默认返回 shape-only；includeDocument: true 和 resources 时
还返回 indexed positions/indices、可选 normals/UV、连续 object/group/material/smoothing
ranges、material library 引用和解析后的 companion MTL material。OBJ 支持正负 position/
UV/normal 索引、seam expansion、slash-separated corner 和 polygon fan triangulation。
exportOBJ(mesh, { materialLibrary }) 对称写回 indexed buffer 和文档 metadata，返回 OBJ
data 及 URI-to-ArrayBuffer companion resources；shape-only 重载仍可用。

PLY 1.0 支持 ASCII、binary little-endian 和 binary big-endian。shape 路径保留 vertex
normal 与 s/t、u/v 或 texture_u/texture_v UV property；includeMesh: true 还返回 indexed
positions、indices、normals、UV、u8 RGBA vertex color 和有序 comments/objectInfo header。
exportPLY() 可直接写回这些数据，并拒绝越界、重复索引和几何退化三角形。

glTF 2.0 JSON/GLB 支持 embedded 或 URI 匹配的 companion buffer、triangle list/strip/fan、
scene 节点 matrix/TRS、float 或 normalized integer NORMAL/TEXCOORD_0、sparse accessor、
POSITION/NORMAL morph、四影响 skin、selected-animation sampling 以及 OCCT Z-up/glTF Y-up
转换。animationIndex 和 animationTime 必须同时出现或同时省略；时间在 active-scene
world transform、morph 和 skinning 前采样，范围外钳制到端点。节点 translation/rotation/
scale/weights 支持 STEP、LINEAR、CUBICSPLINE；线性旋转使用归一化最短路径四元数插值。
显式 morphWeights 覆盖 sampled node weights，后者覆盖 static node weights，再覆盖 mesh
weights，缺失权重为零。skinning 使用 JOINTS_0、WEIGHTS_0、joint world transforms 和
可选 inverse bind matrices。includeDocument: true 时保留完整源 JSON、解析后的 buffers/
images 和 primitive-instance ranges，合法无三角形文档可以没有 shape。JSON 导出把外部
buffer/image 写成 companion resources，GLB 将 buffer 合并到 BIN chunk；materials、
textures、samplers、animations、skins、cameras、extensions 和 extras 保留在源 JSON。
未知必需几何扩展会拒绝。

VRML 1/2 通过内存流导入导出，默认保持 shape-only API；includeMesh: true 时返回 indexed
positions、triangle indices、normals、可选 UV 和 opaque u8 RGBA colors。mesh 导出接受
相同属性，但 VRML 只存 RGB，RGBA 输入必须不透明。probeFormat() 识别 BREP、STEP、IGES、
STL、OBJ、PLY、glTF/GLB、VRML、原生 XCAF binary/XML，并报告编码及 STEP 文档元数据。

BREP 是绑定精确 wasm artifact SHA-256 的缓存格式，不是跨版本交换格式。内核输出裸 BREP
字节，不嵌入或验证哈希；调用方必须随缓存保存 docs/g0-build.json 中的哈希，在调用
importBREP() 前拒绝不匹配项。

## 参数化 CAD

ParametricModel 在任意 BaseClient 之上提供可序列化特征层。表达式支持命名参数和不使用
eval 的算术；特征依赖按拓扑排序，循环会拒绝。recompute() 在新 scope 中构建并原子
发布结果，并串行化并发 recompute 和 disposal。序列化输出带 schemaVersion: 1；
fromJSON() 将原始无版本表示迁移到 v1，并拒绝未知未来版本。特征可在定义中或通过
setFeatureSuppressed() 标记 suppressed；getFeatureDiagnostics() 报告上次重算的 ok、
suppressed 或 failed。

可选 subshapeReferences 命名 feature face/edge。initialIndex 只在第一次成功 recompute
时捕获持久签名，后续按 geometry family 和 parent-normalized subshape bounds 唯一匹配。
带 source: "id" 的引用沿唯一 feature-history path 跟随目标。boolean cut/fuse/common、
cylindrical holes、defeaturing、extrusion、revolution、loft、pipe/pipe-shell sweep、
fillet、chamfer、sewing、shape repair、same-domain unification、translation、rotation、
scale、mirror 和 general affine transform 会自动组合可用 OCCT history，包括非零
multi-input 和多阶段链。pipe-shell auxiliary spine 明确 unmapped；deleted/unmapped source
变为 missing，一对多映射变为 ambiguous。对称且有意依赖原始拓扑顺序时可设置
disambiguation: "initialIndex"；默认仍严格拒绝歧义。有意替换 curve/surface family 且
保留唯一 normalized boundary 时可设置 allowGeometryReplacement: true；否则 family 变化
保持 missing。fillet/chamfer/hollow/draft/local prism/revolution/defeature 的索引字段
接受 { reference: "id" }，引用必须属于 feature input 且类型正确。缺失或歧义会使下游
feature 失败，不发布部分 recompute。getSubshapeReference() 返回当前 scoped handle，
getSubshapeReferenceDiagnostics() 返回 resolved/missing/ambiguous 和候选索引；
resolvePersistentSubshape(source, historyChain) 可供显式历史组合使用。该签名规则只保证
尺寸和均匀拓扑变化中的稳定归属，不是没有 OCCT history 时的通用 B-Rep 命名算法。

Feature 定义覆盖 box、cylinder、sphere、cone、torus、wedge、polygon/face profile、
extrusion、revolution、loft、pipe/pipe-shell、fillet、chamfer、hollow、offset、draft、
local prism、cylindrical holes、defeaturing、translation、rotation、scale、mirror、general
affine transform 和 boolean cut/fuse/common。可选 document 定义 indexed part/assembly
nodes；重复 part node 可作为 component instance，带参数驱动 transform、product name、
RGBA color、layer、visibility、physical material、PBR visual material 和 validation
property。recompute() 后，toSTEPDocumentDefinition() 将 feature ID 和 transform expression
解析为 exportSTEPDocument() 和 exportXCAF() 接受的文档定义。

bezierCurve、bsplineCurve、bezierSurface 和 bsplineSurface feature 序列化自由形控制几何，
并解析每个 pole 坐标中的表达式；rational weights、BSpline degree/knots/multiplicities/
periodic flags 和 construction tolerances 在对应 ShapeScope 构造器支持时可用。
solveSketch() 求解可序列化 2D line/circle/arc/BSpline entity 及 coincident、fixed、
horizontal、vertical、parallel、perpendicular、tangent、concentric、symmetry、
equal-length/equal-radius、distance、length、radius 约束，并报告 convergence、residual、
iterations、degrees of freedom 和逐约束 residual diagnostics。tangent 只定义在线、圆和
弧对之间；BSpline endpoint 可参与 point-reference，但 BSpline tangency 不属于有界 solver。
construction entity 参与求解但不输出。参数化 sketch 可解析尺寸表达式并放在任意 3D plane；
没有 profiles 时输出一个 wire，有序 profiles 输出 compound 和稳定的 sketchId.profileId
wire 引用，可直接用作 face.outer 和 face.holes。特征图还支持 sewing、shape repair、
same-domain unification、section 和 split。

## 错误、取消与限制

fillet() 和 chamfer() 保留 scalar size 参数，同时接受与 edgeIndices 对齐的 radii/
distances 数组。fillet 的 radius2 定义轮廓起点到终点的线性演化；radiusLaw 或 radiusLaws
定义从 0 到 1 严格递增的多点 OCCT law。chamfer 的 distance2/distances2 需要对齐的
referenceFaceIndices，每个参考面必须邻接对应 edge，并指定第一距离所在侧。
启用 includeHistory: true 时，结果包含 retained、OCCT-generated、modified 和 deleted
face/edge mapping；generation 与 deletion 可以同时指向同一输入 edge。没有可靠 history
的操作在 capabilities 中是 unsupported，请求其 history 返回 InvalidArgs。

错误是带稳定 code 的 KernelError 实例；每个派发错误都在 details.operation 中标识操作，
STEP、XCAF、IGES、STL、OBJ、PLY、glTF、VRML 和 BREP 失败还在 details.format 中提供小写
格式标识。低层 request 接受数值 timeout 或 { timeoutMs, signal, onProgress }。
onProgress 接收 { operation, fraction }，fraction 归一化到 0 至 1。tessellation、edge
tessellation、VRML、STL、OBJ、PLY、glTF 和 STEP/IGES shape/XCAF document transfer 将
回调连接到 OCCT Message_ProgressIndicator；mesh exchange 在 parser、serialization 和
buffer-materialization 边界通过 WorkerClient 的共享取消标志观察 AbortSignal，OCCT 操作
在下一个进度检查点停止并保持 Worker/句柄。取消排队请求只移除该请求；其他活动同步
Worker 操作仍会终止并重建 Worker，拒绝旧 queue 并使旧句柄失效。timeout 保持同样的
硬恢复行为。DirectClient 的同步调用不承诺硬 timeout；不受信 CAD 文件应交给 WorkerClient。

大 BSpline pole 数组可用 packed f64x3 顺序的 Float64Array。tessellate 返回 face-isolated
Float32Array positions/normals、Uint32Array indices/groups 和可选 UV；triangulationData
读取现有 face mesh 而不重新三角化，validateTriangulation 报告 mesh 属性和 triangle
问题，replaceTriangulation 返回带调用方 positions/indices/normals/UV 的 copy-on-write
face，repairTriangulation 使用 OCCT BRepMesh 重建复制 shape 的 face mesh 并生成法线。
普通输入 buffer 在 Worker 中会 transfer/detach，共享输入由调用方拥有且不 clone；
WorkerClient.requestShared() 将结果中的每个 kernel buffer 物化为 SharedArrayBuffer，不会
transfer，但 wasm 线性内存和 host shared buffer 之间仍复制一次。普通导出和 tessellation
buffer 在 promise 解决前会复制并释放。

Surface editing 会为 updateSurfacePole 和 convertSurfaceToBSpline 保留 outer/hole wire；
trimSurface 接受一个四边界并拒绝任意 trimmed topology，不会静默丢弃拓扑。
exportSTL 对 shape 和 indexed mesh 接受 encoding: "binary" | "ascii"（默认 binary）；
shape export 的 relative 默认 false，用 shape size 缩放 linearDeflection，mesh export 直接
写调用方三角形，因此没有 tessellation 选项。makeSurfaceFilling 的 constraints 条目
使用 wire 顺序的从零开始 boundary edgeIndex、face support 和 g1/g2 continuity；未指定边
保持 C0，重复/越界 index 以及没有该 edge pcurve 的 support face 会拒绝。
