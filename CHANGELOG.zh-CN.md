# 变更记录

## 1.2.0 - 2026-08-27

### 新增

- 模块化共享 Main/Side WebAssembly 产物和隔离运行时配置。
- 类型化几何引擎、产物解析和运行时兼容性检查。
- 扩展建模、拓扑、网格处理及 CAD/网格格式交换操作。

### 变更

- 公共协议和发布产物清单更新到 1.2.0。
- 按操作领域拆分内核源码和 TypeScript 类型，明确代码归属。

## 1.0.0 - 2026-08-24

首次公开发布。

### 新增

- 版本化消息协议，以及面向 Node 和浏览器 Worker 的 TypeScript 客户端。
- 经过选择的 OCCT 建模、拓扑、查询、网格化和格式交换操作。
- 可序列化参数特征、草图约束、子形引用和装配定义。
- 面向 Node/浏览器宿主的发布版 WebAssembly 产物，以及单独转换的 Wasmtime 产物。
- 可复现源码标识、协议生成证据、校验和及对应源码发布资产。

### 兼容性

- npm 包要求 Node.js `>=20`。
- CI 覆盖桌面 Chromium、Firefox 和 WebKit Worker 环境。
- Wasmtime 使用独立发布产物，CI 使用 Wasmtime 47.0.3。

### 已知限制

可视化、pthread/TBB 执行、通用 OCAF 编辑、移动浏览器内存限制，以及没有公开
操作的 OCCT toolkit 不属于 v1 能力范围。详见[能力矩阵](docs/capabilities.zh-CN.md)
和[宿主支持](docs/hosts.zh-CN.md)。
