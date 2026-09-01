# Documentation / 文档导航

This directory contains the public, release-facing documentation for `occt-worker`.
Each language pair covers the same public behavior and limits. The English file is the
normative source for exact identifiers; the `zh-CN` file expresses the same contract in
natural Chinese rather than mirroring English sentence structure.

本目录包含 `occt-worker` 面向发布的公开文档。每组语言文件覆盖相同的公开行为和限制；
精确标识符以英文文件为准，`zh-CN` 文件在保持契约一致的前提下采用自然中文表达。

| Document | Contents / 内容 |
| --- | --- |
| [getting-started.md](getting-started.md) / [getting-started.zh-CN.md](getting-started.zh-CN.md) | First-use guide for installation, runtime choices, source builds, generated code, and troubleshooting / 首次使用、运行入口、源码构建、生成代码和常见问题 |
| [architecture.md](architecture.md) / [architecture.zh-CN.md](architecture.zh-CN.md) | Runtime layering, Profiles, artifacts, shape ownership, lifecycle, and memory boundaries / 运行时分层、Profile、产物、形状所有权、生命周期和内存边界 |
| [api.md](api.md) / [api.zh-CN.md](api.zh-CN.md) | TypeScript API, typed handles, modeling, exchange, mesh, parametric and cancellation / TypeScript API、句柄、建模、交换、网格、参数化和取消 |
| [protocol.md](protocol.md) / [protocol.zh-CN.md](protocol.zh-CN.md) | Message frames, ABI, buffers, errors, defaults and determinism / 消息帧、ABI、缓冲、错误、默认值和确定性 |
| [capabilities.md](capabilities.md) / [capabilities.zh-CN.md](capabilities.zh-CN.md) | Implemented v1 capability matrix and explicit limits / v1 已实现能力矩阵和明确限制 |
| [hosts.md](hosts.md) / [hosts.zh-CN.md](hosts.zh-CN.md) | Node, Worker, browsers and Wasmtime support / Node、Worker、浏览器和 Wasmtime 支持 |
| [profile-capabilities.generated.md](profile-capabilities.generated.md) / [profile-capabilities.generated.zh-CN.md](profile-capabilities.generated.zh-CN.md) | Generated compile-time profile capability narrowing / 生成的编译期 profile 能力收窄 |
| [g0-build.json](g0-build.json) | Machine-readable release wasm hash and import/export evidence / 发布 wasm 哈希及导入导出证据 |
| [wasmtime-build.json](wasmtime-build.json) | Wasmtime artifact hash and smoke evidence / Wasmtime 产物哈希和 smoke 证据 |

The JSON evidence files are language-neutral. Legal originals such as
`LICENSE-APACHE-2.0.txt` and `THIRD-PARTY-NOTICES.txt` remain authoritative in
their original language; their Chinese files are indexes or summaries, not replacements.

JSON 证据文件不区分语言。`LICENSE-APACHE-2.0.txt` 和
`THIRD-PARTY-NOTICES.txt` 等法律原文仍以原始语言为准；对应中文文件是索引或摘要，
不能替代原文。

## Recommended reading order / 推荐阅读顺序

| Step / 顺序 | Start here / 入口 | Question answered / 解决的问题 |
| --- | --- | --- |
| 1 | [README.md](../README.md) / [README.zh-CN.md](../README.zh-CN.md) | What does the project provide, and which runtime should a new user choose? / 项目提供什么能力，新用户该选哪种运行时？ |
| 2 | [getting-started.md](getting-started.md) / [getting-started.zh-CN.md](getting-started.zh-CN.md) | How do I install, run, and build it? / 如何安装、运行和构建？ |
| 3 | [architecture.md](architecture.md) / [architecture.zh-CN.md](architecture.zh-CN.md) | How do Profiles, artifacts, ownership, and lifecycle fit together? / Profile、产物、所有权和生命周期如何协作？ |
| 4 | [api.md](api.md) / [api.zh-CN.md](api.zh-CN.md) | Which typed methods and result contracts are available? / 有哪些类型化方法和结果契约？ |
| 5 | [protocol.md](protocol.md) / [protocol.zh-CN.md](protocol.zh-CN.md) | What is the wire and WASM ABI contract? / 消息线格式和 WASM ABI 契约是什么？ |
| 6 | [capabilities.md](capabilities.md) / [capabilities.zh-CN.md](capabilities.zh-CN.md) | Which capabilities are complete and what limits apply? / 哪些能力已完成，有哪些限制？ |

The index below is the stable map of document responsibilities; use the source-of-truth files
linked from each page for exact operation names, defaults, and generated capability membership.

下面的索引是文档职责的稳定地图；操作名称、默认值和生成的能力成员关系以各页链接的事实源为准。

Repository-level documents:

仓库根目录文档：

| Document | Contents / 内容 |
| --- | --- |
| [README.md](../README.md) / [README.zh-CN.md](../README.zh-CN.md) | Project overview, quick start and build / 项目介绍、快速开始和构建 |
| [CONTRIBUTING.md](../CONTRIBUTING.md) / [CONTRIBUTING.zh-CN.md](../CONTRIBUTING.zh-CN.md) | Contribution workflow and test expectations / 贡献流程和测试要求 |
| [CODE_OF_CONDUCT.md](../CODE_OF_CONDUCT.md) / [CODE_OF_CONDUCT.zh-CN.md](../CODE_OF_CONDUCT.zh-CN.md) | Community behavior and enforcement / 社区行为与执行 |
| [SOURCE.md](../SOURCE.md) / [SOURCE.zh-CN.md](../SOURCE.zh-CN.md) | Corresponding source and relinking procedure / 对应源码和重新链接流程 |
| [LICENSES.md](../LICENSES.md) / [LICENSES.zh-CN.md](../LICENSES.zh-CN.md) | License boundary summary / 许可证边界说明 |
| [NOTICE](../NOTICE) / [NOTICE.zh-CN.md](../NOTICE.zh-CN.md) | Project and distribution notices / 项目和分发通知 |
| `LICENSE-APACHE-2.0.txt`, `THIRD-PARTY-NOTICES.txt` / `THIRD-PARTY-NOTICES.zh-CN.md` | Original legal texts and a non-authoritative Chinese index; do not replace originals / 原始法律文本及非权威中文索引，不以译文替代原文 |
