import { createHash } from "node:crypto";
import { readdir, readFile, writeFile } from "node:fs/promises";
import { join, posix, relative, sep } from "node:path";

// Builds candidate.json for an immutable release candidate produced by a successful
// main CI run. The tag workflow promotes this candidate instead of rebuilding, so
// everything the tag needs to trust it lives here: the commit it was built from, the
// tag it may be promoted to, the source identities, and a hash of every file shipped.
//
// The expected tag is derived rather than passed in: release.yml already refuses any
// tag that is not "v" + the package version, so the candidate can be packed with its
// final release URLs long before the tag exists.

const CANDIDATE_FILE = "candidate.json";

function argument(name, { required = true } = {}) {
  const index = process.argv.indexOf(`--${name}`);
  if (index === -1 || process.argv[index + 1] === undefined) {
    if (required) throw new Error(`Missing required argument --${name}`);
    return undefined;
  }
  return process.argv[index + 1];
}

const directory = argument("dir");
const commit = argument("commit");
const runId = argument("run-id");
const tarball = argument("tarball");
const integrity = argument("integrity");

if (!/^[0-9a-f]{40}$/.test(commit)) {
  throw new Error(`--commit must be a full 40-character SHA, got "${commit}"`);
}

const repoRoot = new URL("..", import.meta.url);
const packageJson = JSON.parse(await readFile(new URL("package.json", repoRoot), "utf8"));
const identity = JSON.parse(await readFile(new URL("protocol/artifacts.json", repoRoot), "utf8"));

const entries = await readdir(directory, { recursive: true, withFileTypes: true });
const files = {};
for (const entry of entries) {
  if (!entry.isFile()) continue;
  const absolute = join(entry.parentPath ?? entry.path, entry.name);
  const name = relative(directory, absolute).split(sep).join(posix.sep);
  if (name === CANDIDATE_FILE) continue;
  const bytes = await readFile(absolute);
  files[name] = {
    sizeBytes: bytes.byteLength,
    sha256: createHash("sha256").update(bytes).digest("hex"),
  };
}

if (!Object.prototype.hasOwnProperty.call(files, tarball)) {
  throw new Error(`npm tarball ${tarball} is not present in ${directory}`);
}

const candidate = {
  candidateVersion: 1,
  commit,
  version: packageJson.version,
  expectedTag: `v${packageJson.version}`,
  workflowRunId: runId,
  createdAt: new Date().toISOString(),
  sources: Object.fromEntries(
    Object.entries(identity.sources).map(([name, source]) => [
      name,
      { path: source.path, version: source.version, commit: source.commit },
    ]),
  ),
  npm: { filename: tarball, integrity, sha256: files[tarball].sha256 },
  files,
};

await writeFile(join(directory, CANDIDATE_FILE), `${JSON.stringify(candidate, null, 2)}\n`);
console.log(
  `${CANDIDATE_FILE}: ${candidate.expectedTag} from ${commit}, ${Object.keys(files).length} files`,
);
