# 内存上限真机测试

1. 通过 HTTP 服务打开 `tests/device/`，填写设备型号、系统版本和浏览器。
2. 选择 `preview` 或 `full`，点击“开始二分测试”。典型负载完成后，测试会增长 wasm 线性内存，并每隔 4 KiB 写入后读回一个字节以确认页面被触碰。
3. 低于 4096 MiB 的候选若显示 `candidate-declared-maximum`，只表示到达该 wasm 文件声明的 maximum；4096 MiB 若显示 `wasm32-address-space-maximum`，表示到达 wasm32 的 65536 页固有上限。两者都不是设备物理内存上限。
4. 若在声明容量之前显示 `runtime-allocation-limit`，只表示浏览器/运行时提前拒绝增长；若出现白屏、网页重载或 `incomplete`，只记录为本次尝试未完成。不要把这些结果换算成设备内存数字。

结果中的 `candidate.declared.maximum.mb/pages` 是候选文件的声明值；`wasm.last.observed.capacity.mb/pages` 是最后一次成功增长后的线性内存容量；`wasm.touch.checksum` 只确认测试写入并读回了每个 4 KiB 区段。WebAssembly 没有公开读取 memory maximum 的 API，因此 `RangeError` 本身不会提供设备上限；`expects the grown size to be a valid page count` 是 4 GiB wasm32 地址空间边界的表现。

若候选测试都只是到达 wasm 上限，改用页面下方的“进程压力测试”。它先运行 full 典型负载，再分块分配并写入额外的 JS 缓冲区；压力目标从 1024 MiB 起，可越过单个 wasm32 memory 的 4 GiB 上限。`pressure-complete` 只表示页面完成了目标压力，`allocation-error` 只表示浏览器拒绝了分配；Worker 失联或页面重载才是需要记录的未完成结果，仍不能换算成设备物理内存读数。

因此，1 GiB wasm 候选没有看到设备内存明显增长并不矛盾：`memory.buffer.byteLength` 是线性内存容量，浏览器可能按需提交、压缩或不提供系统级读数。要观察进程承压行为，请运行 `full + 1024 MiB`、`2048 MiB` 等压力目标，并记录页面是否完成、分配失败或失联。

压力测试最高会写入 7168 MiB，可能导致网页重载，只在专门的测试设备上运行。

结果只代表填写的设备、系统版本和测试负载，不能作为 iPad 的通用内存上限。
