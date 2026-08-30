# 使用指南

[English reference / 英文规范](getting-started.md)

本页负责首次使用和常见工作流：选择运行入口、运行已发布内核、从源码构建、
理解生成代码的边界，以及处理常见错误。API 的完整参数和协议字段不在本页重复；
请使用 [完整 TypeScript API](api.zh-CN.md)。

## 先选择运行方式

| 你的场景 | 选择 | 需要做什么 |
| --- | --- | --- |
| Node.js 脚本、服务或批处理 | `DirectClient` | 读取完整 WASM，创建客户端和作用域，调用 `ShapeScope`。 |
| 不希望阻塞 Node.js 或浏览器主线程 | `WorkerClient` | 提供 Worker 工厂和 WASM；操作通过消息队列执行。 |
| 按功能加载较小的内核模块 | `SharedClient` 或 `GeometryEngine` | 使用发布清单解析 Main、Side 或 isolated profile 构件。 |
| 参数化零件和装配 | `ParametricModel` | 在客户端上管理参数、特征 DAG、重算和子形状引用。 |
| 自定义宿主或 Wasmtime | 同步 WASM ABI | 按 [宿主支持](hosts.zh-CN.md) 提供导入并调用协议 ABI。 |

如果只是验证项目能否工作，先运行 Node.js 示例即可，不需要重新编译 OCCT：

```powershell
node examples/node-batch.mjs
```

## 安装并运行

使用已发布的 npm 包：

```powershell
npm install occt-worker
```

最小 Node.js 程序：

```js
import { readFile } from "node:fs/promises";
import { DirectClient } from "occt-worker";

const wasm = await readFile(new URL(import.meta.resolve("occt-worker/wasm")));
const kernel = await DirectClient.create(wasm);
const scope = await kernel.beginScope();

const base = await scope.makeBox([100, 60, 4]);
const tool = await scope.makeCylinder(8, 4, { origin: [50, 30, 0] });
const cut = await scope.booleanCut(base, [tool]);

console.log(await kernel.bbox(cut.shape));
await scope.end();
```

Node.js 要求以 `package.json` 的 `engines` 字段为准。形状属于创建它的作用域和
客户端；结束作用域会释放其中的形状，Worker 重建后旧句柄也会失效。

## 使用浏览器 Worker

浏览器使用 `WorkerClient` 和项目提供的 Worker 入口：

```js
import { WorkerClient } from "../../dist/worker-client.js";

const wasm = await (await fetch("../../wasm/occt-worker.wasm")).arrayBuffer();
const factory = () => new Worker(
  new URL("../../dist/worker-entry.js", import.meta.url),
  { type: "module" },
);
const kernel = await WorkerClient.create(factory, wasm);
const scope = await kernel.beginScope();
const box = await scope.makeBox([10, 20, 30]);

console.log(await kernel.massProps(box));
await scope.end();
kernel.close();
```

浏览器示例位于 [`examples/browser`](../examples/browser)。浏览器必须通过 HTTP(S)
加载 ES 模块、Worker 和 WASM，不能依赖 `file://` 直接打开 HTML。使用
`SharedArrayBuffer` 的共享输入、共享输出或协作取消时，还需要跨源隔离响应头；仓库
的浏览器测试会设置 `Cross-Origin-Opener-Policy: same-origin` 和
`Cross-Origin-Embedder-Policy: require-corp`。

Node.js `worker_threads` 使用同一个 `WorkerClient`，但需要一个把 `parentPort`
消息转换为 `WorkerLike` 的宿主适配器。仓库的
[`tests/ts/node-worker.mjs`](../tests/ts/node-worker.mjs) 是该适配器的参考；
浏览器专用的 `dist/worker-entry.js` 不能直接作为 Node Worker 入口。

## 发现可用操作

运行时的 `capabilities` 响应是当前内核实际公开能力的最终依据：

```js
const capabilities = await kernel.initialize();
console.log(capabilities.ops);
```

`OPERATIONS`、`HISTORY_SUPPORT` 等常量由协议定义生成，适合做静态类型和路由；
运行时能力仍应以 `capabilities` 返回值为准。完整操作契约位于：

- [`protocol/operations.json`](../protocol/operations.json)
- [`protocol/operation-contracts.json`](../protocol/operation-contracts.json)
- [`protocol/modules.json`](../protocol/modules.json)

## 从源码构建

源码构建用于修改内核、协议或 TypeScript 客户端。首次获取仓库时保留固定子模块：

```powershell
git clone --recurse-submodules https://github.com/kiseopt/occt-worker.git
cd occt-worker
npm ci
```

只修改协议或 TypeScript：

```powershell
npm run build
```

修改 OCCT C++、内核处理器或构建配置：

```powershell
npm run build:wasm
```

脚本会准备固定版本的 CMake、Ninja 和 Emscripten SDK，编译 OCCT 静态库，再编译
项目的 WASM 内核。需要调试产物时使用：

```powershell
npm run build:wasm:debug
```

完成构建后，执行与改动相关的检查：

```powershell
npm test
npm run test:browser
```

构建脚本要求 `occt` 子模块工作树干净。不要直接编辑协议生成文件；先修改
`protocol/` 下的权威定义，再执行 `npm run generate`。新的 C++ 代码必须接入
`kernel/` 的处理器和 CMake 构建图，不能只把 `.cpp` 文件放进仓库。

## 生成代码的运行边界

| 生成结果 | 能否直接运行 | 处理方式 |
| --- | --- | --- |
| JavaScript API 调用 | 可以 | 安装依赖并加载发布 WASM，直接使用 `DirectClient` 或 `WorkerClient`。 |
| TypeScript API 调用 | 可以 | 先由 TypeScript 工具链编译，再按 JavaScript 方式运行。 |
| 协议 JSON 或 `batch` 序列 | 可以 | 通过 `request()` 或 `batch()` 交给已有 WASM 内核。 |
| 新的 OCCT C++ 源码 | 不可以 | 编译进完整内核或 Side WASM，注册协议操作后再调用。 |

这个项目执行的是已编译的 WebAssembly，不是 C++ 源码解释器。若生成器每次都输出
新的 C++，就需要每次重新构建；若希望不重复编译，应让生成器输出 API 调用、参数
或协议 JSON，把算法固定在已经编译的内核中。

## 资源生命周期

- 每次调用 `beginScope()` 都应对应一次 `scope.end()`。
- 不再需要单个形状时可调用 `kernel.release(shape)`；批量清理使用 `releaseAll()`。
- 二进制导入导出通过内核 buffer 完成；拿到 `DirectBuffer` 后，每次 WASM 调用后都应
  重新获取 `view()`。
- Worker 超时或活动中的非协作操作取消时会重建 Worker，之前的形状句柄不能复用。
- `stats()` 可查看 `liveShapeHandles`、`liveBufferBytes` 和 WASM 内存使用情况。

## 常见问题

| 现象 | 原因 | 处理方式 |
| --- | --- | --- |
| `Cannot find module` 或没有 `dist` | 从源码运行但尚未生成 TypeScript 输出 | 执行 `npm run build`，或从已发布包导入。 |
| `Unknown operation` | 客户端、协议和 WASM 不是同一套生成结果 | 同步修改 `protocol/`，执行 `npm run generate`，再重新构建 WASM。 |
| `BuildFamilyMismatch` 或 Side 加载失败 | Main 与 Side 构件来自不同构建族或版本 | 从同一份 `protocol/artifacts.json` 解析构件，不要混用不同发布版本。 |
| Worker 重建后 `InvalidHandle` | 旧句柄属于已经失效的 Worker epoch | 重新创建作用域和形状，不要缓存跨 Worker 的句柄。 |
| 浏览器中 WASM 或 Worker 加载失败 | 使用了 `file://` 或静态服务器没有正确 MIME/响应头 | 通过 HTTP(S) 提供文件；共享内存功能同时启用跨源隔离。 |
| 新 C++ 代码链接失败 | 使用了当前构建未启用的 toolkit，或未加入 CMake 源文件列表 | 检查 `scripts/build-config.ps1`、`kernel/CMakeLists.txt` 和目标 toolkit。 |

## 下一步阅读

- [完整 TypeScript API](api.zh-CN.md)：按类别查找入口和能力范围。
- [完整 TypeScript API](api.zh-CN.md)：查看方法、类型、错误和交换格式细节。
- [协议规范](protocol.zh-CN.md)：查看消息帧、句柄、buffer 和确定性规则。
- [能力矩阵](capabilities.zh-CN.md)：查看已实现能力与明确限制。
- [宿主支持](hosts.zh-CN.md)：查看浏览器、Worker、Node.js 和 Wasmtime 边界。
- [浏览器示例](../examples/browser)：查看 Worker、共享 buffer 和网格输出的完整流程。
