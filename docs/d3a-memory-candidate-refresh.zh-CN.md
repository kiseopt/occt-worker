# D3a 内存上限候选产物

日期：2026-08-30

候选产物位于 `tests/device/candidates/`，不进入发布 artifact 或 `protocol/artifacts.json`。本次以现行 profile 构建重新生成 1536 MiB 候选；其余档位从同一已验证 wasm 产物派生，仅修改 WebAssembly memory section 的 maximum pages 字段，保持代码、协议和链接内容一致。

| 上限 | wasm maximum pages | preview | full |
| ---: | ---: | ---: | ---: |
| 1536 MiB | 24576 | 12,707,550 bytes | 33,477,106 bytes |
| 1280 MiB | 20480 | 12,707,550 bytes | 33,477,106 bytes |
| 1024 MiB | 16384 | 12,707,550 bytes | 33,477,106 bytes |
| 768 MiB | 12288 | 12,707,550 bytes | 33,477,106 bytes |

八个模块均通过 `new WebAssembly.Module(...)` 检查；脚本解析 memory section 确认每个文件的 maximum pages 与表格一致。候选文件仍只用于 D3b 真机二分，不修改默认 `OCCT_WORKER_MAX_MEMORY=2147483648`，也未写入发布 manifest。

桌面 Chromium 测量页验证：`npm run test:browser` 通过，输出 `workerSoftFailure: true`、`longRunRounds: 5`、`fullLoadAttemptStates: ["success","failure","reload"]`；该结果仅证明测量工装流程可用，不替代 iPad 实测。

## 给项目作者的操作

1. 通过 HTTP 服务打开 `tests/device/`，填写设备型号、系统版本和浏览器。
2. 从 1536 MiB 开始选择 `preview` 或 `full`，点击“开始二分测试”。
3. 记录典型负载是否完成，以及超限时显示“已捕获 RangeError”还是页面重载/白屏。
4. 若页面未完成，重新打开后按页面提示改测更小档位；回报复制出的完整结果文本和最终选定字节数。

结果必须带设备型号、系统版本和测试负载，不代表通用 iPad 安全水位。
