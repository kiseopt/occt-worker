# 贡献指南

欢迎在 `README.zh-CN.md`、[宿主支持](docs/hosts.zh-CN.md) 和 [能力矩阵](docs/capabilities.zh-CN.md) 规定的操作与宿主边界内贡献代码。项目没有支持 SLA，路线由维护者决定。

1. 初始化三个固定版本的子模块：`git submodule update --init --recursive`。
2. 执行 `npm ci`、`npm run build:wasm` 和 `npm test`。
3. 修改宿主边界时，安装 Chromium：`npx playwright install chromium`，并执行 `npm run test:browser`。
4. 新机制至少增加一个代表性成功测试；只有引入独立失败机制时才增加失败测试。
5. 协议修改必须先改 `protocol/`，然后执行 `npm run generate`，不要直接编辑生成文件。

## 常用开发命令

| 命令 | 用途 |
| --- | --- |
| `npm run generate` | 重新生成协议 TypeScript 类型与 Profile 定义 |
| `npm run build` | 构建协议定义并编译 TypeScript（`dist/`） |
| `npm run build:wasm` | 引导工具链并编译发布版 WebAssembly 内核 |
| `npm run build:wasm:debug` | 编译带调试符号的 WebAssembly 内核 |
| `npm test` | 运行 TypeScript 单元测试套件 |
| `npm run test:browser` | 运行默认浏览器 Worker 测试（需要 Chromium） |
| `npm run test:browser:all` | 运行跨 Chromium、Firefox 和 WebKit 的浏览器测试 |
| `npm run test:package` | 验证打包后的 npm 消费者集成测试 |

## 仓库结构

- `kernel/`：C++ 建模、查询、交换和 wasm 边界实现。
- `protocol/`：操作定义、schema、错误定义和协议生成输入的权威目录。
- `ts/src/`：公开 TypeScript 客户端、类型、建模辅助和生成声明。
- `tests/`：TypeScript 行为测试、浏览器/Wasmtime 检查和 npm 打包消费者测试。
- `scripts/`：固定工具链引导、wasm 构建、协议生成和产物校验。
- `docs/`：公开 API、协议、能力、宿主、构建证据和发布文档。
- `wasm/`：发布运行时产物；提交前必须通过对应验证脚本。

修改协议或操作时，先更新 `protocol/` 下的权威文件，执行 `npm run generate`，
然后补充最小必要的内核、TypeScript 和消费者测试。公开能力矩阵与宿主文档必须
和实际验证过的行为保持一致。

独立项目文件按 Apache-2.0 贡献。复制或应用到 OCCT 的改动仍遵循 OCCT 的 LGPL-2.1 及例外条款，并保留对应通知。

英文规范：[CONTRIBUTING.md](CONTRIBUTING.md)。
