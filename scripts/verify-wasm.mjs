import { createHash } from "node:crypto";
import { readFile, writeFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import { resolve } from "node:path";

const wasmPath = new URL("../wasm/occt-worker.wasm", import.meta.url);
const bytes = await readFile(wasmPath);
const repositoryPath = resolve(fileURLToPath(new URL("../", import.meta.url)));
const embeddedText = bytes.toString("latin1");
const embeddedRepositoryPath = [repositoryPath, repositoryPath.replaceAll("\\", "/")]
  .find((candidate) => embeddedText.includes(candidate));
if (embeddedRepositoryPath !== undefined) {
  throw new Error(
    `wasm/occt-worker.wasm contains the build path ${embeddedRepositoryPath}; compile with a file prefix map before publishing`,
  );
}
const expectedSurface = JSON.parse(await readFile(new URL("../protocol/wasm-surface.json", import.meta.url), "utf8"));
const module = new WebAssembly.Module(bytes);
const imports = WebAssembly.Module.imports(module).map(({ module: namespace, name, kind }) => ({
  module: namespace,
  name,
  kind,
}));
const exports = WebAssembly.Module.exports(module).map(({ name, kind }) => ({ name, kind }));
const sortedImports = [...imports].sort((a, b) => JSON.stringify(a).localeCompare(JSON.stringify(b)));
const sortedExpectedImports = [...expectedSurface.imports].sort((a, b) => JSON.stringify(a).localeCompare(JSON.stringify(b)));
const sortedExports = [...exports].sort((a, b) => JSON.stringify(a).localeCompare(JSON.stringify(b)));
const sortedExpectedExports = [...expectedSurface.exports].sort((a, b) => JSON.stringify(a).localeCompare(JSON.stringify(b)));
if (JSON.stringify(sortedImports) !== JSON.stringify(sortedExpectedImports)) {
  throw new Error("Wasm import surface differs from protocol/wasm-surface.json");
}
if (JSON.stringify(sortedExports) !== JSON.stringify(sortedExpectedExports)) {
  throw new Error("Wasm export surface differs from protocol/wasm-surface.json");
}
const requiredExports = [
  "k_alloc",
  "k_free",
  "k_handle",
  "k_response_ptr",
  "k_buffer_ptr",
  "k_buffer_len",
  "_initialize",
];
for (const name of requiredExports) {
  if (!exports.some((item) => item.name === name)) throw new Error(`Missing wasm export: ${name}`);
}

const report = {
  artifact: "wasm/occt-worker.wasm",
  standaloneReactor: true,
  sizeBytes: bytes.byteLength,
  sha256: createHash("sha256").update(bytes).digest("hex"),
  imports,
  exports,
  hostPolicy: {
    filesystem: "no protocol operation uses filesystem; allowlisted fd/syscall imports are deterministic host stubs",
    clock: "clock_time_get is supplied by a deterministic monotonic host stub",
    random: "no random import",
    wasmtime: "this browser/Node artifact uses legacy wasm-EH; use wasm/occt-worker.wasmtime.wasm in Wasmtime",
  },
};
await writeFile(new URL("../docs/g0-build.json", import.meta.url), `${JSON.stringify(report, null, 2)}\n`);
if (!process.argv.includes("--quiet")) {
  console.log(JSON.stringify({ sizeBytes: report.sizeBytes, sha256: report.sha256, imports }, null, 2));
}
