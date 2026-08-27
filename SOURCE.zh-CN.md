# 对应源码与重新链接

wasm 二进制静态链接了 Open CASCADE Technology。规范的版本、上游 URL、commit 和 OCCT 补丁路径统一定义在 `protocol/artifacts.json` 的 `sources` 字段中；构建、源码获取和发布流程直接消费这些字段。

每个 GitHub tag 发布会在 wasm 二进制旁提供 `occt-worker-vX.Y.Z-corresponding-source.tar.gz` 及 SHA-256。该压缩包包含项目源码、构建/重新链接脚本、许可证、清单固定的源码树和生成的 `SOURCE-IDENTITIES.txt`。GitHub 自动生成的源码压缩包不会填充子模块，因此不能代替对应源码包。

从 npm 包或源码目录执行：

```text
npm run bootstrap:sources
npm ci
npm run build:wasm
```

`bootstrap:sources` 会按清单中的 commit 获取源码，并拒绝覆盖不同版本的 checkout。Git checkout 也可以执行 `git submodule update --init --recursive`。Emscripten 会由构建脚本按清单版本安装和激活。

替换 `occt` 子模块为兼容的修改版后可以生成新的 `wasm/occt-worker.wasm`。发布者仍需提供适用的 OCCT 对应源码、项目源码、构建脚本和许可证材料；本文件是工程指引，不是法律意见。

英文规范：[SOURCE.md](SOURCE.md)。
