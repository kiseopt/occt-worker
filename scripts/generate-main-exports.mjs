// Generates the shared-Main export manifests from the Side import closure:
//   <out>.txt     : BASE host-ABI exports for emcc -sEXPORTED_FUNCTIONS=@
//   <out>.lld.rsp : full closure as `-Wl,--export=<sym>` lines. emcc expands
//                   the response file itself, then forwards each entry to
//                   wasm-ld without its EXPORTED_FUNCTIONS definedness filter.
//
// Usage: node scripts/generate-main-exports.mjs <out> <side1.wasm> [...]

import { readFile, writeFile } from "node:fs/promises";

const argv = process.argv.slice(2);
const outBase = argv[0];
const sidePaths = argv.slice(1);
if (!outBase || sidePaths.length === 0) {
  console.error("usage: generate-main-exports.mjs <out> <side.wasm> [...]");
  process.exit(2);
}

const BASE_EXPORTS = [
  "_malloc", "_free",
  "_k_alloc", "_k_free", "_k_handle", "_k_response_ptr", "_k_buffer_ptr", "_k_buffer_len",
  "_occt_host_dispatch_v1", "_occt_host_response_ptr", "_occt_host_build_family",
  "_occt_host_register_plugin_v1", "_occt_host_plugin_count", "_occt_host_plugin_error",
  "_occt_host_plugin_name",
  "_occt_worker_progress", "_occt_worker_cancelled",
];

const providedByHost = new Set([
  "memory", "__stack_pointer", "__memory_base", "__table_base", "__cpp_exception",
  "occt_worker_progress", "occt_worker_cancelled",
]);

const closure = new Set();
for (const sidePath of sidePaths) {
  const bytes = await readFile(sidePath);
  const mod = new WebAssembly.Module(bytes);
  const ownExports = new Set(WebAssembly.Module.exports(mod).map((e) => e.name));
  for (const imp of WebAssembly.Module.imports(mod)) {
    if (providedByHost.has(imp.name)) continue;
    if ((imp.kind === "global" && (imp.module === "GOT.func" || imp.module === "GOT.mem")) && ownExports.has(imp.name)) continue;
    closure.add(imp.name);
  }
}

await writeFile(`${outBase}.txt`, BASE_EXPORTS.join("\n") + "\n");
await writeFile(
  `${outBase}.lld.rsp`,
  [...closure].map((entry) => `-Wl,--export=${entry}`).join("\n") + "\n",
);
console.log(`generate-main-exports: base=${BASE_EXPORTS.length} closure=${closure.size} -> ${outBase}.{txt,lld.rsp}`);
