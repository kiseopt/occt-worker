# 宿主支持

[English reference / 英文规范](hosts.md)

运行时组成、Profile 边界、形状所有权和内存术语由[运行时架构](architecture.zh-CN.md)定义。
本页负责宿主支持范围和集成要求。

## 已验证宿主

发布产物 `wasm/occt-worker.wasm` 已在 Node.js、Node `worker_threads` 以及
Chromium、Firefox 和 WebKit Web Worker 中验证。这些宿主使用同一个 SHA-256 产物，
每个实例只调用一次 `_initialize`。必需的导入和导出面冻结在
`protocol/wasm-surface.json` 中，`scripts/verify-wasm.mjs` 会拒绝任何漂移。

浏览器矩阵使用桌面 Playwright 引擎，不代表 iOS Safari、Android WebView 或移动端
内存上限已经认证；发布说明若要声明真机结果，必须单独列出对应结果。

## 版本与构建矩阵

| 环境 | 声明或固定版本 | 覆盖范围 |
| --- | --- | --- |
| Node.js | `>=20`；CI 使用 Node 22 | DirectClient、WorkerClient、npm 消费者和 wasm 校验 |
| Chromium、Firefox、WebKit | Playwright 1.62.1 安装的桌面引擎 | 浏览器 Worker 和打包 npm 消费者 smoke 测试 |
| Wasmtime | CI 使用 47.0.3 | 独立转换产物和协议 smoke 测试 |
| 源码构建宿主 | CI 使用 `windows-latest`；构建脚本为 PowerShell | 发布 wasm 重建和生成文件校验 |

npm 包运行时目标为 Node.js 20 及以上，以及上表列出的桌面浏览器 Worker 环境。
除非发布说明另有声明，其他操作系统和浏览器版本不属于发布认证范围。在其他平台
进行源码构建可能需要调整 PowerShell 工具链引导和构建命令。

## Wasmtime

### 产物转换

Wasmtime 使用独立的 `wasm/occt-worker.wasmtime.wasm`。Emscripten 4.0.23 在 `-fwasm-exceptions` 下生成 Phase 3 WebAssembly 异常处理编码；当前 Wasmtime 会对此报错：`legacy_exceptions feature required for try instruction`。

`npm run build:wasm:wasmtime` 使用 Binaryen 把已构建模块转换为标准 `try_table`/`exnref` 编码，同时保留 C++ 异常行为。该命令不会覆盖 `wasm/occt-worker.wasm`，因为 Node.js 和受支持浏览器尚未全部接受这种异常引用形式。

### 验证与嵌入

运行 `npm run test:wasmtime -- -WasmtimePath /path/to/wasmtime` 可验证备用产物。测试预加载确定性的 `env` 替代实现，由 Wasmtime 提供 WASI Preview 1，并通过 `k_handle` 覆盖作用域、基元、布尔建模、包围盒、三角化、结构化错误和资源释放。

CI 和发布流程使用 `scripts/bootstrap-wasmtime.ps1` 安装固定的 Wasmtime 版本，重建转换产物，运行冒烟测试，并发布带校验和的文件。发布证据记录在 `docs/wasmtime-build.json`。

嵌入方必须提供 `protocol/wasm-surface.json` 中固定的导入，只调用一次 `_initialize`，再使用与浏览器和 Node.js 宿主相同的消息 ABI。

### 确定性导入

允许使用的 WASI `fd_*`、环境和时钟导入不会向协议暴露文件系统或实际时钟。`DirectClient` 提供确定性替代实现：写入会被丢弃但仍报告已消费的字节数，读取和环境枚举为空，时钟值从固定纪元开始递增。

非 WASI 的 `env` 导入是 Emscripten/OCCT 运行时钩子，由浏览器和 Node.js 适配器实现。`occt_worker_progress` 与 `occt_worker_cancelled` 将受支持的 OCCT 算法连接到宿主进度和取消状态。

## Worker、取消和共享内存

### 协作取消与硬恢复

`WorkerClient.request()` 支持 `{ signal, onProgress }`。排队中的调用会被直接移除而不影响当前活动 Worker。

- **协作取消：** 形状与边三角化、VRML 与网格（STL/OBJ/PLY/glTF）交换，以及 STEP/IGES 的形状和文档导入导出使用 `SharedArrayBuffer` 进行协作取消。OCCT 确认取消后，Worker、形状句柄和后续排队请求仍然有效。网格交换会在解析、序列化和复制输出缓冲区时检查取消标志。
- **硬恢复：** 其他正在执行的同步调用，或在不支持 `SharedArrayBuffer` 的宿主中，取消会触发硬恢复：终止并重建 Worker，此前创建的所有旧句柄及排队调用均会失效。

### SharedArrayBuffer 与跨源隔离

批量数据的二进制导入接受 `SharedArrayBuffer`；与普通 `ArrayBuffer` 不同，共享输入跨 Worker 边界时不会被 transfer 或 detach。

`WorkerClient.requestShared(op, args, options)` 会为低层协议结果中的每个输出缓冲区描述符分配一个 `SharedArrayBuffer`，再从 WebAssembly 线性内存复制数据。共享缓冲区跨 Worker 边界时不会被克隆或分离，但该路径仍会向宿主内存复制一次。

浏览器使用批量共享缓冲和协作取消都需要配置跨源隔离响应头：

```http
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

### Node.js `worker_threads` 集成

`WorkerClient` 依赖与浏览器 Web Worker 兼容的 `WorkerLike` 接口。在 Node.js `worker_threads` 中承载 `occt-worker` 时，宿主适配器需实现以下接口：

```ts
export interface WorkerLike {
  postMessage(message: unknown, transfer?: Transferable[]): void;
  addEventListener(type: "message", listener: (event: MessageEvent) => void): void;
  addEventListener(type: "error", listener: (event: ErrorEvent) => void): void;
  terminate(): void;
}
```

### 进程隔离与移动端限制

Dedicated Worker 隔离允许主页面捕获并处理 JavaScript 可见的软失败（例如 `abort()` 和 `RangeError`）。但它不能隔离操作系统级的进程内存强杀：例如在 iOS WebKit 中，Worker 与页面运行在同一个 WebContent 进程内，Jetsam 内存压力会直接终止整个进程。

`ArtifactLoadAttemptTracker` 为完整 profile 加载提供启发式状态跟踪。将它作为 `loadAttempt` 传给 `createWorkerProfileRuntime` 后，加载流程会记录 `fetching`、`compiling`、`instantiating` 和 `ready`。应用重新载入后可通过 `unfinished()` 检测上次加载是否未完成。

### 浏览器中的产物缓存

在浏览器环境下，默认 isolated Profile loader 会通过 Cache API 将带声明 SHA-256 的正式 artifact 缓存为原始 WASM 字节。缓存键包含 artifact 名称和哈希；每次命中仍会重新校验哈希，失效字节会被清理并重新下载。自定义 `loadArtifact` 回调和 shared Main/Side loader 不使用该缓存。编译后的 `WebAssembly.Module` 对象不会被持久化。
