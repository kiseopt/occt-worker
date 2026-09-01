# 变更记录

## 1.3.2 - 2026-09-02

### 变更

- 将所有 GitHub 官方 workflow Action 升级到当前主版本，移除已弃用的
  Node.js 20 Action 运行时。

## 1.3.1 - 2026-09-02

### 变更

- 扩充并重组中英文 API、协议、能力、宿主集成和运行时架构文档。
- 明确 standalone、isolated Profile 和 shared 运行时的 BREP 缓存身份要求，
  并同步生成的协议语义。
- 从公开文档和源码注释中移除过时的实施阶段术语。

## 1.3.0 - 2026-08-31

### 变更

- 发布用 WebAssembly 产物改为 `wasm-opt -O3`，不再使用 `--converge`。在 27 MB 产物上实测：
  收敛到不动点需 212 秒，而单趟 `-O3` 只需 34 秒，多出的 178 秒仅换来 288 字节；这一步在
  6 个大产物上各跑一次。发布产物相应增大约 0.001%。
- 发布标签改为提升由 main CI 针对同一提交构建出的不可变候选，不再从标签重新构建。只能为
  已经通过 main CI 的提交打标签。
- CI 将 standalone、shared、debug 三套 OCCT 拆为并行任务，分别缓存 OCCT install、ThinLTO
  codegen 和 Emscripten sysroot；浏览器测试改为矩阵并行，不再串行。
- Pull request 只构建 standalone 产物并运行精简测试矩阵。

### 新增

- 快速上手、API、协议、能力、宿主集成和行为准则文档的中文翻译，并与英文原文互相链接。
- `scripts/build-cache-key.mjs`、`scripts/write-candidate.mjs` 和
  `scripts/verify-candidate.mjs`，支撑缓存构建与候选提升流程。

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
