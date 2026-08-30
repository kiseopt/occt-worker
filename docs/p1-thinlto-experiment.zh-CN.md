# P1 ThinLTO 实验记录

任务编号：P1
状态：完成（实验通过；性能与体积结果需项目作者确认是否接受）
日期：2026-08-30

## 改动

- `scripts/build-config.ps1` 的本地 OCCT 编译加入 `-flto=thin`。
- `kernel/CMakeLists.txt` 的 isolated、shared Main、shared Side 编译与链接统一使用 `-flto=thin`。
- `Dockerfile` 的 full 与 shared PIC OCCT 构建加入 `-flto=thin`。
- `bench/smoke.mjs` 支持 `--wasm=<path>`，便于对指定 artifact 测量。
- `tests/ts/thin-lto-regression.test.mjs` 覆盖多截面 loft、多边 fillet 后 tessellate。

这改变了 `tmp-docs/2026-08-17-OCCT-WASM开源内核项目方案.md` 记录的 v0.1 安全决策（该决策因完整 LTO 的圆角网格化越界而禁用编译期全库 LTO），需要项目作者确认是否接受。

## 验证

| 验证 | 结果 |
| --- | --- |
| ThinLTO 圆角结果网格化回归 | 通过；多截面八边形 loft，四条边 fillet，三角化成功且结果有限 |
| `npm run test:artifacts` | 通过：`verify-artifacts: ok (16 artifacts)` |
| `npm test` | 通过：403 tests，403 passed，0 failed |
| `node bench/smoke.mjs --wasm=artifacts/full.wasm` | 通过 |

## 产物与基准

正式 isolated artifact（`wasm-opt -O3 --all-features --converge` 后）：

| Profile | 大小 |
| --- | ---: |
| `core-modeling` | 22,232,596 bytes |
| `mesh` | 22,780,556 bytes |
| `exchange` | 18,599,871 bytes |
| `preview` | 12,707,550 bytes |
| `full-profile` (`artifacts/full.wasm`) | 33,477,106 bytes |

`full.wasm` 相对基线 27,478,954 bytes 增加 21.83%。shared Main wasm 为 36,831,532 bytes，相对基线 30,614,797 bytes 增加 20.31%；shared glue 与 side 也已重新构建并回填哈希。

五轮 benchmark 中位数对比：

| 操作 | 基线 | ThinLTO | 变化 |
| --- | ---: | ---: | ---: |
| booleanCut | 3.117 ms | 4.395 ms | 回退 |
| fillet | 5.418 ms | 7.121 ms | 回退 |
| tessellate | 2.200 ms | 3.228 ms | 回退 |

构建资源记录：

| 构建 | 耗时 | 峰值工作集 |
| --- | ---: | ---: |
| 原配置 OCCT 全量 | 710.830 s | 2,961,272,832 bytes |
| ThinLTO OCCT 全量 | 703.515 s | 2,574,004,224 bytes |
| 原配置 `full-profile` kernel | 102.238 s | 3,343,908,864 bytes |
| ThinLTO `full-profile` kernel | 246.929 s | 3,338,715,136 bytes |

ThinLTO OCCT 编译峰值工作集下降 13.08%，但 kernel 链接耗时增加，运行 benchmark 有回退，不能据此宣称性能收益。

遇到的 STOP：无。几何回归未失败，构建未 OOM-kill。

发现但未处理的问题：benchmark 回退及产物增大需要项目作者决定是否保留该构建决策；不在本任务中自行撤销实验。

等待真机数据：无。
