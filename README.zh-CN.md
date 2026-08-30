# occt-worker

无需安装原生 CAD 环境，即可在 Node.js 和浏览器 Worker 中运行 OCCT 建模与几何操作。

本 WebAssembly 几何内核使用版本化消息协议，并提供面向 Node.js 和浏览器 Worker 的
TypeScript 客户端。

[![CI](https://github.com/kiseopt/occt-worker/actions/workflows/ci.yml/badge.svg)](https://github.com/kiseopt/occt-worker/actions/workflows/ci.yml)
[![npm 版本](https://img.shields.io/npm/v/occt-worker.svg)](https://www.npmjs.com/package/occt-worker)
[![Node.js](https://img.shields.io/node/v/occt-worker.svg)](https://www.npmjs.com/package/occt-worker)
[![许可证](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE-APACHE-2.0.txt)

[English README](README.md) · [文档导航](docs/README.md) · [使用指南](docs/getting-started.zh-CN.md) · [API 参考](docs/api.zh-CN.md) · [能力矩阵](docs/capabilities.zh-CN.md)

**当前版本：** `v1.2.0` · **运行时：** Node.js `>=20` · **许可证：** 项目代码采用
Apache-2.0；OCCT 和其他第三方组件请以分发通知为准。

本项目独立开发，与 Open Cascade SAS 无关联、未获其背书或赞助，也不使用 Open
Cascade SAS 商标。

## 核心能力

- **建模：** 基元、线框、面、实体、扫掠、放样、旋转、布尔、分割、局部特征、圆角、
  倒角、抽壳、偏移、修复和变换。
- **几何与拓扑：** 质量属性、包围盒、拓扑遍历、分类、距离、交运算、投影、极值、
  曲线和曲面查询。
- **CAD 交换：** BREP、STEP、IGES、STL、VRML、OBJ、PLY、glTF/GLB 以及 XCAF 文档
  的导入导出，并提供格式探测和明确的元数据行为。
- **网格处理：** 可复现网格化、边网格化、三角化检查与修复、索引网格导入导出，以及
  支持进度的取消。
- **应用层：** TypeScript 包提供可序列化参数、表达式、特征 DAG、草图、子形状引用和
  装配定义。

公开操作集以 `capabilities` 响应为准。本项目是经过选择的粗粒度 API，不是对全部非
可视化 OCCT 类或 toolkit 的逐一绑定。

## 安装

要求 Node.js 20 或更高版本：

```sh
npm install occt-worker
```

> **安装体积与首次下载体积不同。** npm 包包含完整 full WASM、运行时、类型和协议
> manifest；安装时不会按功能懒加载。浏览器首次使用时，shared Side/Profile 产物可通过
> `resolveArtifact({ name })` 从 Release/CDN 懒加载，也可以用 `baseUrl` 或自定义 resolver
> 指向私有镜像。

首次使用请先阅读[使用指南](docs/getting-started.zh-CN.md)，其中按 Node.js、Worker、
浏览器和源码构建分别给出运行路径。

## 快速开始

`DirectClient` 显式加载发布版 WebAssembly，并通过作用域管理形状句柄：

```js
import { readFile } from "node:fs/promises";
import { DirectClient } from "occt-worker";

const wasm = await readFile(new URL(import.meta.resolve("occt-worker/wasm")));
const kernel = await DirectClient.create(wasm);
const scope = await kernel.beginScope();

const plate = await scope.makeBox([100, 60, 4]);
const hole = await scope.makeCylinder(8, 4, { origin: [50, 30, 0] });
const result = await scope.booleanCut(plate, [hole]);

console.log(await kernel.bbox(result.shape));
await scope.end();
```

示例会创建一个 100 x 60 x 4 的板件，在中心切出圆柱孔，并打印结果的包围盒数据。

支持 `AsyncDisposable` 的运行时可以使用
`await using scope = await kernel.beginScope()`。浏览器 Worker 用法请从[浏览器示例](examples/browser)
开始，并参阅[宿主支持](docs/hosts.zh-CN.md)。

## API 速览

| 使用场景 | 公开入口 | 参考文档 |
| --- | --- | --- |
| Node.js 进程内核 | `DirectClient` 和 `ShapeScope` | [TypeScript API](docs/api.zh-CN.md) |
| Node.js 或浏览器 Worker 内核 | `WorkerClient` | [TypeScript API](docs/api.zh-CN.md) · [浏览器示例](examples/browser) |
| 建模、查询、网格和交换操作 | `ShapeScope` 方法 | [能力矩阵](docs/capabilities.zh-CN.md) |
| 底层消息、缓冲区、错误和默认值 | `request()` 与协议帧 | [协议规范](docs/protocol.zh-CN.md) |
| Wasmtime 或自定义宿主集成 | 同步 WebAssembly ABI | [宿主支持](docs/hosts.zh-CN.md) |
| 参数化特征和草图 | `ParametricModel` 与特征定义 | [TypeScript API](docs/api.zh-CN.md) |
| shared Main/Side 高层兼容 | `SharedClient` + `EngineCompatClient` | [TypeScript API](docs/api.zh-CN.md) |

使用 isolated Profile 时，从 `protocol/artifacts.json` 读取 descriptor，并向
`createWorkerProfileRuntime` 提供 Worker 工厂；该 helper 会在启动 Worker 前校验产物，随后
把返回的 factory 注册到 `GeometryEngine`，以便按首次使用懒启动 Profile。shared Side/Profile
二进制不在 npm tarball 中，而是在版本 tag 的 GitHub Release 中由 release workflow 自动构建并上传；
`protocol/artifacts.json` 同步记录对应版本 URL 和 SHA-256。使用 `SharedClient` 时传入 Main/Side
构件即可通过旧的 `BaseClient`/`EngineCompatClient` 调用面运行；私有部署仍可配置 `baseUrl`、
`defaultBase` 或 `resolve`，不需要项目手工维护 CDN。

## 支持矩阵

| 环境或功能 | 发布范围 |
| --- | --- |
| Node.js | `>=20`；覆盖 DirectClient、WorkerClient 和 npm 消费者测试 |
| 桌面浏览器 Worker | 发布测试矩阵覆盖 Chromium、Firefox 和 WebKit |
| Node `worker_threads` | 使用同一个主产物 `wasm/occt-worker.wasm` |
| Wasmtime | 使用独立的 `wasm/occt-worker.wasmtime.wasm`；CI 使用 Wasmtime 47.0.3 |
| 共享内存 | `SharedArrayBuffer` 能力要求浏览器启用跨源隔离 |
| 移动浏览器和 WebView | 不属于发布认证矩阵 |
| 可视化和并行 OCCT toolkit | 可视化、pthread/TBB 执行和通用 OCAF 编辑不属于 v1 |
| BREP | 仅作缓存格式；每个缓存必须绑定 `docs/g0-build.json` 中的精确 wasm SHA-256 |

完整操作集、默认值、缓冲区布局、取消规则和宿主细节见[协议规范](docs/protocol.zh-CN.md)、
[能力矩阵](docs/capabilities.zh-CN.md)和[宿主支持](docs/hosts.zh-CN.md)。

## 已知问题与限制

- 通过 `WorkerClient` 取消正在执行的同步操作会触发硬恢复：Worker 被重建，旧 Worker
  拥有的所有句柄都会失效。
- `SharedArrayBuffer` 传输仍需在 WebAssembly 线性内存和宿主内存之间复制一次，并非零拷贝。
- BREP 是缓存格式，不是可移植交换格式。运行中的 wasm SHA-256 与记录值不一致时必须拒绝缓存。
- 移动浏览器和 WebView（包括其内存上限）不属于发布测试矩阵。
- 可视化、pthread/TBB 执行和通用 OCAF 编辑不属于 v1 API。
- STEP/IGES/XCAF 元数据支持遵循各格式的文档化子集；格式无法保存的数据会被拒绝，或按文档化
  扩展路径表示。

报告可复现问题时，请附上输入格式、宿主运行时、版本和脱敏的最小复现。漏洞请走[安全政策](SECURITY.zh-CN.md)
中的私密渠道，普通 Bug 和功能请求请提交到 [Issues](https://github.com/kiseopt/occt-worker/issues)。

## 从源码构建

使用固定子模块克隆仓库，然后构建 WebAssembly 产物：

```powershell
git clone --recurse-submodules https://github.com/kiseopt/occt-worker.git
cd occt-worker
npm ci
.\scripts\build-wasm.ps1
npm test
```

构建脚本会引导固定版本的 CMake、Ninja 和 Emscripten SDK，并生成
`wasm/occt-worker.wasm`。常用检查命令：

```powershell
npm run build:wasm:debug
npm run test:browser:all
npm run test:wasmtime -- -WasmtimePath C:\path\to\wasmtime.exe
```

Release 构建、协议生成文件和 wasm 哈希由仓库脚本及 CI 校验。不要直接编辑协议生成
结果；应先修改 `protocol/` 下的权威文件。

## 发布与对应源码

发布的 npm 包包含 TypeScript 构建产物、协议元数据、文档和 WebAssembly 运行时产物。
每个二进制发布都必须附带 [SOURCE.zh-CN.md](SOURCE.zh-CN.md) 所述的对应源码与重新链接
材料，以及 [LICENSES.zh-CN.md](LICENSES.zh-CN.md)、[NOTICE.zh-CN.md](NOTICE.zh-CN.md)
和 `THIRD-PARTY-NOTICES.txt` 中的通知。

## 文档

- [文档导航](docs/README.md)
- [使用指南](docs/getting-started.zh-CN.md)
- [TypeScript API](docs/api.zh-CN.md)
- [协议规范](docs/protocol.zh-CN.md)
- [能力矩阵](docs/capabilities.zh-CN.md)
- [宿主支持与 Wasmtime](docs/hosts.zh-CN.md)
- [变更记录](CHANGELOG.zh-CN.md)

## 社区

- [报告 Bug 或请求功能](https://github.com/kiseopt/occt-worker/issues)
- [贡献指南](CONTRIBUTING.zh-CN.md)
- [安全政策](SECURITY.zh-CN.md)
- [行为规范](CODE_OF_CONDUCT.zh-CN.md)
- [发布版本](https://github.com/kiseopt/occt-worker/releases)

## 许可证

项目代码和文档采用 [Apache-2.0](LICENSE-APACHE-2.0.txt) 许可。分发内容还包括采用
LGPL-2.1 及 Open CASCADE exception 的 OCCT、Emscripten 运行时/系统代码、nlohmann/json、
Draco 和 meshoptimizer。适用通知和再分发边界见 [LICENSES.zh-CN.md](LICENSES.zh-CN.md)
及 `THIRD-PARTY-NOTICES.txt`。
