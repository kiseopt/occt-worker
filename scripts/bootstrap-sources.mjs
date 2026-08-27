import { spawnSync } from "node:child_process";
import { mkdir, readFile, readdir } from "node:fs/promises";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const root = fileURLToPath(new URL("..", import.meta.url));
const identity = JSON.parse(await readFile(new URL("../protocol/artifacts.json", import.meta.url), "utf8"));
const sources = Object.values(identity.sources ?? {});

const git = (args, cwd = root) => {
  const result = spawnSync("git", args, { cwd, encoding: "utf8" });
  if (result.status !== 0) throw new Error(result.stderr.trim() || `git ${args.join(" ")} failed`);
  return result.stdout.trim();
};

for (const { path: relativePath, url, commit } of sources) {
  const destination = join(root, relativePath);
  await mkdir(dirname(destination), { recursive: true });
  let entries = [];
  try {
    entries = await readdir(destination);
  } catch (error) {
    if (error.code !== "ENOENT") throw error;
  }
  if (entries.length === 0) {
    git(["clone", "--no-checkout", url, destination]);
    git(["checkout", "--detach", commit], destination);
  }
  const actual = git(["rev-parse", "HEAD"], destination);
  if (actual !== commit) throw new Error(`${relativePath} is ${actual}; expected ${commit}`);
}

const patchResult = spawnSync(process.execPath, [join(root, "scripts/apply-occt-patches.mjs")], {
  cwd: root,
  encoding: "utf8",
  stdio: "inherit",
});
if (patchResult.status !== 0) throw new Error("Failed to apply OCCT wasm patch");

console.log(JSON.stringify(Object.fromEntries(sources.map(({ path, commit }) => [path, commit])), null, 2));
