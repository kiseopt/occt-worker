# Corresponding source and relinking

[中文说明](SOURCE.zh-CN.md)

The wasm binary contains statically linked Open CASCADE Technology code. The canonical versions, upstream URLs, commits, and OCCT patch path are the `sources` entries in `protocol/artifacts.json`. Build, bootstrap, and release workflows consume those entries directly.

Every tagged GitHub release attaches `occt-worker-vX.Y.Z-corresponding-source.tar.gz` and its `.sha256` checksum beside the wasm binaries. That archive contains the source of the tagged project, its build and relinking scripts, license and notice files, all populated source trees at the manifest commits, and a generated `SOURCE-IDENTITIES.txt`. The OCCT tree already has the manifest's patch applied, and the patch file is retained separately for review. Download this asset from the release's Assets section; GitHub's automatically generated source archives do not populate submodules and are not the corresponding-source asset.

The npm package and Git repository include the kernel and protocol source, exact dependency commits, a published `npm-shrinkwrap.json`, and a bootstrap command that fetches those corresponding sources. From an unpacked npm package or source checkout run:

```text
npm run bootstrap:sources
npm ci
npm run build:wasm
```

`bootstrap:sources` clones each upstream repository at the exact manifest commit and refuses to overwrite a checkout at a different revision. In a Git checkout, `git submodule update --init --recursive` is an equivalent way to obtain those same revisions. `npm ci` uses the shrinkwrap shipped in the npm tarball. The Emscripten toolchain itself is installed and activated at the manifest version by the build command.

Replacing the `occt` submodule checkout with a compatible modified tree and running the same build command produces a replacement `wasm/occt-worker.wasm`. The build deliberately keeps the wasm as a separate runtime-loadable file. The npm tarball's network bootstrap is a reproducible build input, not a substitute for a distributor's corresponding-source obligations. Binary distributors must accompany their distribution with the applicable OCCT corresponding source (including local changes), the independent project source and build scripts, and the license files named in `LICENSES.md`, using a delivery method confirmed by their legal review; this engineering guidance is not legal advice.
