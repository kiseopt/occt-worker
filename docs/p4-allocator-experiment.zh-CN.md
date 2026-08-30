# P4 内存分配器对比

本报告记录 `full-profile` 在相同源码和链接参数下切换 Emscripten 分配器的候选结果。候选文件位于 `build/candidates/p4/`，没有覆盖发布 artifact。

## 结果

| 分配器 | wasm 字节 | Brotli-5 字节 | 初始线性内存 | 最大线性内存 | `npm test` | smoke 中位数（booleanCut / fillet / tessellate） |
| --- | ---: | ---: | ---: | ---: | --- | --- |
| `dlmalloc` | 27,478,954 | 6,434,105 | 16 MiB | 2,048 MiB | 403/403 通过 | 3.085 / 5.710 / 2.309 ms |
| `emmalloc` | 27,481,685 | 6,432,641 | 16 MiB | 2,048 MiB | 403/403 通过 | 3.257 / 6.877 / 2.227 ms |
| `mimalloc` | 未生成 | 未生成 | 未生成 | 未生成 | 未执行 | 链接失败：`mallinfo` 未定义 |

线性内存限制来自候选 wasm 的 memory section；它表示 wasm 地址空间容量，不是系统内存峰值。桌面 smoke 只覆盖运算速度，不能代表 iPad 的系统内存压力或长期堆碎片。

## 判定

`dlmalloc` 与 `emmalloc` 都通过了现有 Node 回归和 smoke。当前桌面数据中，`emmalloc` 的布尔和圆角中位数较慢，三角化略快，未形成稳定的速度优势；体积差异也不足以支持切换默认值。

`mimalloc` 在当前 Emscripten/OCCT 链接配置下因 `mallinfo` 未定义而触发指南规定的 STOP，未修改公共链接参数绕过该错误。因此 P4 暂不选定新的默认分配器。堆碎片、系统内存压力和真机长期行为仍需在同一目标设备与负载下测量后再决定。
