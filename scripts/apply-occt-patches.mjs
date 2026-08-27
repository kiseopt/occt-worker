import { spawnSync } from "node:child_process";
import { readFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";

const root = fileURLToPath(new URL("..", import.meta.url));
const occt = fileURLToPath(new URL("../occt", import.meta.url));
const identity = JSON.parse(await readFile(new URL("../protocol/artifacts.json", import.meta.url), "utf8"));
const patch = fileURLToPath(new URL(`../${identity.sources.occt.patch}`, import.meta.url));

const git = (args) => spawnSync("git", args, { cwd: occt, encoding: "utf8" });
if (git(["apply", "--reverse", "--check", patch]).status === 0) {
  process.exit(0);
}

const check = git(["apply", "--check", patch]);
if (check.status !== 0) {
  throw new Error(check.stderr.trim() || "OCCT wasm patch does not apply cleanly");
}
const apply = git(["apply", patch]);
if (apply.status !== 0) {
  throw new Error(apply.stderr.trim() || "OCCT wasm patch failed");
}

console.log(`Applied OCCT wasm patch from ${patch.slice(root.length + 1)}`);
