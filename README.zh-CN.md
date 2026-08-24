# occt-worker

面向 WebAssembly 的 OCCT 几何内核：窄消息协议、可复现工作流，优先支持浏览器和 Node。

本仓库独立开发，与 Open Cascade SAS 无关联、未获其背书，也不使用其商标。

## 状态

v1.0.0 冻结了已经实现的建模和查询能力：基于 OCCT 8.0.1 与 Emscripten 4.0.23 构建的 standalone reactor、同步消息 ABI、带 generation 校验的形状 arena 和作用域、基元与构造、拉伸/旋转/放样/扫掠、布尔和分割、General Fuse 单元选择与材料合并、glue/imprint、拔模、局部棱柱与旋转筋/槽、圆柱孔和去特征、圆角/倒角/抽壳/偏移/修复、变换、拓扑与质量查询、拓扑感知投影/极值/交运算、诊断、网格化，以及 BREP/STEP/IGES/STL/VRML/OBJ/PLY/glTF 交换、STEP XCAF 装配文档和格式探测。TypeScript 层还提供可序列化参数/表达式和支持原子重算的特征 DAG。公开操作集始终以 `capabilities` 响应为准。

OCCT 编译时关闭 `OCC_CONVERT_SIGNALS`，内核不安装信号处理器。这是为了避免固定 wasm-EH 工具链在 ShapeHealing/STEP 路径中生成无效 wasm。完整能力和限制见 [docs/capabilities.md](docs/capabilities.md)。

输出契约保持窄边界：网格顶点按面隔离，`faceGroups` 是 `(faceIndex, indexStart, indexCount)` 的 u32 三元组，`edgeGroups` 是 `(edgeIndex, vertexStart, vertexCount)` 的三元组。几何结果应使用容差比较；矩阵顺序、变换组合、惯量顺序、默认值和布局见 [docs/protocol.md](docs/protocol.md)。BREP 是绑定精确 wasm SHA-256 的缓存格式，不是跨版本交换格式。

主 wasm 已在 Node、Node Worker、Chromium、Firefox 和 WebKit Web Worker 中验证。Wasmtime 使用单独的 `wasm/occt-worker.wasmtime.wasm`，详见 [docs/hosts.md](docs/hosts.md)。

本项目是经过选择的粗粒度 CAD/OCCT 平台，不绑定全部非可视化 OCCT 类或 toolkit。可视化、pthread/TBB、通用 OCAF 编辑以及没有公开操作的 toolkit 不属于当前 v1。

## Node 快速开始

Node.js 20 或更高版本安装已发布的 npm 包：

```sh
npm install occt-worker
```

DirectClient 会显式加载发布版 wasm：

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

支持 `AsyncDisposable` 的运行时可以使用 `await using scope = await kernel.beginScope()`。

浏览器 Worker 用法见[浏览器示例](examples/browser)和[宿主支持](docs/hosts.zh-CN.md)。

## 构建

首次构建会下载固定版本的 CMake/Ninja，初始化 Emscripten 子模块，构建 OCCT，并生成 `wasm/occt-worker.wasm`：

```powershell
npm install
npm test
.\scripts\build-wasm.ps1
```

`npm run build:wasm:debug` 生成带断言的诊断产物 `build/debug/occt-worker.debug.wasm`，不进入 npm 包。Release 使用 `-O3` 和 `wasm-opt -O3`；由于固定工具链在圆角结果网格化时的实测问题，OCCT 全库编译期 LTO 被关闭。

项目采用静态链接并包含 OCCT 代码。npm 包带有 `npm-shrinkwrap.json`，可按 [SOURCE.md](SOURCE.md) 重建。许可证边界见 [LICENSES.md](LICENSES.md)、[NOTICE](NOTICE) 和 [SOURCE.md](SOURCE.md)。

## 项目文档

- [变更记录](CHANGELOG.zh-CN.md)
- [安全政策](SECURITY.zh-CN.md)
- [行为规范](CODE_OF_CONDUCT.md)
- [贡献指南](CONTRIBUTING.zh-CN.md)

## 文档

- [英文 README](README.md)
- [文档导航](docs/README.md)
- [中文 API 参考](docs/api.zh-CN.md)
- [中文协议规范](docs/protocol.zh-CN.md)
- [中文能力矩阵](docs/capabilities.zh-CN.md)
- [中文宿主支持](docs/hosts.zh-CN.md)
