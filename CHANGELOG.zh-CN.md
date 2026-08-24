# 变更记录

## 1.0.1 - 2026-08-24

### 修复

- 将 Windows 引导脚本和发布工作流中不可用的 `Get-FileHash` 替换为 .NET SHA-256 计算。
- 将 GitHub 官方 Actions 升级到基于 Node.js 24 的版本。

### 变更

- 重新构建发布版 wasm，并更新记录的文件大小和 SHA-256 证据。
- 记录编译期间出现的非致命 OCCT 8.0.1 弃用警告。

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
