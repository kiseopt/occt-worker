# 公共 CAD 平台能力矩阵

[English reference / 英文完整矩阵](capabilities.md)

> 编译期 Profile 与操作映射及操作总数明细，参见生成的 [Profile 能力矩阵](profile-capabilities.generated.zh-CN.md)。

本矩阵描述 `protocol/artifacts.json` 所固定的未修改上游 OCCT 提交支持的公共能力，但不表示项目绑定了全部非可视化 OCCT 类或工具包。`TKV3d`、`AIS`、`TKOpenGl`、`TKMeshVS` 等渲染和 UI 工具包、通用 OCAF 编辑、更广泛的 TKXMesh/TKExpress API，以及 pthread/TBB 执行均不属于 v1 公共范围。与渲染器无关的三角化仍属于公共能力。

仅当一项功能同时满足以下条件时，本矩阵才将其列为正式支持：

1. 已通过协议和类型化 TypeScript API 提供；
2. 可由 Release WASM 执行；
3. 已有正式文档；
4. 涉及宿主边界时，已有对应的集成测试。

## 内核与拓扑

- 受作用域管理并带代次校验的 `TopoDS_Shape` 句柄、缓冲区、批处理、参数校验、拓扑计数、带索引的子形状目录、邻接关系和子形状句柄，以及组合体、壳和组合实体的直接组装。
- 楔体、半空间等基元，以及线框、面、壳、实体、组合体和组合实体构造。
- 核心布尔、截面、非破坏分割，以及带源输入映射、选取/排除/材质规则、内部边界删除和容器构造的 General Fuse 单元选择。
- 拉伸、旋转、放样、管道扫掠、拔模、局部拉伸与旋转、圆柱孔、去特征、圆角、倒角、抽壳、偏移、缝合、修复、同域统一、变换和变换阵列。局部特征支持文档规定的长度、限制形状、半无限、贯穿、角度及原生孔限制模式。
- 根据闭合边界线框进行 N 边曲面填充，可添加内部点和 G1/G2 支持面约束；也可在已有曲面上用任意外边界和孔线框裁剪，并生成 pcurve。
- 通过原生 `BRepFeat_MakeLinearForm` 构造平面筋或槽，支持单侧或相向厚度向量及添加/切除语义。
- 提供形状级 ShapeUpgrade 连续性、角度、面积、闭合面和闭合边分割，以及选择性 Bezier 转换、内部线框删除和位置删除，并保留原生精度、分段、转换和边选择控制。`linearForm` 与 `revolutionForm` 提供原生平面/旋转筋槽，`BRepFeat_Gluer` 提供显式面边绑定压印。
- 倒角支持标量及逐边的第一、第二距离，并可显式指定参考面。
- 圆角支持标量半径、逐边半径、线性变半径，以及共享或逐边的多点半径变化规则。
- 拉伸、旋转、放样、管道扫掠、圆角/倒角、缝合、形状修复、同域统一，以及刚体、缩放、镜像和一般仿射变换提供完整的局部面/边历史。结果区分保留的子形状、生成或修改后的替换项和删除的输入。
- OCCT 不映射辅助 pipe-shell 脊线，因此该扫掠的历史支持为 `partial`；主脊线和轮廓仍有映射。
- 参数化 SDK 可组合布尔、孔、去特征、扫掠、圆角/倒角、修复、同域统一和变换的可用历史。其他特征路径使用确定性签名匹配，并明确诊断缺失或歧义。

## 曲线与曲面

- 构造直线、圆弧、圆、椭圆、双曲线、抛物线、偏移曲线、Bezier、BSpline 和圆柱螺旋线边；无界圆锥曲线会被限制为有限拓扑域。
- 查询边参数域，并计算点、一阶/二阶导数、切向量和曲率。
- 提取解析曲线类型和参数、Bezier/BSpline 元数据，以及完整的控制点、节点、重数和权重。
- 控制点编辑返回新边且不修改输入；支持原生 BSpline 节点插入/删除、升阶、有界降阶、周期性转换和批量控制网编辑，以及全局曲线和 U/V 曲面连续性、有限曲线裁剪及精确曲线到 BSpline 转换。
- 查询受限的面 UV 域，并计算点、一阶/二阶偏导数、按拓扑方向确定的法向量，以及主曲率、平均曲率和高斯曲率。
- 构造直接有界的平面、圆柱、圆锥、球面和环面，以及有限的拉伸、旋转、直纹和偏移曲面；支持解析曲面类型与参数提取，以及按拓扑裁剪的 U/V 等参线。
- 在有限拓扑上执行曲线/曲面点投影、曲线/曲面极值和曲线-曲线、曲线-曲面、曲面-曲面相交。驻点集合连续时会明确标记平行状态，并返回代表性的全局最近解。
- 支持带显式拓扑边界的有限矩形曲面裁剪、保留拓扑的 BSpline 转换，以及返回新面且不修改输入的控制点编辑。
- 支持 U/V BSpline 节点插入/删除、升阶、有界降阶、周期性转换、方向交换/反向和批量控制网编辑。
- 支持曲线/曲面的精确仿射重参数化并重建边界 pcurve，以及通过 `makeFaceOnSurface` 使用任意线框和 pcurve 裁剪。
- 支持 Bezier/BSpline 曲面构造、带可选 OCCT 变分光顺的点集 BSpline 近似、有界曲线/曲面延展、带 C0/G1/G2 约束的 N 边填充和完整控制数据提取。

## 参数化 CAD

### 草图覆盖摘要

| 范围 | 边界 |
| --- | --- |
| 直线、圆和圆弧实体 | 支持几何与尺寸约束；精确组合以特征契约为准。 |
| BSpline 实体 | 端点可参与点引用约束；BSpline 相切不在 v1 有界求解器范围内。 |
| 构造几何 | 参与求解，但不会出现在最终草图中。 |
| 求解器诊断 | 返回收敛状态、残差、迭代次数、自由度和逐约束残差。 |

| 约束分组 | 支持值 |
| --- | --- |
| 几何约束 | `coincident`、`fixed`、`horizontal`、`vertical`、`parallel`、`perpendicular`、`tangent`、`concentric`、`symmetry`、`equalLength`、`equalRadius` |
| 尺寸约束 | `distance`、`length`、`radius` |

- 可序列化特征图支持命名算术参数、依赖拓扑排序、循环拒绝、确定性重算、原子结果替换、失败回滚和作用域清理。
- 特征覆盖基元、轮廓、扫掠、圆角与倒角、抽壳、偏移、拔模、局部特征、孔、去特征、修复、截面、分割、缝合、变换和布尔操作。
- Bezier 和 BSpline 曲线/曲面的控制几何可使用表达式、有理权重，以及支持的次数、节点、重数和周期性选项。
- 逐特征诊断报告重算与抑制状态。特征被抑制或依赖失败时，不会发布部分结果作用域。
- 序列化写入 schema v1，可迁移原始无版本格式，并拒绝未知的未来版本。
- 可序列化的面和边引用保存几何类型及父形状归一化边界签名。引用可沿唯一特征路径跟随可用的 OCCT 历史，包括多输入操作，并明确报告删除、未映射或一对多结果。
- OCCT 不提供辅助 pipe-shell 脊线的历史，因此该拓扑明确标记为未映射。默认执行严格几何匹配并拒绝歧义；文档规定的场景可启用 `disambiguation: "initialIndex"` 或 `allowGeometryReplacement`。
- 引用可为下游圆角、倒角、抽壳、拔模、局部拉伸/旋转和去特征选择拓扑。没有可用 OCCT 历史时，操作使用确定性签名并报告歧义，不声称提供通用 B-Rep 命名。
- 草图支持可序列化的直线、圆、圆弧和 BSpline 实体、构造几何、上表所列几何与尺寸约束、任意平面、求解诊断和草图到特征引用。BSpline 端点可参与点约束；BSpline 相切不在此有界求解器范围内。
- 有序多轮廓草图返回组合和稳定的 `sketchId.profileId` 线框引用，可用于外边界、孔和下游实体特征。
- 可序列化装配支持可复用特征实例、参数驱动变换、产品层级、名称、颜色、图层、可见性、物理与 PBR 材质及验证属性，并映射到 STEP/XCAF 文档模型。

## 交换格式

### 格式总览

| 格式 | Shape 路径 | Mesh 或 document 路径 | 主要边界 |
| --- | --- | --- | --- |
| BREP | 导入/导出 | 仅缓存 | 每个缓存必须绑定精确 WASM artifact 哈希。 |
| STEP | Shape 导入/导出 | XCAF document 导入/导出 | 显式模型单位；支持 AP203/AP214/AP242；STEP 无法保留的 document link 会拒绝。 |
| IGES | Shape 导入/导出 | XCAF document 导入/导出 | 根层级和 presentation metadata 使用文档化 marker；不支持的语义标注会拒绝。 |
| STL | 三角化 Shape 导入/导出 | Indexed mesh 导入/导出 | 不保留 UV、material 或 hierarchy。 |
| OBJ | Shape 导入/导出 | Indexed mesh/document 导入/导出 | 保留 corner attribute，并可使用 MTL companion resource。 |
| PLY | 三角化 Shape 导入/导出 | Indexed mesh/document 导入/导出 | 支持 ASCII 和两种 binary endian；颜色及 header metadata 可选。 |
| glTF/GLB | 三角化 Shape 导入/导出 | Scene/document 导入/导出 | 在文档化限制内保留 JSON/GLB 打包、资源和标准 scene metadata。 |
| VRML | Shape 导入/导出 | Indexed mesh 导入/导出 | VRML 只存 RGB；输入 alpha 必须是不透明值。 |

#### BREP

- 支持内存中的导入导出，但 BREP 仅作为缓存格式使用，并绑定当前 WASM 的精确身份。

#### STEP、IGES 与 XCAF

- 仅交换形状时，STEP 和 IGES 支持显式模型单位、多个独立根、每个导入根对应的受作用域管理句柄，以及 OCCT 合并结果。STEP 导出使用调用方时间戳或协议默认值。
- STEP XCAF 支持根装配树、重复形状实例、局部变换、名称、RGBA 颜色、图层、可见性、物理材料元数据、验证属性、从 0 开始的面/边显示样式和几何句柄。STEP 将 PBR 材质映射为显示颜色；导出 GDT 时要求文档只有一个根。
- 原生 XCAF 二进制格式还保留命名相机 `views`、裁剪平面、组件路径 `shuo`、基准链接、图形标注拓扑、几何公差和完整的公开 PBR 字段。
- Worker 生成的 XmlXCAF 包含已验证的二进制恢复标记。节点颜色、子形状样式、带颜色的 SHUO 和第三方 XmlXCAF 输入必须使用二进制持久化；STEP 导出会拒绝其传输路径无法保留的基准和公差链接。
- IGES XCAF 通过已验证的元数据标记保留根层级、名称、RGBA 颜色、图层和实例变换，并拒绝 `gdt`、`datums`、`geometricTolerances`、`views` 和 `shuo`。

#### 网格格式

- STL 支持二进制和 ASCII 的形状或索引网格交换。可选网格导入返回索引几何、逐面片法线、ASCII 实体名称和精确二进制文件头。STL 不保留 UV、材质、层级或解析曲面。
- OBJ 支持负索引、独立角点属性、接缝展开、多边形三角化、对象/组/材质/平滑范围、材质库引用和支持的 MTL 配套数据。仅交换形状和对称的网格文档路径都可用。
- PLY 1.0 支持 ASCII 和两种二进制字节序。形状路径保留法线和常见 UV 名称；索引网格文档路径还保留 u8 RGBA 顶点颜色和有序的 `comment`/`obj_info` 元数据。
- glTF 2.0 JSON 和 GLB 支持内嵌或配套缓冲区、场景层级与实例、法线、UV、稀疏访问器、变形目标、四影响蒙皮、指定动画采样及 Z-up/Y-up 转换。文档导入导出保留标准 JSON 元数据、已解析资源和图元实例范围；合法文档可以不含三角形。TypeScript 层会在传入内核前解码 Draco 与 meshoptimizer 压缩。
- VRML 1/2 支持形状和索引网格交换。网格结果在格式限制内包含索引几何、法线、UV 和不透明的 u8 RGBA 颜色。

#### 交换格式的共同规则

- `probeFormat()` 识别 BREP、STEP、IGES、STL、OBJ、PLY、glTF/GLB、VRML 和原生 XCAF 二进制/XML，并报告编码和 STEP 文档元数据。
- 每种格式都有命名明确的 TypeScript 选项和生成协议契约，用于规定编码、单位、文档/资源行为、三角化控制和默认值。
- STEP/IGES 形状和文档、VRML、边三角化，以及 STL、OBJ、PLY 和 glTF 的形状/网格传输会报告归一化进度，并在 `WorkerClient` 中协作观察 `AbortSignal`。网格交换会在解析、序列化和复制输出缓冲区时检查取消。
- 交换操作的 `KernelFailure` 响应提供稳定的 `details.operation` 和 `details.format`；格式错误的请求和运行时失败可以省略 `details`。

## 网格处理

- 带 normals、indices、face groups 和可选 UV 的逐面隔离实体三角化，以及带拓扑
  group 的边折线三角化。
- STL、OBJ、PLY、glTF 和 GLB mesh import 为有效的三角化 TopoDS shape。
- 读取现有 face triangulation，以及返回新面且不修改输入的 positions、indices、normals
  和 UV。
- 检查缺失/空 mesh、非有限属性、无效索引和退化三角形的三角化校验；在复制的形状上使用 OCCT
  BRepMesh 重建并生成 normals。
- 带 position tolerance 的 typed-array weld，默认保留 normal/UV seam，面积加权
  vertex-normal 重建和 UV tangent 生成，并保留 face-group index range。
- 按目标三角形数或 ratio 的确定性 grid-cluster 简化，重建 normals 并保留
  face-group range。
- 最多八级的 midpoint triangle subdivision/refinement，保留 face-group range、
  插值 UV 和重建 normals。
- 确定性 indexed boundary-loop 提取和有界 planar-hole filling，明确拒绝非流形、
  分支、方向不一致、非平面、退化和自交边界。
- 从渲染 triangle 和 polyline-vertex index 映射回从零开始 OCCT face/edge index
  的拓扑选择表，并在内置 weld、simplify 和 subdivision 后保留映射。
- OBJ 和 glTF 提供对称的网格文档导入导出，并保留格式元数据和配套资源。
- PLY 保留位置、法线、UV、u8 RGBA 颜色和有序的全局文件头元数据；VRML 保留索引位置、法线、UV 和不透明的 u8 RGBA 颜色；STL 保留面片法线、ASCII 实体名称和精确二进制文件头。
- 这些保证受各格式自身限制。STL 和 PLY 索引导出会拒绝越界或重复索引、非有限数据和几何退化三角形。

## 查询、运行时和边界

- bounding box、mass properties、distance、point classification、validity、shape
  identity、topology type、resource statistics、worker、timeout recovery 以及 Node/browser
  宿主。
- oriented bounding box、拓扑感知 geometric projection/intersection、带 parallel
  状态报告的 stationary extrema 和详细 `BRepCheck` shape diagnostics。
- 可按面、边、顶点层级读取带索引的容差；编辑会返回复制后的新形状，不修改输入。
- 所有 minimum-distance solutions、inner-solid state、稳定拓扑 support index、edge
  parameter 和 face UV parameter。
- 两个 face 或 wire section 之间 pipe-like shape center-path 提取。
- section edge length、双侧 ancestor-face mapping、isolated contact vertex、
  intersection approximation 和可选输入 pcurve。
- 通过 `TKHLR` 的精确 B-Rep hidden-line removal，支持 parallel/perspective
  camera projection，并在 view coordinates 返回可复用 visible/hidden edge compound。
- 独立产物将固定 Emscripten Phase 3 wasm-EH 指令转换为标准 exception reference；
  Wasmtime 47 可通过冻结 ABI 执行 scope、基元、boolean、bbox、tessellation、结构化错误
  和资源清理。Node release-wasm 套件仍是完整操作测试，宿主测试只验证宿主边界和代表性
  内核路径。
- 形状与边三角化、VRML、网格交换，以及 STEP/IGES 形状和 XCAF 文档传输通过单字 `SharedArrayBuffer` 提供进度与协作取消。
- `WorkerClient.requestShared()` 分配共享输出缓冲区，并从 WASM 线性内存复制数据。共享二进制和网格输入跨 Worker 边界时不会被分离。
- pthread/TBB 构建不属于单线程 v1；浏览器并行通过多个 Worker 和独立句柄池实现。
