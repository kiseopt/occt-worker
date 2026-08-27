# occt-worker reproducible build image (stage 7).
#
# Toolchain and source identities come from protocol/artifacts.json. CMake,
# Ninja, and Node remain image bootstrap inputs below.
#
# The image wraps the SAME build graph as scripts/build-wasm.ps1 /
# scripts/build-shared.ps1 through CMake; it never re-implements business
# build logic.

FROM debian:12-slim AS tools

ARG CMAKE_VERSION=4.4.2
ARG NINJA_VERSION=1.13.2
ARG NODE_VERSION=20.18.1

RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates curl xz-utils python3 git libatomic1 \
    && rm -rf /var/lib/apt/lists/*

RUN test "$(dpkg --print-architecture)" = "amd64" \
    && curl -fsSLo /tmp/cmake.sh \
       "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh" \
    && sh /tmp/cmake.sh --prefix=/usr/local --skip-license \
    && curl -fsSLo /tmp/ninja.zip \
       "https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/ninja-linux.zip" \
    && python3 -c "import zipfile,sys; zipfile.ZipFile('/tmp/ninja.zip').extractall('/usr/local/bin')" \
    && chmod +x /usr/local/bin/ninja \
    && curl -fsSLo /tmp/node.tar.xz \
       "https://nodejs.org/dist/v${NODE_VERSION}/node-v${NODE_VERSION}-linux-x64.tar.xz" \
    && tar -xJf /tmp/node.tar.xz -C /usr/local --strip-components=1

FROM tools AS build-env

ENV EMSDK=/opt/emsdk
WORKDIR /src
COPY protocol/artifacts.json /src/protocol/artifacts.json
RUN EMSDK_URL="$(node -p 'JSON.parse(require("fs").readFileSync("/src/protocol/artifacts.json")).sources.emsdk.url')" \
    && EMSDK_COMMIT="$(node -p 'JSON.parse(require("fs").readFileSync("/src/protocol/artifacts.json")).sources.emsdk.commit')" \
    && EMSDK_VERSION="$(node -p 'JSON.parse(require("fs").readFileSync("/src/protocol/artifacts.json")).sources.emsdk.version')" \
    && git clone "${EMSDK_URL}" "${EMSDK}" \
    && git -C "${EMSDK}" checkout --detach "${EMSDK_COMMIT}" \
    && "${EMSDK}/emsdk" install "${EMSDK_VERSION}" \
    && "${EMSDK}/emsdk" activate "${EMSDK_VERSION}"

COPY . /src

# Submodule content is expected to be present in the build context; when the
# context comes from `git archive`, fall back to cloning the pinned commit.
RUN if [ ! -f /src/occt/CMakeLists.txt ]; then \
      OCCT_URL="$(node -p 'JSON.parse(require("fs").readFileSync("/src/protocol/artifacts.json")).sources.occt.url')" \
      && OCCT_COMMIT="$(node -p 'JSON.parse(require("fs").readFileSync("/src/protocol/artifacts.json")).sources.occt.commit')" \
      && git clone "${OCCT_URL}" /src/occt \
      && git -C /src/occt checkout --detach "${OCCT_COMMIT}"; \
    fi

ENV PATH="${EMSDK}:${EMSDK}/upstream/emscripten:${EMSDK}/upstream/bin:${PATH}"
SHELL ["/bin/bash", "-c"]

RUN source "${EMSDK}/emsdk_env.sh" \
    && export PATH="${EMSDK}:${EMSDK}/upstream/emscripten:${EMSDK}/upstream/bin:${PATH}" \
    && node scripts/apply-occt-patches.mjs \
    && npm ci --ignore-scripts --no-audit --no-fund \
    && npm run build

FROM build-env AS full

RUN source "${EMSDK}/emsdk_env.sh" \
    && export PATH="${EMSDK}:${EMSDK}/upstream/emscripten:${EMSDK}/upstream/bin:${PATH}" \
    && emcmake cmake -S occt -B build/release/occt-build -G Ninja \
         -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=/src/build/release/occt-install \
         -DBUILD_LIBRARY_TYPE=Static \
         -DBUILD_RELEASE_DISABLE_EXCEPTIONS=OFF \
         '-DBUILD_ADDITIONAL_TOOLKITS=TKPrim;TKBO;TKBool;TKMesh;TKFillet;TKOffset;TKXSBase;TKDESTEP;TKDEIGES;TKDESTL;TKDEVRML;TKBinXCAF;TKXmlXCAF' \
         -DBUILD_DOC_Overview=OFF -DBUILD_DOC_RefMan=OFF -DBUILD_GTEST=OFF \
         -DUSE_TBB=OFF -DUSE_OPENGL=OFF -DUSE_GLES2=OFF -DUSE_FREETYPE=OFF \
         -DBUILD_MODULE_FoundationClasses=OFF -DBUILD_MODULE_ModelingData=OFF \
         -DBUILD_MODULE_ModelingAlgorithms=ON -DBUILD_MODULE_Visualization=OFF \
         -DBUILD_MODULE_ApplicationFramework=OFF -DBUILD_MODULE_DataExchange=OFF \
         -DBUILD_MODULE_Draw=OFF \
         '-DCMAKE_CXX_FLAGS=-fwasm-exceptions -ffp-contract=off -UOCC_CONVERT_SIGNALS' \
    && cmake --build build/release/occt-build --target install --parallel

RUN source "${EMSDK}/emsdk_env.sh" \
    && export PATH="${EMSDK}:${EMSDK}/upstream/emscripten:${EMSDK}/upstream/bin:${PATH}" \
    && emcmake cmake -S . -B build/release/kernel -G Ninja \
         -DCMAKE_BUILD_TYPE=Release -DOCCT_ROOT=/src/build/release/occt-install \
    && cmake --build build/release/kernel --parallel \
    && wasm-opt build/release/kernel/kernel/occt-worker.wasm -O3 --all-features \
         -o build/release/kernel/kernel/occt-worker.optimized.wasm \
    && mkdir -p wasm \
    && cp build/release/kernel/kernel/occt-worker.optimized.wasm wasm/occt-worker.wasm \
    && node scripts/verify-wasm.mjs

FROM full AS profiles

RUN source "${EMSDK}/emsdk_env.sh" \
    && export PATH="${EMSDK}:${EMSDK}/upstream/emscripten:${EMSDK}/upstream/bin:${PATH}" \
    && emcmake cmake -S . -B build/release/profiles -G Ninja \
         -DCMAKE_BUILD_TYPE=Release -DOCCT_ROOT=/src/build/release/occt-install \
         -DKERNEL_BUILD_PROFILES=ON \
    && cmake --build build/release/profiles --parallel \
    && mkdir -p artifacts \
    && for profile in core-modeling mesh exchange step-preview modeling-viewer full; do \
         wasm-opt "build/release/profiles/kernel/profile-${profile}.wasm" -O3 --all-features \
           -o "artifacts/${profile}.wasm"; \
       done

FROM full AS shared

RUN source "${EMSDK}/emsdk_env.sh" \
    && export PATH="${EMSDK}:${EMSDK}/upstream/emscripten:${EMSDK}/upstream/bin:${PATH}" \
    && emcmake cmake -S occt -B build/release/shared/occt-build-pic -G Ninja \
         -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=/src/build/release/shared/occt-install-pic \
         -DBUILD_LIBRARY_TYPE=Static \
         -DBUILD_RELEASE_DISABLE_EXCEPTIONS=OFF \
         '-DBUILD_ADDITIONAL_TOOLKITS=TKPrim;TKBO;TKBool;TKMesh;TKFillet;TKOffset;TKXSBase;TKDESTEP;TKDEIGES;TKDESTL;TKDEVRML;TKBinXCAF;TKXmlXCAF' \
         -DBUILD_DOC_Overview=OFF -DBUILD_DOC_RefMan=OFF -DBUILD_GTEST=OFF \
         -DUSE_TBB=OFF -DUSE_OPENGL=OFF -DUSE_GLES2=OFF -DUSE_FREETYPE=OFF \
         -DBUILD_MODULE_FoundationClasses=OFF -DBUILD_MODULE_ModelingData=OFF \
         -DBUILD_MODULE_ModelingAlgorithms=ON -DBUILD_MODULE_Visualization=OFF \
         -DBUILD_MODULE_ApplicationFramework=OFF -DBUILD_MODULE_DataExchange=OFF \
         -DBUILD_MODULE_Draw=OFF \
         '-DCMAKE_CXX_FLAGS=-fPIC -fwasm-exceptions -ffp-contract=off -UOCC_CONVERT_SIGNALS' \
    && cmake --build build/release/shared/occt-build-pic --target install --parallel

RUN source "${EMSDK}/emsdk_env.sh" \
    && export PATH="${EMSDK}:${EMSDK}/upstream/emscripten:${EMSDK}/upstream/bin:${PATH}" \
    && emcmake cmake -S . -B build/release/shared/kernel -G Ninja \
         -DCMAKE_BUILD_TYPE=Release \
         -DOCCT_ROOT=/src/build/release/shared/occt-install-pic \
         -DKERNEL_BUILD_SHARED=ON \
    && cmake --build build/release/shared/kernel --parallel \
    && mkdir -p artifacts \
    && cp build/release/shared/kernel/kernel/shared-main.mjs artifacts/shared-main.mjs \
    && cp build/release/shared/kernel/kernel/shared-main.wasm artifacts/shared-main.wasm \
    && cp build/release/shared/kernel/kernel/shared-side-geometry_topology.wasm artifacts/geometry-topology.side.wasm \
    && cp build/release/shared/kernel/kernel/shared-side-modeling.wasm artifacts/modeling.side.wasm \
    && cp build/release/shared/kernel/kernel/shared-side-algorithms.wasm artifacts/algorithms.side.wasm \
    && cp build/release/shared/kernel/kernel/shared-side-mesh.wasm artifacts/mesh.side.wasm \
    && cp build/release/shared/kernel/kernel/shared-side-exchange_mesh.wasm artifacts/exchange-mesh.side.wasm \
    && cp build/release/shared/kernel/kernel/shared-side-exchange_cad.wasm artifacts/exchange-cad.side.wasm

FROM shared AS build-all
COPY --from=profiles /src/artifacts/ /src/artifacts/
RUN node scripts/verify-modules.mjs --quiet \
    && node scripts/verify-artifacts.mjs --write \
    && node scripts/verify-symbol-closure.mjs \
         --main artifacts/shared-main.wasm \
         --side artifacts/geometry-topology.side.wasm \
         --side artifacts/modeling.side.wasm \
         --side artifacts/algorithms.side.wasm \
         --side artifacts/mesh.side.wasm \
         --side artifacts/exchange-mesh.side.wasm \
         --side artifacts/exchange-cad.side.wasm
