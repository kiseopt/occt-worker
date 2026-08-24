# Corresponding source and relinking

[中文说明](SOURCE.zh-CN.md)

The wasm binary contains statically linked Open CASCADE Technology code. The exact source identities are:

- OCCT 8.0.1: commit `b8f597c677811d1f9f4d8a97f5ae2825c0353a42`
- Emscripten SDK 4.0.23: commit `c0bb220cb6e6f4e0fabb6f6db9efd53390ef5e56`
- nlohmann/json 3.12.0: commit `55f93686c01528224f448c19128836e7df245f72`

The OCCT corresponding source also includes `patches/occt-8.0.1-wasm.patch`. The build applies this patch to the pinned OCCT commit before compiling. It contains the memory-stream, wasm, and XML persistence fixes used by the distributed binary.

Every tagged GitHub release attaches `occt-worker-vX.Y.Z-corresponding-source.tar.gz` and its `.sha256` checksum beside the wasm binaries. That archive contains the source of the tagged project, its build and relinking scripts, license and notice files, all three populated submodule source trees at the commits above, and `SOURCE-IDENTITIES.txt`. The OCCT tree in the archive already has `patches/occt-8.0.1-wasm.patch` applied, and the patch file is retained separately for review. Download this asset from the release's Assets section; GitHub's automatically generated source archives do not populate submodules and are not the corresponding-source asset.

The npm package and Git repository include the kernel and protocol source, exact dependency commits, a published `npm-shrinkwrap.json`, and a bootstrap command that fetches those corresponding sources. From an unpacked npm package or source checkout run:

```text
npm run bootstrap:sources
npm ci
npm run build:wasm
```

`bootstrap:sources` clones each upstream repository at the exact commit listed above and refuses to overwrite a checkout at a different revision. In a Git checkout, `git submodule update --init --recursive` is an equivalent way to obtain those same revisions. `npm ci` uses the shrinkwrap shipped in the npm tarball. The Emscripten toolchain itself is installed and activated at the pinned version by the build command.

The pinned OCCT 8.0.1 headers mark several compatibility APIs used by the kernel as deprecated, so the build currently emits non-fatal compiler warnings for types such as `TopTools_IndexedMapOfShape`, `TopTools_ListOfShape`, and the `TColgp_*` collection aliases, as well as `Standard_Failure::GetMessageString()`. These warnings do not prevent `npm run build:wasm` from completing or invalidate the generated wasm artifact. They identify source migrations required before a future OCCT version removes those compatibility APIs.

Replacing the `occt` submodule checkout with a compatible modified OCCT 8.0.1 tree and running the same build command produces a replacement `wasm/occt-worker.wasm`. The build deliberately keeps the wasm as a separate runtime-loadable file. The npm tarball's network bootstrap is a reproducible build input, not a substitute for a distributor's corresponding-source obligations. Binary distributors must accompany their distribution with the applicable OCCT corresponding source (including local changes), the independent project source and build scripts, and the license files named in `LICENSES.md`, using a delivery method confirmed by their legal review; this engineering guidance is not legal advice.
