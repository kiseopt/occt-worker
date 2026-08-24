# 宿主支持

英文规范：[hosts.md](hosts.md)。

## 已验证宿主

- Node 与 Node `worker_threads`：使用同一个 `wasm/occt-worker.wasm`，每个实例只调用一次 `_initialize`。
- Chromium、Firefox、WebKit Web Worker：使用与 Node 相同的主 wasm SHA-256。
- Wasmtime 47.0.3：使用独立的 `wasm/occt-worker.wasmtime.wasm`。该文件由 Binaryen 将 Emscripten 4.0.23 的旧 wasm-EH 编码转换为标准 `try_table`/`exnref` 编码，不覆盖浏览器/Node 主产物。

桌面 Playwright 矩阵不代表 iOS Safari、Android WebView 或移动端内存上限认证。

## 版本与构建矩阵

| 环境 | 声明或固定版本 | 覆盖范围 |
| --- | --- | --- |
| Node.js | `>=20`；CI 使用 Node 22 | DirectClient、WorkerClient、npm 消费者和 wasm 校验 |
| Chromium、Firefox、WebKit | Playwright 1.62.1 安装的桌面引擎 | 浏览器 Worker 和打包 npm 消费者 smoke |
| Wasmtime | CI 使用 47.0.3 | 独立转换产物和协议 smoke |
| 源码构建宿主 | CI 使用 `windows-latest`；构建脚本为 PowerShell | 发布 wasm 重建和生成文件校验 |

npm 包运行时目标为 Node.js 20 及以上，以及上表列出的桌面浏览器 Worker 环境。
除非发布说明另有声明，其他操作系统和浏览器版本不属于发布认证范围。在其他平台
进行源码构建可能需要调整 PowerShell 工具链引导和构建命令。

## Wasmtime

```powershell
npm run build:wasm:wasmtime
npm run test:wasmtime -- -WasmtimePath /path/to/wasmtime
```

smoke 会预加载确定性的 `env` stub，使用 Wasmtime 提供 WASI Preview 1，并通过同一 ABI 验证初始化、作用域、基元、布尔、包围盒、网格、结构化错误和资源释放。证据写入 `docs/wasmtime-build.json`。

宿主必须提供 `protocol/wasm-surface.json` 中的导入，并且只调用一次 `_initialize`。协议文件系统和墙上时钟不会暴露给操作；DirectClient 提供确定性的 stub。

## Worker、取消与共享内存

`WorkerClient.request()` 支持 `{ signal, onProgress }`。排队请求可以直接移除；支持进度检查点的 STEP/IGES、VRML、网格和网格化操作使用一字 `SharedArrayBuffer` 协作取消，并保留 worker 和现有句柄。其他正在执行的同步操作采用终止并重建 worker 的硬取消，旧句柄同时失效。

共享输入不会 detach，`requestShared()` 可把输出物化为 `SharedArrayBuffer`。即使使用共享内存，wasm 线性内存到宿主共享缓冲之间仍有一次必要复制。浏览器使用这些能力需要 cross-origin isolation。
