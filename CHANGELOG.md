# Changelog

## 1.2.0 - 2026-08-27

### Added

- Modular shared Main/Side WebAssembly artifacts and isolated runtime profiles.
- A typed geometry engine, artifact resolution, and runtime compatibility checks.
- Expanded modeling, topology, mesh processing, and CAD/mesh exchange operations.

### Changed

- The public protocol and release artifact manifests are updated to version 1.2.0.
- Kernel sources and TypeScript types are split by operation domain for clearer ownership.

## 1.0.0 - 2026-08-24

Initial public release.

### Added

- A versioned message protocol and TypeScript clients for Node and browser Workers.
- Curated OCCT modeling, topology, query, tessellation, and exchange operations.
- Serializable parametric features, sketch constraints, subshape references, and assembly definitions.
- Release WebAssembly artifacts for Node/browser hosts and a separately translated Wasmtime artifact.
- Reproducible source identities, generated protocol evidence, checksums, and corresponding-source release assets.

### Compatibility

- Node.js `>=20` is required by the package.
- Desktop Chromium, Firefox, and WebKit Worker environments are covered by CI.
- Wasmtime support uses the separately published artifact and Wasmtime 47.0.3 in CI.

### Known limits

Visualization, pthread/TBB execution, generic OCAF authoring, mobile browser memory
limits, and OCCT toolkits without a public operation are outside the v1 surface. See
the [capability matrix](docs/capabilities.md) and [host support](docs/hosts.md).
