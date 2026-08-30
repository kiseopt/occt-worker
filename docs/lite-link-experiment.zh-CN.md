# `lite` 与轻量建模链接实验

日期：2026-08-30

## 目的

验证 `lite` 与轻量建模是否需要两个 wasm 层。设计定义中的 `lite` 已包含查看、
基元和布尔能力；轻量建模另外要求拉伸与旋转。实验只改变候选链接命令中的
toolkit 列表，不修改 C++ 源码、正式 profile 或发布产物。

## 输入

当前最小的“能看能改”现有闭包是 `mesh`：它在 `core-modeling` 的建模与布尔能力
之外包含三角化。两组候选都复用该 profile 已编译的 source/object 闭包，并输出到
忽略目录 `build/candidates/e2/`。

| 候选 | Source/object 闭包 | Toolkit 闭包 |
| --- | --- | --- |
| `lite` | `runtime`、`geometry-topology`、`topology-query`、`modeling`、`algorithms`、`tessellation`、`mesh` | `TKXSBase`、`TKBO`、`TKShHealing`、`TKTopAlgo`、`TKGeomAlgo`、`TKBRep`、`TKGeomBase`、`TKG3d`、`TKG2d`、`TKMath`、`TKernel`、`TKFeat`、`TKOffset`、`TKFillet`、`TKBool`、`TKPrim`、`TKHLR`、`TKMesh` |
| 轻量建模 | 与 `lite` 相同 | 与 `lite` 相同；拉伸和旋转没有新增 toolkit |

`modeling` 是一个实现单元：基元、布尔所需工具体、拉伸和旋转使用相同源码组，
且 `lite` 已经需要的 `TKPrim` / `TKBO` 覆盖拉伸和旋转的 toolkit 依赖。因此当前
边界下不存在只供轻量建模追加的 toolkit。

## 结果

两组候选均使用 Release 配置、`-O3 -flto`，并以
`wasm-opt -O3 --all-features --converge` 后处理。

| 候选 | Operations | Raw bytes | Brotli-11 bytes | SHA-256 |
| --- | ---: | ---: | ---: | --- |
| `lite.wasm` | 140 | 18,720,952 | 4,083,850 | `c1eaaa3190a0617ff7d59168c326a726579c4ed5b302b1bb5564d7e3d0b2eceb` |
| `light-modeling.wasm` | 140 | 18,720,952 | 4,083,850 | `c1eaaa3190a0617ff7d59168c326a726579c4ed5b302b1bb5564d7e3d0b2eceb` |

五轮同进程 smoke 的中位耗时如下。耗时仅用于确认没有明显异常；候选字节完全
相同，不把同进程运行顺序造成的小幅波动解释为性能差异。

| 候选 | Boolean cut | Fillet | Tessellate | Extrude | Revolve |
| --- | ---: | ---: | ---: | ---: | ---: |
| `lite.wasm` | 2.893 ms | 5.301 ms | 2.667 ms | 0.160 ms | 0.295 ms |
| `light-modeling.wasm` | 2.831 ms | 4.775 ms | 1.644 ms | 0.196 ms | 0.324 ms |

两组候选的布尔、圆角、三角化、拉伸和旋转均成功，结束 scope 后
`liveShapeHandles` 与 `liveBufferBytes` 均为 0，线性内存容量均为 16,777,216
字节。

## 结论

假设成立：当前实现单元与 toolkit 边界下，轻量建模相对 `lite` 的 wasm 增量为
0，且两份优化产物逐字节相同。没有依据发布两个等价预设；后续出现能形成不同
能力闭包的真实调用方时，再从 `protocol/modules.json` 定义新的预设。
