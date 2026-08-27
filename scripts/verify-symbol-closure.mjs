// Symbol closure verifier for the occt-worker shared Main/Side architecture.
//
// Checks, per loaded pair (Main + one or more Side modules):
//   1. every Side import is either a host-provided runtime import or is
//      exported by Main under the exact wasm symbol name
//   2. import/export kinds agree (function/global/tag/memory/table)
//   3. Side modules carry a dylink custom section and no memory/table exports
//   4. exactly one memory + table across the pair; single __cpp_exception tag
//   5. no duplicate exports across sides
//
// Signature-level agreement is enforced at instantiation time by the JS
// WebAssembly API; this static pass catches missing/duplicated symbols first.
//
// Usage:
//   node scripts/verify-symbol-closure.mjs --main <main.wasm> --side <side.wasm> [--side <side2.wasm> ...]

import { readFile } from "node:fs/promises";

const args = process.argv.slice(2);
const mainPath = args[args.indexOf("--main") + 1];
const sidePaths = [];
for (let i = 0; i < args.length; ++i) {
  if (args[i] === "--side") sidePaths.push(args[i + 1]);
}
if (!mainPath || sidePaths.length === 0) {
  console.error("usage: verify-symbol-closure.mjs --main <main.wasm> --side <side.wasm> [--side ...]");
  process.exit(2);
}

function readCustomSections(bytes) {
  // Minimal WASM parser: section id/varint length; collect custom section names.
  let offset = 8; // skip magic+version
  const names = [];
  const readVaruint = () => {
    let result = 0, shift = 0, byte;
    do {
      byte = bytes[offset++];
      result |= (byte & 0x7f) << shift;
      shift += 7;
    } while (byte & 0x80);
    return result;
  };
  while (offset < bytes.length) {
    const id = bytes[offset++];
    const size = readVaruint();
    const start = offset;
    if (id === 0) {
      const nameLen = readVaruint();
      let name = "";
      for (let i = 0; i < nameLen; ++i) name += String.fromCharCode(bytes[start + 1 + i]);
      names.push(name);
    }
    offset = start + size;
  }
  return names;
}

const errors = [];
const check = (condition, message) => {
  if (!condition) errors.push(message);
};

const mainBytes = await readFile(mainPath);
const mainModule = new WebAssembly.Module(mainBytes);
const mainExports = new Map(WebAssembly.Module.exports(mainModule).map((e) => [e.name, e.kind]));
const mainImports = WebAssembly.Module.imports(mainModule);

check(readCustomSections(mainBytes).includes("dylink.0"), "Main module must contain a dylink.0 custom section");

const seenSideExports = new Map();
let totalMemories = (mainExports.get("memory") === "memory" ? 1 : 0);
let totalTables = (mainExports.get("__indirect_function_table") === "table" ? 1 : 0);

// Host-provided imports that never come from Main's export table.
const HOST_PROVIDED = new Set([
  "memory", "__memory_base", "__table_base", "__stack_pointer",
  "__cpp_exception",
  "occt_worker_progress", "occt_worker_cancelled",
]);

for (const sidePath of sidePaths) {
  const bytes = await readFile(sidePath);
  const mod = new WebAssembly.Module(bytes);
  check(readCustomSections(bytes).includes("dylink.0"), `${sidePath}: missing dylink.0 section`);

  for (const e of WebAssembly.Module.exports(mod)) {
    if (e.kind === "memory") { totalMemories += 1; errors.push(`${sidePath}: side modules must not define memory (${e.name})`); }
    if (e.kind === "table") { totalTables += 1; errors.push(`${sidePath}: side modules must not define table (${e.name})`); }
    if (seenSideExports.has(e.name)
        && e.name !== "__wasm_call_ctors"
        && e.name !== "__wasm_apply_data_relocs") {
      errors.push(`${sidePath}: duplicate side export '${e.name}' (also in ${seenSideExports.get(e.name)})`);
    }
    seenSideExports.set(e.name, sidePath);
  }

  for (const imp of WebAssembly.Module.imports(mod)) {
    if (HOST_PROVIDED.has(imp.name)) continue;
    const kind = mainExports.get(imp.name);
    if (kind === undefined) {
      errors.push(`${sidePath}: unresolved import '${imp.name}' (${imp.module}) - not exported by Main`);
    } else if (imp.module === "GOT.func" && kind !== "function") {
      errors.push(`${sidePath}: GOT.func import '${imp.name}' must resolve to a Main function, found ${kind}`);
    } else if (imp.module === "GOT.mem" && kind !== "global") {
      errors.push(`${sidePath}: GOT.mem import '${imp.name}' must resolve to a Main global, found ${kind}`);
    } else if (imp.module !== "GOT.func" && imp.module !== "GOT.mem" && kind !== imp.kind) {
      errors.push(`${sidePath}: import '${imp.name}' kind mismatch: side wants ${imp.kind}, Main exports ${kind}`);
    }
  }
}

check(totalMemories === 1, `exactly one memory must exist across Main+Sides, found ${totalMemories}`);
check(totalTables === 1, `exactly one indirect function table must exist across Main+Sides, found ${totalTables}`);
const exceptionTags = [...mainExports.entries()].filter(([n, k]) => k === "tag");
if (!exceptionTags.some(([n]) => n === "__cpp_exception")) {
  errors.push("Main must export the __cpp_exception tag so sides share the EH runtime");
}

if (errors.length > 0) {
  console.error(`verify-symbol-closure: ${errors.length} error(s):`);
  for (const e of errors) console.error(`  - ${e}`);
  process.exit(1);
}

console.log(JSON.stringify({
  closure: "ok",
  main: mainPath,
  sides: sidePaths,
  mainExports: mainExports.size,
  mainImports: mainImports.length,
}, null, 2));
