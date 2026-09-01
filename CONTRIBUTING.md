# Contributing

[中文说明](CONTRIBUTING.zh-CN.md)

Contributions are welcome within the operation and host boundaries documented in `README.md` and `docs/hosts.md`. There is no support SLA, and roadmap decisions remain with the maintainers.

1. Initialize the three pinned submodules with `git submodule update --init --recursive`.
2. Run `npm ci`, `npm run build:wasm`, and `npm test`.
3. For host-facing changes, also install Chromium with `npx playwright install chromium` and run `npm run test:browser`.
4. Add one representative success test for a new mechanism and a failure test only when it introduces a distinct failure mode.
5. Keep protocol changes synchronized through `protocol/`; run `npm run generate` rather than editing generated files directly.

## Common development commands

| Command | Purpose |
| --- | --- |
| `npm run generate` | Regenerate protocol TypeScript types and profile definitions |
| `npm run build` | Build protocol definitions and compile TypeScript (`dist/`) |
| `npm run build:wasm` | Bootstrap toolchains and build the full release WebAssembly kernel |
| `npm run build:wasm:debug` | Build the debug WebAssembly kernel with source symbols |
| `npm test` | Run TypeScript unit test suite |
| `npm run test:browser` | Run default browser worker tests (requires Chromium) |
| `npm run test:browser:all` | Run browser tests across Chromium, Firefox, and WebKit |
| `npm run test:package` | Validate packed npm consumer integration |

## Repository map

- `kernel/`: C++ modeling, query, exchange, and wasm boundary implementation.
- `protocol/`: authoritative operation definitions, schemas, errors, and generated protocol inputs.
- `ts/src/`: public TypeScript clients, types, modeling helpers, and generated declarations.
- `tests/`: TypeScript behavior tests, browser/Wasmtime checks, and packed npm consumer tests.
- `scripts/`: pinned toolchain bootstrap, wasm builds, protocol generation, and artifact verification.
- `docs/`: public API, protocol, capability, host, build evidence, and release documentation.
- `wasm/`: release runtime artifacts; generated files must pass the verification scripts before being committed.

For a protocol or operation change, update the authoritative files under `protocol/`, run `npm run generate`, then add the smallest relevant kernel, TypeScript, and consumer coverage. Keep the public capability matrix and host documentation aligned with the behavior that is actually verified.

Independent project files are contributed under Apache-2.0. Changes copied from or applied to OCCT remain under OCCT's LGPL-2.1 plus exception terms and must retain their notices.
