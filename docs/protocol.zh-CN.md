# Protocol v1.2.0 / 协议 v1.2.0

[English reference / 英文规范](protocol.md)

> **版本说明：** 本页描述 Protocol Wire Version `v1.2.0`（与 `protocol/artifacts.json` 一致）。npm 包发布版本在 `package.json` 中独立维护。

## 请求与响应

协议的权威定义位于 `protocol/`：`operation-contracts.json` 定义每个操作的参数和结果，`protocol.schema.json` 包含生成的请求与结果 schema。若 `operation-contracts.json` 或 `historySupport` 遗漏任何操作，生成过程会失败。

协议帧分为三种：

- 请求：`{ id, op, args }`
- 成功响应：`{ id, ok: true, result }`
- 失败响应：`{ id, ok: false, error: { code, message, details? } }`

ID 由客户端分配并单调递增。`message` 用于人工诊断；`code` 和文档明确规定的 `details` 字段用于程序分支。协作取消统一返回 `Cancelled`，不复用算法、三角化或交换格式的失败码。

`details` 是可选字段。已解析操作派发的 `KernelFailure` 包含 `details.operation`；交换操作还包含 `details.format`，取值为 `step`、`xcaf`、`iges`、`stl`、`obj`、`ply`、`gltf`、`vrml` 或 `brep`。格式错误的帧和运行时失败可以省略 `details`。`probeFormat()` 通过 `BINFILE` 文件头识别原生 XCAF 二进制数据，通过 `XmlXCAF` 文档格式识别 XML。

### 消息帧示例

**1. 请求帧：**
```json
{
  "id": 1,
  "op": "makeBox",
  "args": {
    "scopeId": 1,
    "size": [100, 60, 4]
  }
}
```

**2. 成功响应帧：**
```json
{
  "id": 1,
  "ok": true,
  "result": {
    "shape": 1
  }
}
```

**3. 失败响应帧：**
```json
{
  "id": 2,
  "ok": false,
  "error": {
    "code": "InvalidArgs",
    "message": "Box dimensions must be positive",
    "details": {
      "operation": "makeBox"
    }
  }
}
```

### WebAssembly 串行消息 ABI

WebAssembly 边界是同步且串行的。单次调用轮次遵循以下步骤：

1. 宿主对每个实例只调用一次 `_initialize`。
2. 宿主使用 `k_alloc(len)` 在 WebAssembly 线性内存中为 UTF-8 JSON 请求分配空间。
3. 宿主将请求字节写入分配的指针。
4. 宿主调用 `k_handle(ptr, len)`，返回值是响应的字节长度。
5. 宿主从 `k_response_ptr()` 指针开始读取该长度的字节。
6. 宿主使用 `k_free(ptr)` 释放请求缓冲区。

公共消息 ABI 包含初始化、消息入口、内存分配与释放，以及缓冲区访问器。完整的 WebAssembly 导入导出面冻结在 `wasm-surface.json` 中。

## 句柄、作用域和缓冲

- 形状句柄是 `u32` 值，只在创建它的内核实例中有效。
- 类型化 API 将句柄包装为带 epoch 检查的 `ShapeHandle`。
- 底层 `request()` 是明确标记为不安全的原始协议接口。直接传入 `u32` 会绕过 TypeScript 的所有者、epoch 和作用域来源检查。
- 内核的 generation 检查会拒绝过期句柄，但裸数值不包含实例身份；同一数值可能在另一个活动实例中指向有效形状。
- 作用域拥有通过其 `scopeId` 创建的全部形状。`endScope` 释放该作用域中尚未释放的形状；`release` 拒绝过期或未知句柄。

缓冲区由 `{ bufferId, byteLength, layout }` 描述：

```json
{
  "bufferId": 1,
  "byteLength": 2400,
  "layout": "f32x3"
}
```

- 普通 `DirectClient` 请求把输出缓冲区复制到宿主内存，并立即释放内核中的描述符。
- `DirectClient.createBuffer()` 创建可在同一线程重复使用的 `DirectBuffer`。每次 WASM 调用后都要重新获取 `view()`；不再使用时调用 `freeBuffer()`。`releaseAll()` 也会使所有活动的直接缓冲区失效。
- Worker 客户端不公开内核缓冲区 ID。普通请求返回可传输的 `ArrayBuffer`。
- `WorkerClient.requestShared()` 为每个输出描述符分配 `SharedArrayBuffer`，再从 WASM 线性内存复制数据。
- 共享输入跨 Worker 边界时不会被传输或分离。两种 Worker 输出模式都需要在 WASM 线性内存与宿主内存之间复制一次。

## Batch 批量执行

`batch` 按顺序执行操作，遇到第一个失败就停止。`{"$ref": N}` 引用从零开始的第 `N` 个先前操作的 `shape` 结果。

```json
{
  "id": 3,
  "op": "batch",
  "args": {
    "scopeId": 1,
    "ops": [
      { "op": "makeBox", "args": { "size": [100, 60, 4] } },
      { "op": "makeCylinder", "args": { "radius": 8, "height": 4, "origin": [50, 30, 0] } },
      { "op": "booleanCut", "args": { "base": { "$ref": 0 }, "tools": [{ "$ref": 1 }] } }
    ]
  }
}
```

Batch **不是事务**：失败前创建的形状仍归其作用域所有。外层协议响应仍为成功；嵌套失败通过 `result.error` 返回，其中包含 `details.operation: "batch"`、`nestedOperation`，交换操作还会包含 `format`。

## 网格布局与矩阵

三角化使用以下布局：

- 位置：`f32x3`
- 法线：`f32x3`
- 三角形索引：`u32`
- 面分组：`u32x3`，每项为 `(faceIndex, indexStart, indexCount)`
- 可选 UV：`f32x2`，与位置数组对齐

每个面都有独立的顶点范围，调用方不能假设不同面之间共享顶点。面分组的偏移和数量按 `indices` 中的标量条目计数，因此一个三角形占三个条目。边三角化使用 `(edgeIndex, vertexStart, vertexCount)`，偏移和数量按 `f32x3` 顶点计数。面、边、顶点和批处理索引均从 0 开始。

`matrix12` 是行主序
`[m11,m12,m13,m14,m21,m22,m23,m24,m31,m32,m33,m34]`，将点映射为
`(m11*x+m12*y+m13*z+m14, m21*x+m22*y+m23*z+m24, m31*x+m32*y+m33*z+m34)`。
对 `transform` 而言，`matrix` 优先，其他变换部分会被忽略。没有 matrix
时，合成矩阵是 `T*R*S*M`，点依次经历镜像、缩放、旋转和位移。
`massProps.inertia` 是返回中心处、全局模型坐标中的行主序
`[Ixx,Ixy,Ixz,Iyx,Iyy,Iyz,Izx,Izy,Izz]`。

## 能力、版本和默认值

公共默认值定义在 `operation-contracts.json` 的 `semantics.defaults` 中，并生成到 `PROTOCOL_SEMANTICS` 和 `protocol.schema.json` 的 `x-protocolSemantics`。主要默认值包括：

- 分类容差：`1e-7`
- 建模与修复容差：`1e-6`
- 三角化和 STL 偏差：线性 `0.1`，角度 `0.5`
- 原点：`(0,0,0)`；轴方向：`(0,0,1)`
- 历史和 UV 输出：`false`
- 多边形闭合和变换复制：`true`
- 省略的变换分量：单位变换

交换默认值按操作定义：STEP 和 IGES 使用 `mm`；STEP 使用协议规定的默认时间戳；STL 默认导出二进制，PLY 默认导出 ASCII，glTF 默认导出 GLB，原生 XCAF 默认使用二进制持久化；可选的网格或文档导入结果默认关闭。基于形状的 STL、OBJ、PLY、glTF 和 VRML 导出器共享线性偏差、角度偏差和相对模式选项；基于缓冲区的重载忽略三角化选项。

## 历史和 ShapeUpgrade

历史记录只描述一次操作中的局部映射，不是持久拓扑命名。跨操作命名和特征树引用由更高层的参数化 SDK 负责。

`historySupport` 为每个操作指定支持级别：

- `full`：三个布尔操作、`cylindricalHole`、`defeature`、`extrude`、`revolve`、`loft`、`sweepPipe`、`fillet`、`chamfer`、`sew`、`fixShape`、`unifySameDomain`、`transform`、`generalTransform`、`translate`、`rotate`、`scale` 和 `mirror`
- `partial`：`sweepPipeShell`。OCCT 会映射主脊线和轮廓，但不提供辅助脊线的拓扑历史
- `unsupported`：没有可靠 OCCT 历史来源的操作

`shapeUpgrade` 封装形状级 OCCT ShapeUpgrade 算法，支持 `continuity`、`angle`、`area`、`closedFaces`、`closedEdges`、`convertToBezier`、`removeInternalWires` 和 `removeLocations` 模式。

- 连续性条件为 `c0` 到 `c3` 或 `cn`，角度以弧度表示。
- 面积分割必须在最大面积、近似份数和固定 U/V 份数中选择一种方式。
- 删除内部线框时使用投影轮廓面积；删除位置可从形状、组合、实体、壳或面层级开始。
- 分割和转换模式共享精度边界、曲面分段模式和边选择规则：`0` 表示自由边，`1` 表示共享边，`2` 表示全部边。
- ShapeUpgrade 会改变拓扑，不沿用输入的面和边索引，也不报告操作历史。

## 操作目录

### 语义模块

当前 v1.2.0 的操作按语义模块组织如下。精确的操作名称和 Profile 成员关系以
[`protocol/modules.json`](../protocol/modules.json) 和 [`protocol/operations.json`](../protocol/operations.json) 为准。

| 语义模块 | 范围 |
| --- | --- |
| `runtime` | capabilities、作用域、释放、buffer、统计和 batch |
| `geometry-topology` | 曲线、曲面、构造、近似、编辑、连续性和容差 |
| `topology-query` | 子形状目录、邻接、类型、身份、包围盒和 OBB |
| `modeling` | 基元、扫掠、局部特征、偏移、圆角、倒角、孔和变换 |
| `algorithms` | 布尔/截面分析、投影、极值、交运算、HLR、修复和 ShapeUpgrade |
| `tessellation` | 形状/边三角化及现有三角化读取 |
| `mesh` | 三角化校验、替换和修复 |
| `exchange-mesh` | STL、OBJ、PLY、glTF/GLB 和 VRML 网格/文档交换 |
| `step-shape-exchange` | BREP、STEP shape 交换和格式探测 |
| `cad-document-exchange` | STEP/IGES XCAF 文档及原生 XCAF persistence |

`capabilities` 是已加载构建实际可用操作集的最终依据；逐操作参数和结果由
`operation-contracts.json` 定义，本页只说明线协议规则。

## 查询、截面和 HLR

- `distance` 返回全部最小距离解、`innerSolution` 及两个输入上的顶点、边或面支持类型。支持记录在属于对应输入时带从 0 开始的稳定拓扑索引，并在适用时包含边参数或面 UV 参数。
- `middlePath` 提取管状形状两个面或线框截面之间的 OCCT 中心路径。
- `sectionAnalysis` 返回带索引的截面边、长度、两个输入上的祖先面、孤立截面顶点，以及可选的交线近似和输入 pcurve。
- `projectHLR` 使用精确的 `TKHLR` B-Rep 隐藏线算法，返回受作用域管理的 `visible` 和 `hidden` 边组合。`direction` 从相机指向模型，`up` 固定视图方向。默认使用平行投影；透视投影要求提供从 Z=0 投影平面到视点的正 `focus` 距离。结果边位于投影坐标中，Z 坐标均为 0。
- `getSubShape` 按从 0 开始的索引，为指定子形状创建受当前作用域管理的句柄。
- 曲线求值使用原生 OCCT 边参数域，返回点、一阶和二阶导数，以及带可用性标记的切线与曲率。
- 曲面求值使用受限的面 UV 域，返回导数数据、按拓扑方向确定的法向量，以及有定义时的有符号主曲率、平均曲率和高斯曲率。反转面会反转法向量、有符号主曲率和平均曲率，但不改变高斯曲率。

## 曲线、曲面和网格操作

- `editCurveBSpline` 和 `editSurfaceBSpline` 使用显式 action 值完成节点插入与删除、升阶、周期性转换、方向交换或反向，以及完整控制网替换。协议索引从 0 开始；每个操作都返回受作用域管理的新形状，输入保持不变。
- `makeEdgeArc` 接受三个点，或由 `center`、`normal`、`radius`、`startAngle`、`endAngle` 和可选 `xDirection` 组成的圆定义。
- 双曲线和抛物线边必须显式提供有限参数界限，因为其自然 OCCT 参数域无界。
- `makeEdgeOffset` 保留基准边的参数域，偏移方向遵循 OCCT 的“切向量 × 参考方向”叉乘约定。
- `makeEdgeBSpline` 接受 JSON 点或 `f64x3` 输入缓冲区。默认使用插值；`mode: "controlPoints"` 接受次数、节点、重数、权重和周期性参数。
- `makeEdgeHelix` 从圆柱面上的二维直线构造边，并支持两种旋向。
- 派生曲面使用输入边或面的有限拓扑参数域；曲面偏移会保留外边界和孔线框。
- Bezier 和 BSpline 曲面使用 U-major 控制网构造有限面。`makeSurfaceFilling` 默认采用 C0 边界，也可使用 G1/G2 支持面和内部点约束。`makeFaceOnSurface` 不修改输入。
- `surfaceIsoCurve(face, "u" | "v", parameter)` 返回按外边界和孔裁剪的受作用域管理边。切线、法向量或曲率无定义时，相应布尔字段为 `false`，可选值不返回。
- `inspectTolerances()` 返回按从 0 开始索引排列的面、边和顶点容差。`ShapeScope.setTolerance()` 在复制后的形状上修改指定拓扑层级，输入保持不变。
- 投影、极值和相交运算针对有限拓扑，并遵守外边界与孔。驻点解形成连续集合时，极值结果设置 `parallel`，并返回一组代表性的全局最近点。
- `intersectCurveCurve()` 和 `intersectSurfaceSurface()` 为孤立点或一维结果创建受作用域管理的顶点或边句柄。
- `tessellate` 返回按面隔离的三角化数据。`weldTessellation()` 按位置容差合并顶点，默认保留硬法线和 UV 接缝，并重写 `faceGroups`。
- `recalculateVertexNormals()` 计算面积加权法线；`generateTangents()` 为带 UV 的三角化生成归一化 `f32x4` 切线和旋向。
- `findBoundaryLoops()` 返回顺序确定、方向一致的索引边界环。`fillPlanarHoles()` 只处理调用方选定的简单平面环，并明确拒绝非流形、分支、方向不一致、非平面、退化或自交边界。

各网格与文档交换格式的字段、验证规则和资源行为见下文“交换与缓存”。

## 参数化 CAD

协议通过 TypeScript 客户端公开可序列化的 `ParametricModel` 特征层。表达式使用命名算术参数，不调用 `eval`；特征依赖按拓扑排序，循环依赖会被拒绝。每次重算都在新作用域中构建，并原子发布结果。特征定义覆盖基元、轮廓、扫掠、圆角、倒角、抽壳、偏移、拔模、局部特征、孔、去特征、修复、变换、布尔、截面和分割。Bezier 与 BSpline 曲线/曲面的每个控制点坐标都可使用表达式，并支持有理权重及相应的次数、节点和周期性字段。

序列化输出使用 `schemaVersion: 1`。无版本输入会迁移到 v1，未知的未来版本会被拒绝。特征可设为抑制；诊断状态为 `ok`、`suppressed` 或 `failed`。依赖失败时不会发布部分重算结果。面和边引用保存几何类型及按父形状归一化的边界签名，也可通过 `source: "id"` 沿唯一的特征历史路径跟随上游引用。删除、未映射和一对多结果分别报告为 `missing` 或 `ambiguous`。默认执行严格匹配；只有文档规定的场景可显式启用 `disambiguation: "initialIndex"` 或 `allowGeometryReplacement`。

`solveSketch` 接受可序列化的二维直线、圆、圆弧和 BSpline 实体、构造几何、几何与尺寸约束，以及任意三维草图平面。结果包含收敛、残差、迭代次数、自由度和逐约束诊断。BSpline 端点可以参与点引用约束；BSpline 相切不在 v1 求解器的范围内。不提供 `profiles` 时，草图结果为一条线框；有序 `profiles` 会生成组合，并提供稳定的 `sketchId.profileId` 线框引用，可用作面的外边界或孔。

## 交换、缓存和错误

### 交换与缓存

所有交换操作都使用内存流，不访问文件系统。

#### STEP 与 XCAF

- STEP 的 `unit` 默认为 `"mm"`，也接受 `"cm"`、`"m"`、`"inch"` 和 `"foot"`。该值定义内核无量纲坐标所代表的模型长度。
- `exportSTEP` 按所选单位写出坐标，返回布局为 `step-utf8` 的 `data`。`importSTEP` 把文件声明的单位转换为请求的模型单位，并返回 `shape` 和 `rootCount`。
- `exportSTEPDocument` 接受由 `part` 和 `assembly` 节点组成的索引树。子节点带局部 `matrix12` 变换；节点可包含名称、RGBA 颜色、图层、可见性、物理材料元数据和验证属性。
- 材料密度不得为负。省略密度元数据时，单位和类型分别默认为 `kg/m^3` 和 `mass density`。原生 XCAF 保留全部材料字段；STEP 可能省略 `densityValueType`。
- 原生 `exportXCAF` 和 `importXCAF` 在二进制文件及 Worker 生成的 XML 中保留基准、关联的几何公差和图形标注。
- Worker 生成的 XML 是带已验证二进制恢复标记的标准 XmlXCAF。内存导入只接受 `exportXCAF()` 生成的 XML；第三方 XmlXCAF 必须先转换为 XCAF 二进制格式。
- XML 导出拒绝节点 `color`、非空 `subshapeStyles` 和 SHUO `color`。需要这些 RGBA 字段时，应使用 XCAF 二进制格式。
- `exportSTEPDocument` 拒绝固定 STEP 传输无法保留的基准或几何公差链接。导出 GDT 时，文档必须恰好有一个根节点。
- `importSTEPDocument` 返回实例树、每个节点对应的受作用域管理形状，以及一个合并后的文档形状。原生 XCAF 视图还保留命名裁剪平面标签的原点、法向量和封盖状态。
- STEP 导出使用调用方传入的 `timestamp` 或协议默认值，不读取系统时钟。不同宿主生成的字节不保证完全一致。

原生 XCAF 尺寸、基准和几何公差可包含 `presentation: { shape, name? }`。其中 `shape` 是存储在 OCAF 标注标签上的图形拓扑；导入时返回受作用域管理的句柄。二进制持久化及 Worker 生成 XML 所用的恢复标记都会保留该拓扑。

#### IGES

- IGES 形状和 XCAF 文档操作使用与 STEP 相同的显式长度单位。
- `exportIGESDocument` 和 `importIGESDocument` 通过已验证的元数据标记保留 `part`/`assembly` 根层级、名称、RGBA 颜色、图层和实例变换。
- IGES 无法保留 `gdt`、`datums`、`geometricTolerances`、`views` 或 `shuo`，文档导出会拒绝这些字段。
- `exportIGES` 接受 `mode: "faces"`（默认）或 `mode: "brep"`。
- STEP 和 IGES 形状导出接受一个 `shape` 或多个独立的 `shapes`。导入返回 OCCT 合并后的 `shape`、每个传输结果对应的受作用域管理句柄 `shapes`，以及 `rootCount`。
- 仅交换形状时，IGES 不传输 XCAF 名称、颜色、图层、材质或装配关系。

#### STL

- `exportSTL` 接受 `encoding: "binary" | "ascii"`，默认使用二进制。ASCII 导出接受可打印的 `solidName`，二进制导出接受精确的 80 字节 `binaryHeader`。
- 形状导出还接受 `relative`，用于按形状尺寸解释 `linearDeflection`；网格重载直接写出提供的三角形，因此忽略三角化选项。
- 选择返回网格时，导入还会返回逐面片法线和相应的名称或文件头元数据。STL 导入得到三角化的 TopoDS 组合，不恢复解析 CAD 曲面。

#### OBJ

- OBJ 接受正负位置、UV 和法线索引，支持独立的斜杠分隔角点字段、接缝展开和多边形扇形三角化。
- `importOBJ` 默认只返回形状，并在面三角化中保留完整法线和 UV。
- 传入 `includeDocument: true` 时，导入还返回索引缓冲区和文档。图元范围保留连续的 `o`、`g`、`usemtl` 和 `s` 状态；`indexStart` 与 `indexCount` 按标量索引条目计数。`materialLibraries` 保留全部 `mtllib` 引用。
- 匹配的 MTL `resources` 会填充环境色、漫反射色、镜面色、自发光色、不透明度、光泽度、光学密度、照明模型和支持的贴图引用。缺少配套资源不会阻止几何导入，也不会删除材质库引用。
- 网格形式的 `exportOBJ` 接受索引缓冲区和文档元数据，要求图元范围完整划分索引缓冲区，并返回 OBJ 数据及命名 MTL 资源。形状重载仍然可用。

#### PLY

- PLY 1.0 导入导出支持 ASCII、二进制小端和二进制大端编码。
- 形状路径保留顶点法线及常见 UV 属性：`s/t`、`u/v` 和 `texture_u/texture_v`。
- 传入 `includeMesh: true` 时，导入还返回索引位置、索引、法线、UV、u8 RGBA 顶点颜色，以及有序的 `comment`/`obj_info` 文件头。
- `exportPLY` 直接写出索引网格，拒绝越界或重复索引及几何退化三角形。三种编码的输出都可由对应的 `importPLY` 路径读取。

#### glTF 与 GLB

- glTF 操作支持 glTF 2.0 JSON 与 GLB、数据 URI 和 URI 匹配缓冲区、三角形列表/带/扇、节点矩阵或 TRS、归一化整数或浮点 `NORMAL`/`TEXCOORD_0`、稀疏访问器、变形目标、四影响蒙皮、动画采样，以及 OCCT Z-up 与 glTF Y-up 转换。
- `animationIndex` 和 `animationTime` 必须同时提供。节点 `translation`、`rotation`、`scale` 和 `weights` 在世界变换、变形求值和蒙皮前按 `STEP`、`LINEAR` 或 `CUBICSPLINE` 插值。线性旋转使用归一化最短路径四元数插值；范围外时间会钳制到端点。
- 无效的动画、采样器、访问器、通道、目标、插值、数量和有限数值约束会被拒绝。
- 显式 `morphWeights` 依次覆盖采样节点权重、静态节点权重和网格权重。蒙皮在变形求值后使用 `JOINTS_0`、`WEIGHTS_0`、关节世界变换和可选逆绑定矩阵。
- `importGLTF` 默认只返回形状。传入 `includeDocument: true` 时，还返回未修改的源 JSON、活动场景及根节点、求值后的 Z-up 缓冲区、图元实例范围、已解析的缓冲区和图像，以及可选形状。
- 文档导出保留标准元数据。JSON 使用配套资源保存外部缓冲区和图像；GLB 把缓冲区合并到 BIN 块，外部图像仍作为资源返回。
- TypeScript 导入层会在内核导入前解码 `KHR_draco_mesh_compression` 和 `EXT_meshopt_compression`。文档导入仍保留原始压缩 JSON 和源缓冲区。
- 未知的必需几何扩展会被拒绝。网格导入产生由三角形面组成的 TopoDS 组合，不恢复解析曲面。

#### VRML

- VRML 1/2 导入导出使用内存流，默认保留仅返回形状的接口。
- `importVRML` 传入 `includeMesh: true` 时，还返回索引位置、三角形索引、法线、可选 UV 和可选的 u8 RGBA 顶点颜色。
- `exportVRML` 接受相同的索引网格属性。VRML 只能存储 RGB，因此 RGBA 颜色必须不透明。形状导出保留三角化选项。

#### BREP 缓存

- BREP 是绑定当前 WASM 精确身份的缓存格式，不是跨版本交换格式。内核输出裸 BREP 字节，不嵌入也不验证该身份。
- 正式 standalone 运行时使用 `docs/g0-build.json` 中的 SHA-256。
- 正式 isolated Profile 使用 [`protocol/artifacts.json`](../protocol/artifacts.json) 中对应产物的 SHA-256。
- 正式 shared 运行时的身份同时包含同一清单中 `shared-main.wasm` 和 `step-shape-exchange.side.wasm` 的 SHA-256。
- 调用 `importBREP` 前必须拒绝身份不匹配的缓存。

### 操作约束

#### 圆角与倒角

- 边选择使用输入形状稳定边映射中从 0 开始的 `edgeIndices`。两个操作都保留兼容用的标量尺寸，也接受按边对齐的 `radii` 或 `distances` 数组。
- 圆角的 `radius2` 定义起点到终点的线性半径变化。`radiusLaw` 或对齐的 `radiusLaws` 定义多点半径变化规则，各相对参数必须严格递增并覆盖 0 到 1。
- 倒角双距离模式接受标量 `distance2` 或对齐的 `distances2`，并要求提供对齐的 `referenceFaceIndices`。每个参考面必须与对应边相邻，并指定第一段距离所在的一侧。
- 圆角支持解析面和 BSpline 面；算法失败返回 `FilletFailed`。

#### 操作历史

- `history` 只描述一次操作中的局部映射，不是持久拓扑命名。
- 布尔、圆柱孔、去特征、拉伸、旋转、放样、管道扫掠、pipe-shell 主脊线和轮廓、圆角/倒角、缝合、形状修复、同域统一，以及刚体、缩放、镜像和一般仿射变换提供完整映射。
- 辅助 pipe-shell 脊线没有 OCCT 历史映射，因此 `sweepPipeShell` 标记为 `partial`。
- 圆角/倒角历史报告保留的子形状、OCCT 从输入边生成的面、修改后的替换面和删除的输入；同一输入边可以同时出现在生成和删除分类中。
- `capabilities` 将没有可靠历史来源的操作标记为 `unsupported`。对这类操作请求 `includeHistory: true` 会返回 `InvalidArgs`。

#### 孔与去特征

`cylindricalHole` 使用显式轴，支持 `throughAll`、`throughNext`、`untilEnd`、`blind` 和 `between` 限制。盲孔的 `length` 以及 `between` 的 `from`/`to` 都是从 `origin` 沿轴测量的模型单位参数。`defeature` 接受实体、组合实体或实体组合中从 0 开始的面索引。两个操作都提供完整的局部面/边历史。

#### BSpline 与三角化编辑

- `editCurveBSpline` 和 `editSurfaceBSpline` 提供原生节点插入/删除、升阶、周期性和控制网编辑。
- `reduceCurveDegree` 和 `reduceSurfaceDegree` 使用有界 OCCT BSpline 近似，返回新形状及 OCCT 的 `maxError`，并保留源参数域和曲面边界拓扑。
- `triangulationData` 读取已有面网格，不重新三角化；`validateTriangulation` 检查文档规定的有限属性、索引和退化约束。
- `replaceTriangulation` 返回带调用方位置、索引、法线和 UV 的独立面副本。`repairTriangulation` 复制形状，使用 OCCT BRepMesh 重新网格化，并在重建后的面上计算法线。

#### 曲面填充

`makeSurfaceFilling.constraints` 按线框顺序使用从 0 开始的边界边索引，并使用面句柄表示 `g1` 或 `g2` 支持面约束。未指定的边采用 C0；支持面必须包含约束边的 OCCT pcurve。

#### 局部特征

- `localPrism` 默认为 `length` 模式。`until`、`fromUntil`、`untilEnd`、`fromEnd` 和 `thruAll` 直接映射到 `BRepFeat_MakePrism` 的对应限制操作；`until` 和 `from` 是受作用域管理的限制形状句柄。
- `localRevolution` 使用选定的基准面和显式轴。`angle`、`until`、`fromUntil` 和 `thruAll` 直接映射到 `BRepFeat_MakeRevol`；角度可为正或负，但绝对值不得超过一整圈。
- `linearForm` 映射到 `BRepFeat_MakeLinearForm`，使用轮廓线框、工作平面及一个或两个相向厚度向量构造平面筋或槽。`direction1` 必须为零向量或与 `direction` 相反；`operation` 选择融合（`add`）或切除（`cut`）。该原生形式没有长度或限制形状模式。
- `revolutionForm` 提供 `BRepFeat_MakeRevolutionForm` 的旋转筋/槽构造。它接受轮廓线框、平面、旋转轴和非负 `height1`/`height2`；至少一个高度必须非零。该操作没有限制形状模式。
- `glue` 通过 `BRepFeat_Gluer` 执行显式压印。`faceBindings` 配对 `newShape` 和 `baseShape` 中从 0 开始的面索引；可选 `edgeBindings` 配对边索引。至少需要一个面绑定，OCCT 还会在可能时匹配已绑定面中的边。

### 错误与取消

- VRML、STL、OBJ、PLY 和 glTF 导入导出以及边三角化会报告归一化进度。
- 网格交换会在解析、序列化和复制输出缓冲区时检查取消。
- 只有宿主适配器列为可协作取消的操作，才会在派发后观察 `AbortSignal`。
