# Protocol v1.2.0 / 协议 v1.2.0

英文规范：[protocol.md](protocol.md)。`protocol/` 是机器可读单一事实源，生成文件由 `npm run generate` 更新。

## 请求与响应

请求帧：`{ id, op, args }`；成功：`{ id, ok: true, result }`；失败：`{ id, ok: false, error: { code, message, details? } }`。`id` 由客户端单调递增。错误按稳定 `code` 分支，`message` 仅用于阅读。

wasm ABI 为同步串行调用：宿主用 `k_alloc` 分配请求内存，写入 UTF-8 JSON，调用 `k_handle(ptr, len)`，再从 `k_response_ptr()` 读取响应，最后用 `k_free` 释放请求内存。`k_handle` 返回 0 表示实例不可继续使用，应重建。公开 Worker 客户端通过 `postMessage` 转发，不暴露 wasm 线性内存。

## 句柄、作用域和缓冲

形状句柄是实例内 u32，内部带 generation；`ShapeScope` 负责批量释放。TypeScript 的 `ShapeHandle` 还绑定客户端 epoch，worker 重建后旧句柄发送前就会失败。

大数据使用 `{ bufferId, byteLength, layout }` 描述符。输入通过 `createBuffer` 或 Worker 的二进制传输进入内核，输出在宿主 materialize 后调用 `freeBuffer`。memory growth 会使旧 TypedArray 视图失效，必须重新获取。

## Batch

`batch` 按顺序执行子操作，遇错停止并返回已完成前缀及错误。`{ "$ref": 0 }` 引用前序子操作的 `result.shape`。Batch 不是事务：此前创建的句柄仍归请求的 scope 所有。

## 取消与超时

队列中的请求可以直接撤销。STEP/IGES/XCAF、VRML、网格和网格化在进度检查点读取一字 `SharedArrayBuffer` 取消标志，并在协作停止时返回稳定的 `Cancelled`。其他活动同步操作通过终止并重建 worker 硬取消；旧句柄和队列同时失效。

`timeoutMs` 是 WorkerClient 选项，不进入协议。计时从请求实际派发给空闲 worker 开始；超时会重建 worker 并报 `TimeoutError`。DirectClient 同步执行期间不能提供硬超时。

## 能力、版本和错误码

`capabilities` 返回 `protocolVersion`、`kernelVersion`、`occtVersion`、`ops`、`historySupport` 和 `buildFlags`。当前协议版本为 `1.2.0`，发布产物实测公开 161 个操作。历史能力逐操作为 `full`、`partial` 或 `unsupported`；请求不支持的历史会得到 `InvalidArgs`。

稳定错误码包括：`ConstructionFailed`、`BooleanFailed`、`FilletFailed`、`TessellationFailed`、`ImportExportFailed`、`HealingFailed`、`Cancelled`、`InvalidHandle`、`InvalidArgs`、`OutOfMemory`、`KernelError` 和 `ProtocolError`。

## 网格和交换布局

网格 positions/normals 使用 f32，indices 和 groups 使用 u32。`faceGroups` 三元组为 `(faceIndex, indexStart, indexCount)`，其中 start/count 指向标量 index 条目；`edgeGroups` 为 `(edgeIndex, vertexStart, vertexCount)`。网格顶点按面隔离，不能假设跨面共享。

STEP/IGES 单位、schema、时间戳、OBJ/PLY/VRML 编码、XCAF 格式、glTF 资源和所有默认值均在 `protocol/operation-contracts.json` 冻结。BREP 必须绑定精确 wasm SHA-256 才能作为缓存使用；它不是跨版本交换格式。

## 确定性边界

内核关闭并行 BOP/网格，固定浮点收缩策略，不读取系统时间或随机数。几何结果按容差比较；只有协议明确规范化的布局才比较原始字节。拓扑顺序不等同于持久拓扑命名。
