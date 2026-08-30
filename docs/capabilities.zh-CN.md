# 公共 CAD 平台能力矩阵

[English reference / 英文完整矩阵](capabilities.md)

本矩阵定义由 `protocol/artifacts.json` 固定的、未修改的上游 OCCT 提交所支持的公共
能力。它不表示绑定全部非可视化 OCCT 类或 toolkit。`TKV3d`、`AIS`、
`TKOpenGl`、`TKMeshVS` 可视化、通用 OCAF 编辑、更广泛的
TKXMesh/TKExpress API 以及 pthread/TBB 执行不属于 v1 公共范围。只有同时通过协议和
TypeScript API 暴露、由 release wasm 执行、有文档并在宿主边界适用时有集成测试的行为，
才算在下述行为和限制范围内完成。

## 内核与拓扑

- 已完成：带 generation 校验的作用域 `TopoDS_Shape` 句柄、buffer、batch、验证、
  拓扑计数、索引子形状目录、邻接、物化子形状句柄，以及直接的 compound/shell/compsolid
  组装。
- 已完成：wedge 和 half-space 等基元、wire/face/shell/solid 构造、核心布尔、section、
  非破坏 splitter、带源输入映射和 take/avoid/material 选择的 General Fuse 单元、内部边界
  移除和容器构造、extrusion、revolution、loft、pipe sweep、多面 draft、带 length/until/
  from/semi-infinite/thru-all 限制的 `BRepFeat` local prism add/cut、带 angle/
  until/from/thru-all 限制的 local revolution、支持全部原生限制模式的 cylindrical hole、
  face-selected defeature、fillet、chamfer、hollow、offset、sewing、healing、同域统一、
  变换和变换阵列。
- 已完成：从闭合边界 wire 进行 N-side surface filling，可选内部点约束和 G1/G2 支持面约束，
  以及在已有曲面上任意 outer/hole wire 的拓扑保留裁剪和生成 pcurve。
- 已完成：通过原生 `BRepFeat_MakeLinearForm` 实现平面 rib/slot，支持单侧或相向
  厚度向量以及 add/cut 语义。
- 已完成：shape-level ShapeUpgrade 的 continuity、angular、area、closed-face、closed-edge
  分割；选择性 Bezier 转换；internal-wire 和 location 移除，带原生精度、分段、转换和
  edge 选择控制。原生 planar/revolution rib/slot 通过 `linearForm` 和
  `revolutionForm` 提供，显式 face/edge-bound glue/imprint 通过
  `BRepFeat_Gluer` 提供。
- 已完成：chamfer 暴露标量及按边的第一/第二距离，并支持显式参考面。
- 已完成：fillet 暴露标量及按边半径、线性起止演化以及共享或逐边多点 radius law。
- 已完成：extrusion、revolution、loft、pipe sweep、fillet/chamfer、sewing、shape repair、
  同域统一以及刚体/缩放/镜像/一般仿射变换提供完整的局部 face/edge history，包括
  retained、OCCT generated/modified 替换映射和 deleted 输入子形状。参数化 SDK 组合
  booleans、cylindrical holes、defeaturing、各类 sweep、fillet/chamfer、修复、统一和
  变换；其他特征使用确定性签名匹配，并明确诊断缺失或歧义。Pipe-shell 辅助脊线没有
  OCCT history，因此该部分保持 partial。

## 曲线与曲面

- 已完成：line、arc、circle、ellipse、hyperbola、parabola、offset、Bezier、BSpline 和
  cylindrical helix edge 构造；无界圆锥曲线使用有限拓扑域。
- 已完成：edge 参数域、点/一阶和二阶导数、切向及曲率求值。
- 已完成：解析曲线类型和参数、Bezier/BSpline degree 与 size 元数据、完整 pole/knot/
  multiplicity/weight 读取、copy-on-write pole 编辑、BSpline 节点插入/删除、升阶及带
  max error 的近似降阶、周期性转换、批量控制网编辑、全局曲线和 U/V 曲面连续性、有限
  曲线裁剪和精确 curve-to-BSpline 转换模式。
- 已完成：受限 face UV 域、点和一阶/二阶导数、拓扑定向法线以及主/平均/高斯曲率。
- 已完成：直接有界的 plane、cylinder、cone、sphere、torus face；有限 linear-extrusion、
  revolution、ruled、offset surface face；解析曲面类型/参数；拓扑裁剪的 U/V iso-curve。
- 已完成：拓扑有界的曲线/曲面点投影、curve/curve-surface/surface extrema，以及拓扑
  裁剪的 curve-curve/curve-surface/surface-surface 交运算。并行极值会明确标记，因驻点
  集连续而返回代表性的全局最近解。
- 已完成：有限矩形曲面裁剪、拓扑保留的 BSpline 转换和 copy-on-write pole 编辑、U/V
  BSpline 节点插入/删除、升阶和带 max error 的近似降阶、周期性转换、精确仿射曲线/
  曲面重参数化和边界 pcurve 重建、方向交换/反向、批量控制网编辑、任意 wire/pcurve
  裁剪、Bezier/BSpline 曲面构造、带可选 OCCT variational length/curvature/torsion
  fairing 的点集 BSpline 近似、有界曲线/曲面扩展、带 C0/G1/G2 约束的 N-side filling，
  以及完整 pole/knot/multiplicity/weight 读取。

## 参数化 CAD

- 已完成：可序列化参数模型、安全的命名算术参数及依赖、特征 DAG 排序和循环检查、确定性
  序列化重算、原子结果替换、失败回滚和作用域资源清理。特征涵盖基元、polygon/face
  profile、extrusion/revolution、loft、pipe sweep、fillet、chamfer、hollow、offset、
  draft、local prism、cylindrical holes、defeature、拓扑修复、同域统一、section、split、
  sewing、变换和 boolean cut/fuse/common。
- 已完成：表达式驱动的 Bezier/BSpline 曲线曲面控制几何序列化，包括 rational weights、
  BSpline degree、knot、multiplicity 和 periodic 选项。
- 已完成：逐特征状态诊断和 suppression 语义；suppressed 特征不参加重算，依赖特征报告
  failed 诊断且不发布部分 scope。
- 已完成：版本化参数序列化写入 schema v1，迁移原始无版本表示并拒绝未知未来版本。
  可序列化 face/edge 引用持久化 geometry-family 和 parent-normalized-bound 签名，在参数
  重算后唯一解析，报告缺失或歧义，并可为下游 fillet/chamfer/hollow/draft/local prism/
  revolution/defeature 选择拓扑。引用也可通过 `source` 跟随上游引用；模型自动
  组合唯一特征图路径上的 boolean、hole、defeature、各种 sweep、fillet/chamfer、sew、
  repair、同域统一和变换 history，并报告 deleted、unmapped 或一对多映射。辅助
  pipe-shell spine 使用同样的显式 unmapped 诊断。对称 equal-signature 候选可显式选择
  `disambiguation: "initialIndex"`；有意的几何族替换可在唯一 normalized-bound
  时选择 `allowGeometryReplacement`。严格几何匹配和歧义拒绝仍为默认。
- 已完成：可序列化 2D line/circle/arc/BSpline sketch entity、construction geometry、
  几何约束（coincident、fixed、horizontal、vertical、parallel、perpendicular、line/
  circle/arc 对的 tangent、concentric、symmetry、equal length/radius）、distance/length/
  radius 尺寸约束、逐约束残差、非线性 solver 诊断、任意 sketch plane 以及 sketch-to-wire/
  face/feature 引用。BSpline 端点支持点约束；BSpline tangency 不属于有界 v1 solver，
  不支持的组合会明确拒绝。
- 已完成：有序多 profile sketch 输出、稳定的 `sketchId.profileId` wire 引用、
  compound sketch 结果以及下游 outer/hole Face 和实体特征使用。
- 已完成：可序列化参数化 assembly、可复用 feature component instance、参数驱动变换、
  product hierarchy、名称、颜色、层、可见性、物理材料、PBR visual material 和验证属性，
  映射到现有 STEP/XCAF 文档导出模型。

## 交换格式

- 已完成：绑定精确 wasm 哈希的 BREP 导入导出。
- 已完成：带显式模型单位和确定性 header timestamp 的 shape-only STEP 导入导出。
- 已完成：从 shape 和已验证 indexed triangle buffer 导出 binary STL。
- 已完成：STEP XCAF 的根装配树、重复 shape instance、局部变换、label 名称、RGBA 颜色、
  层、可见性、物理材料名称/描述/密度、面积/体积/质心验证属性、从零开始 face/edge
  presentation style 和 geometry handle。STEP 仅把 PBR material 映射为 presentation color；
  原生 XCAF binary 还保留 bounded named-camera views、clipping-plane label、component-path
  SHUO、datum 元数据、尺寸/datum/GDT presentation shape，以及 geometric tolerance 的公开
  字段。GDT 导出要求一个文档根；worker 生成 XML 使用 binary recovery marker，节点颜色、
  subshape style、带颜色 SHUO 和第三方 XmlXCAF 输入要求 binary；无法由 STEP 保留的链接
  会明确拒绝。
- 已完成：通过 memory stream 的 IGES shape 导入导出（`TKDEIGES`），带显式模型
  单位和多根计数。
- 已完成：通过 memory stream 的 IGES XCAF 文档导入导出，保留根层级、名称、RGBA 颜色、
  层和 occurrence transform；`gdt`、`datums`、`geometricTolerances`、
  `views` 和 `shuo` 等无法保留的语义标注会明确拒绝。
- 已完成：STL 1.0 binary/ASCII 内存流导入导出；默认导入生成三角化 TopoDS face，不恢复
  解析曲面；可选 mesh 导入返回 indexed positions、indices、facet normals、ASCII solid
  name 和精确 binary 80-byte header。STL 不保留 UV、material 或层级。
- 已完成：OBJ 内存流导入导出，支持 vertex/polygon geometry、负索引、独立 position/UV/
  normal corner index、seam expansion 和 polygon triangulation。默认 shape-only；可选
  document 导入返回 indexed mesh、连续 object/group/material/smoothing ranges、material
  library 引用，并从 MTL 资源解析支持的颜色和 map 字段；对称 document 导出写回 OBJ、
  companion MTL 和支持的材料字段。
- 已完成：PLY 1.0 ASCII、little-endian 和 big-endian 内存流导入导出，包含 vertex position
  和 polygon face；shape 路径保留 vertex normals 和常见 UV property，indexed mesh-document
  路径保留 u8 RGBA vertex colors 以及有序 `comment`/`obj_info` header 元数据。
- 已完成：glTF 2.0 JSON/GLB 内存流导入导出，支持 embedded/companion buffer、list/strip/
  fan 三角形、active-scene 层级变换和 instance、NORMAL/TEXCOORD_0、sparse accessor、
  POSITION/NORMAL morph、四影响 linear blend skinning、动画采样和 OCCT Z-up/glTF Y-up
  转换。可选 document 导入保留源 JSON、材质、纹理、动画、skin、camera、extension、
  extras，返回解析后的 buffer/image 和 primitive instance range；对称导出保留 JSON 元数据
  和资源。TypeScript 预处理使用固定 Draco 和 meshoptimizer runtime 解码
  `KHR_draco_mesh_compression` 与 `EXT_meshopt_compression`。
- 已完成：VRML 1/2 shape 和 indexed mesh 内存流导入导出；可选导入返回 indexed positions、
  normals、UV、opaque u8 RGBA colors 和 triangle indices；mesh 导出在格式限制内接受相同
  indexed 属性。
- 已完成：原生 OCCT/XCAF binary persistence、worker 生成的 XmlXCAF 和验证过的内嵌
  recovery marker；在 XML 限制内保留完整 PBR visual-material 元数据。
- 已完成：对 BREP、STEP、IGES、STL、OBJ、PLY、glTF/GLB、VRML 和原生 XCAF binary/XML
  的内存格式探测，包括编码和 STEP 文档元数据检测。
- 已完成：shape-only STEP/IGES 多根导出及导入时独立作用域 root handle，同时返回组合
  OCCT 结果。
- 已完成：每个支持交换格式的 TypeScript 和生成协议 option contract，包括共享 shape
  tessellation 控制、格式 encoding/version、STEP/IGES unit、document/resource 选项、原生
  XCAF persistence format 和冻结的逐操作默认值。
- 已完成：shape-only 和 XCAF STEP/IGES、VRML、edge tessellation 以及 shape/mesh STL、
  OBJ、PLY、glTF 传输报告归一化 OCCT progress；WorkerClient 同一路径协作观察 AbortSignal。
  mesh exchange 在 parser、serialization 和 buffer materialization 边界确认取消；每个交换
  失败提供稳定的 `details.operation` 和 `details.format`。

## 网格处理

- 已完成：带 normals、indices、face groups 和可选 UV 的逐面隔离实体三角化，以及带拓扑
  group 的边折线三角化。
- 已完成：STL、OBJ、PLY、glTF 和 GLB mesh import 为有效的三角化 TopoDS shape。
- 已完成：读取现有 face triangulation，以及 copy-on-write 替换 positions、indices、normals
  和 UV。
- 已完成：检查缺失/空 mesh、非有限属性、无效索引和退化三角形的三角化校验；使用 OCCT
  BRepMesh copy-on-write 重建并生成 normals。
- 已完成：带 position tolerance 的 typed-array weld，默认保留 normal/UV seam，面积加权
  vertex-normal 重建和 UV tangent 生成，并保留 face-group index range。
- 已完成：按目标三角形数或 ratio 的确定性 grid-cluster 简化，重建 normals 并保留
  face-group range。
- 已完成：最多八级的 midpoint triangle subdivision/refinement，保留 face-group range、
  插值 UV 和重建 normals。
- 已完成：确定性 indexed boundary-loop 提取和有界 planar-hole filling，明确拒绝非流形、
  分支、方向不一致、非平面、退化和自交边界。
- 已完成：从渲染 triangle 和 polyline-vertex index 映射回从零开始 OCCT face/edge index
  的拓扑选择表，并在内置 weld、simplify 和 subdivision 后保留映射。
- 已完成：OBJ/glTF 对称 mesh-document 导入导出及其格式元数据和 companion resource；
  PLY 保留 positions、normals、UV、u8 RGBA colors 和有序全局 header metadata；VRML 保留
  indexed positions、normals、UV 和 opaque u8 RGBA；STL 保留 facet normals、ASCII solid
  name 和精确 binary header。STL/PLY indexed export 会拒绝越界、重复、非有限和几何退化
  三角形。

## 查询、运行时和边界

- 已完成：bounding box、mass properties、distance、point classification、validity、shape
  identity、topology type、resource statistics、worker、timeout recovery 以及 Node/browser
  宿主。
- 已完成：oriented bounding box、拓扑感知 geometric projection/intersection、带 parallel
  状态报告的 stationary extrema 和详细 `BRepCheck` shape diagnostics。
- 已完成：按 face/edge/vertex 拓扑层级的 indexed tolerance inspection 和 copy-on-write 编辑。
- 已完成：所有 minimum-distance solutions、inner-solid state、稳定拓扑 support index、edge
  parameter 和 face UV parameter。
- 已完成：两个 face 或 wire section 之间 pipe-like shape center-path 提取。
- 已完成：section edge length、双侧 ancestor-face mapping、isolated contact vertex、
  intersection approximation 和可选输入 pcurve。
- 已完成：通过 `TKHLR` 的精确 B-Rep hidden-line removal，支持 parallel/perspective
  camera projection，并在 view coordinates 返回可复用 visible/hidden edge compound。
- 已完成：独立产物将固定 Emscripten Phase 3 wasm-EH 指令转换为标准 exception reference；
  Wasmtime 47 可通过冻结 ABI 执行 scope、基元、boolean、bbox、tessellation、结构化错误
  和资源清理。Node release-wasm 套件仍是完整操作测试，宿主测试只验证宿主边界和代表性
  内核路径。
- 已完成：tessellation、edge tessellation、VRML、mesh exchange 和 shape/XCAF STEP/IGES
  transfer 通过一个字的 `SharedArrayBuffer` 暴露 progress 与协作取消。
  `WorkerClient.requestShared()` 将批量二进制结果直接物化为 `SharedArrayBuffer`，
  shared binary/mesh 输入跨 Worker 不 detach，wasm 线性内存复制保持明确。pthread/TBB 构建
  不属于单线程 v1；浏览器并行通过多个 Worker 和 arena 实现。
