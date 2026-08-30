# P2 SIMD 候选实验

日期：2026-08-30

本实验按 P2 要求在本地 full/profile 构建的五处编译与链接路径临时加入普通 `-msimd128`，生成候选 wasm；实验结束后已移除这些改动，正式 `wasm/occt-worker.wasm` 未采用 SIMD。

## 产物

| 产物 | 大小 | SHA-256（前 16 位） |
| --- | ---: | --- |
| 正式基线 `artifacts/full.wasm`（full-profile） | 33,477,106 bytes | `425A1E1BA2F800A3` |
| SIMD 候选 `build/candidates/p2/full.wasm` | 33,770,388 bytes | `CF87995EA4D0AC30` |

候选 profile wasm 可正常实例化，`node bench/smoke.mjs --wasm=build/candidates/p2/full.wasm` 的 booleanCut、fillet、tessellate 均完成并返回合法结果，未见协议或几何异常。

## Smoke 基准

同一桌面环境下各运行两次；表中为每次运行的中位数（毫秒）：

| 操作 | 正式基线（运行 1 / 2） | SIMD 候选（运行 1 / 2） | 观察 |
| --- | ---: | ---: | --- |
| booleanCut | 3.596 / 3.106 | 3.756 / 3.201 | 基本持平，未形成稳定提升 |
| fillet | 5.106 / 5.118 | 6.955 / 6.288 | 候选较慢 |
| tessellate | 2.235 / 2.552 | 2.226 / 1.895 | 候选略快但不稳定 |

候选没有形成可重复的整体性能提升，且体积增加约 0.9%。依据指南中“benchmark 证明收益 + 目标宿主通过”的双重准入条件，本次不合入 `-msimd128`，P2 保持未完成。由于没有 iPad WebKit 或实际发布宿主的验证证据，后续仍需项目作者在目标宿主上单独确认后才能重新评估。
