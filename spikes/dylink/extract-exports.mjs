// Extracts the dynamic import surface of a side module and emits an
// EXPORTED_FUNCTIONS list (@file format) for the Main module.
//
// Usage: node extract-exports.mjs <side.wasm> <out.txt> [baseEntry...]
//
// Every imported symbol must appear in the Main module's export namespace
// under its asm.js-mangled name (one additional leading '_'). Undefined
// entries (e.g. the side's own GOT self-references) are tolerated by emcc.

import { readFile, writeFile } from "node:fs/promises";

const [sidePath, outPath, ...baseEntries] = process.argv.slice(2);
if (!sidePath || !outPath) {
  console.error("usage: node extract-exports.mjs <side.wasm> <out.txt> [baseEntry...]");
  process.exit(2);
}

const bytes = await readFile(sidePath);
const module = new WebAssembly.Module(bytes);

const ownExports = new Set(WebAssembly.Module.exports(module).map((item) => item.name));

const providedByHost = new Set([
  "memory",
  "__stack_pointer",
  "__memory_base",
  "__table_base",
  "__cpp_exception",
]);

const entries = new Set(baseEntries.flatMap((entry) => entry.split(",")));
for (const item of WebAssembly.Module.imports(module)) {
  if (item.kind === "function" && item.module === "env") {
    entries.add(`_${item.name}`);
  } else if (item.kind === "global" && (item.module === "GOT.func" || item.module === "GOT.mem")) {
    // Skip the side's own symbols that reach themselves through a GOT entry;
    // everything else must be provided by Main.
    if (!ownExports.has(item.name)) entries.add(`_${item.name}`);
  }
}

const lines = [...entries].filter((entry) => !providedByHost.has(entry.slice(1)));
await writeFile(outPath, lines.join("\n") + "\n");
console.log(`extract-exports: ${lines.length} entries -> ${outPath}`);
