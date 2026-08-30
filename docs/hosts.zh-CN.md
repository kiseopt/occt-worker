# 宿主支持

[English reference / 英文规范](hosts.md)

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

Wasmtime 使用独立的 `wasm/occt-worker.wasmtime.wasm` 产物。Emscripten 4.0.23
在 `-fwasm-exceptions` 下生成 Phase 3 wasm 异常处理形式；当前 Wasmtime 对这种
编码报 `legacy_exceptions feature required for try instruction`。命令
`npm run build:wasm:wasmtime` 保留 C++ 异常行为，并通过 Binaryen 将已构建模块
转换为标准 `try_table`/`exnref` 编码；它不会覆盖 Node 和受支持浏览器使用的主产物。

运行 `npm run test:wasmtime -- -WasmtimePath /path/to/wasmtime` 可验证备用产物。
测试会预加载确定性的 `env` stub，由 Wasmtime 提供 WASI Preview 1，并通过冻结 ABI
验证 reactor 初始化、作用域、基元、布尔建模、包围盒查询、三角化、结构化错误和
通过 `k_handle` 的资源释放。CI 和发布流程使用
`scripts/bootstrap-wasmtime.ps1` 安装固定的 Wasmtime 47.0.3，重建转换产物，
运行 smoke，并发布带校验和的文件；证据写入 `docs/wasmtime-build.json`。嵌入方
必须提供 `protocol/wasm-surface.json` 中冻结的导入，只调用一次 `_initialize`，
然后使用与浏览器和 Node 宿主相同的消息 ABI。

白名单 WASI `fd_*`、环境和时钟导入不会暴露协议文件系统或墙上时钟行为。
DirectClient 提供确定性 stub：写入被丢弃但报告已消费的字节数，读取和环境枚举为空，
时钟从固定纪元推进。非 WASI 的 `env` 导入是 Emscripten/OCCT 运行时钩子，同样
由浏览器/Node 适配器实现；`occt_worker_progress` 和
`occt_worker_cancelled` 将支持的 OCCT 算法连接到宿主进度和取消状态。

`WorkerClient.request()` 支持 `{ signal, onProgress }`。排队调用会被移除而不影响
活动 Worker。三角化、VRML、网格（STL/OBJ/PLY/glTF）以及 shape-only STEP/IGES 导入导出
使用 `SharedArrayBuffer` 协作取消；当 OCCT 确认中断时，Worker、句柄和后续队列
都会保留。网格交换在解析、序列化和 buffer 物化阶段检查取消。宿主必须提供
`SharedArrayBuffer`；浏览器因此需要跨源隔离。其他正在执行的同步调用，以及不支持
共享内存的调用，继续使用硬取消：终止并重建 Worker，并使旧句柄和排队调用失效。

Dedicated Worker 隔离允许主页面报告 JavaScript 可见的软失败，例如 `abort()` 和
`RangeError`，但不能保护 iOS 页面免受内存不足终止：WebKit 的 Worker 与页面
运行在同一个 WebContent 进程内，Jetsam 会终止整个进程。页面重新载入后，未完成尝试
标记只能说明上次尝试没有完成，不能证明原因。

`ArtifactLoadAttemptTracker` 为完整 profile 加载提供这项启发式记录。将它作为
`loadAttempt` 传给 `createWorkerProfileRuntime` 后，加载流程会记录
`fetching`、`compiling`、`instantiating` 和 `ready`；
加载成功或进入可捕获失败时，会清除活动标记并另存结果。应用重新载入后可用
`unfinished()` 提示上次完整功能加载未完成，但不能断言原因；真机测量页的
内存候选也复用同一记录器。

浏览器会通过 Cache API 缓存带声明 SHA-256 的正式 wasm 字节。缓存键包含 artifact 名称
和哈希；每次命中仍重新计算哈希，失效字节会被删除并重新下载。编译后的
`WebAssembly.Module` 不会持久化。

批量数据的二进制导入也接受 `SharedArrayBuffer`；与普通 `ArrayBuffer` 不同，
共享输入不会 detach。`WorkerClient.requestShared(op, args, options)` 会要求将低层
协议结果中的所有输出 buffer 描述符物化为 `SharedArrayBuffer`，避免在 Worker
边界克隆或 detach 数据；宿主仍会在 wasm 线性内存与共享缓冲之间复制一次。浏览器使用
批量共享和协作取消都需要跨源隔离。
