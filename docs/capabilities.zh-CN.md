# 公共 CAD 平台能力矩阵

英文完整矩阵：[capabilities.md](capabilities.md)。本文件是中文发布说明；“已完成”表示已经进入协议和 TypeScript API、由 release wasm 执行并有代表性测试覆盖的能力，不表示绑定全部非可视化 OCCT 类。

## 内核与拓扑

已完成：generation 校验句柄、scope/buffer/batch、基元、线/面/壳/实体/compound/compsolid 构造、布尔、section、split、General Fuse 单元选择、glue/imprint、拉伸、旋转、放样、pipe sweep、draft、local prism/revolution、cylindrical hole、defeature、fillet、chamfer、hollow、offset、sew、ShapeFix、UnifySameDomain、ShapeUpgrade、刚体/缩放/镜像/仿射变换和变换阵列。

N-side filling、G1/G2 支持面约束、pcurve 曲面裁剪、线性/旋转筋槽和显式面/边绑定也已完成。

主要造型和修复操作支持局部 face/edge 历史，包括 retained、generated、modified 和 deleted。Pipe-shell 的辅助脊线没有 OCCT 可靠映射，因此该项为 `partial`；这不是被隐藏的完整支持。

## 曲线与曲面

已完成：line、arc、circle、ellipse、hyperbola、parabola、offset、Bezier、BSpline、helix；plane、cylinder、cone、sphere、torus、extrusion、revolution、ruled、offset、Bezier/BSpline surface。

已完成：参数域、解析几何、点/一阶/二阶导数、切向、曲率、连续性、控制点/节点/重数/权重读取，copy-on-write 编辑，节点插入删除，升阶/近似降阶，周期性，U/V 交换和反向，裁剪、扩展、重参数化、点集近似、fairing、N-side filling、投影、极值和拓扑裁剪交运算。

## 参数化 CAD

已完成：命名算术参数、依赖关系、特征 DAG、循环检查、schema v1 序列化、原子重算、失败回滚、suppressed 特征、诊断、几何签名和 OCCT 历史组合的子形引用。

已完成的参数特征包括基元、polygon/face、extrude/revolve/loft/pipe、fillet/chamfer/hollow/offset/draft、local feature、hole、defeature、repair、section、split、sew、transform、boolean、Bezier/BSpline 曲线曲面、2D sketch 和多 profile 草图，以及参数化装配实例、颜色、层、材料和验证属性。

Sketch 支持 line/circle/arc/BSpline、construction geometry、几何/尺寸约束和每约束诊断。BSpline 切向约束不属于当前 v1 有界求解器，非法组合会明确拒绝。

## 交换格式

已完成：

- BREP：绑定精确 wasm 哈希的缓存往返。
- STEP：shape-only、AP203/AP214/AP242、多根、单位和固定时间戳。
- STEP XCAF：装配树、实例变换、名称、颜色、层、材料、验证属性、面/边样式及支持的 views/SHUO/datum/GDT 字段。
- IGES：shape-only 和受限 XCAF 文档、多根、单位和内存流。
- STL：binary/ASCII 导入导出；可选网格数据、法线、solid name 和 binary header。
- OBJ：负索引、独立 UV/normal corner index、polygon 三角化、OBJ/MTL 文档和材质字段。
- PLY：ASCII、little-endian、big-endian；法线、UV、u8 颜色和 comments/obj_info。
- VRML 1/2：shape 和 indexed mesh、法线、UV、透明度受格式限制的颜色。
- glTF/GLB：场景层级、实例变换、材质/纹理/动画/skin/camera/extras、sparse accessor、morph、四影响骨骼、动画采样、Z-up/Y-up 转换，以及 Draco/meshopt 解码。
- 原生 XCAF binary/XML：装配和公共 PBR 字段，XML 使用受验证的元数据标记恢复固定驱动遗漏的记录。

不支持的语义不会静默丢弃：例如 IGES XCAF 的语义标注和 STEP 无法保留的链接会明确返回错误。

## 网格能力

已完成：面隔离三角网格、法线、UV、faceGroups、边折线和 edgeGroups；现有三角化读取、替换、验证和修复；weld、法线/切线生成、网格简化、三角细分、边界环提取、平面孔填充和拓扑选择映射。

网格处理会拒绝非有限属性、越界/重复索引、退化三角形、非流形或自交边界。当前不提供四面体/六面体体网格。

## 查询、运行时和边界

已完成：bbox/OBB、mass properties、distance、point classification、validity、详细诊断、容差检查/编辑、投影、极值、交运算、section analysis、middle path、exact HLR、进度、协作取消、共享 buffer、超时重建和 Node/Worker/浏览器/Wasmtime 宿主。

明确不属于当前 v1：TKV3d/AIS/TKOpenGl/TKMeshVS 可视化、pthread/TBB、字体文本、通用 OCAF 编辑、完整 TKXMesh/TKExpress API、全部非可视化 OCCT 绑定，以及方案未列出的 DXF/DWG、Parasolid、ACIS/SAT、JT、3MF、USD、FBX、Collada 等格式。
