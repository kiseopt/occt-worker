# 对应源码与重新链接

wasm 二进制静态链接了 Open CASCADE Technology。固定源码身份如下：

- OCCT 8.0.1：commit `b8f597c677811d1f9f4d8a97f5ae2825c0353a42`
- Emscripten SDK 4.0.23：commit `c0bb220cb6e6f4e0fabb6f6db9efd53390ef5e56`
- nlohmann/json 3.12.0：commit `55f93686c01528224f448c19128836e7df245f72`

OCCT 对应源码还包含 `patches/occt-8.0.1-wasm.patch`。构建会在编译前将该补丁应用到固定 OCCT commit；补丁包含内存流、wasm 和 XML 持久化修复。

每个 GitHub tag 发布会在 wasm 二进制旁提供 `occt-worker-vX.Y.Z-corresponding-source.tar.gz` 及 SHA-256。该压缩包包含项目源码、构建/重新链接脚本、许可证、三个固定版本的子模块源码和 `SOURCE-IDENTITIES.txt`。GitHub 自动生成的源码压缩包不会填充子模块，因此不能代替对应源码包。

从 npm 包或源码目录执行：

```text
npm run bootstrap:sources
npm ci
npm run build:wasm
```

`bootstrap:sources` 会按上述 commit 获取源码，并拒绝覆盖不同版本的 checkout。Git checkout 也可以执行 `git submodule update --init --recursive`。Emscripten 会由构建脚本按固定版本安装和激活。

替换 `occt` 子模块为兼容的修改版 OCCT 8.0.1 后可以生成新的 `wasm/occt-worker.wasm`。发布者仍需提供适用的 OCCT 对应源码、项目源码、构建脚本和许可证材料；本文件是工程指引，不是法律意见。

英文规范：[SOURCE.md](SOURCE.md)。
