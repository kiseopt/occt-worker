import { createHash } from "node:crypto";
import { readdir, readFile } from "node:fs/promises";
import { join, posix, relative, sep } from "node:path";

// Verifies a downloaded release candidate before the tag workflow publishes any of it.
// Every check here is a reason to refuse to publish: the candidate must have been built
// from exactly the commit the tag points at, declare exactly this tag, and every byte
// must still hash to what the producing run recorded.
//
// Extra files are rejected as well as missing ones: promoting a candidate must never
// publish a file that the producing run did not record and test.

const CANDIDATE_FILE = "candidate.json";

function argument(name) {
  const index = process.argv.indexOf(`--${name}`);
  if (index === -1 || process.argv[index + 1] === undefined) {
    throw new Error(`Missing required argument --${name}`);
  }
  return process.argv[index + 1];
}

const directory = argument("dir");
const tag = argument("tag");
const commit = argument("commit");

const repoRoot = new URL("..", import.meta.url);
const packageJson = JSON.parse(await readFile(new URL("package.json", repoRoot), "utf8"));
const candidate = JSON.parse(await readFile(join(directory, CANDIDATE_FILE), "utf8"));

const failures = [];
function check(condition, message) {
  if (!condition) failures.push(message);
}

check(candidate.candidateVersion === 1, `unsupported candidateVersion ${candidate.candidateVersion}`);
check(
  candidate.commit === commit,
  `candidate was built from ${candidate.commit}, but tag ${tag} points at ${commit}`,
);
check(
  candidate.expectedTag === tag,
  `candidate declares tag ${candidate.expectedTag}, but this run is publishing ${tag}`,
);
check(
  candidate.version === packageJson.version,
  `candidate version ${candidate.version} does not match package version ${packageJson.version}`,
);
check(
  tag === `v${packageJson.version}`,
  `tag ${tag} does not match package version ${packageJson.version}`,
);

const entries = await readdir(directory, { recursive: true, withFileTypes: true });
const present = new Set();
for (const entry of entries) {
  if (!entry.isFile()) continue;
  const absolute = join(entry.parentPath ?? entry.path, entry.name);
  const name = relative(directory, absolute).split(sep).join(posix.sep);
  if (name === CANDIDATE_FILE) continue;
  present.add(name);
  const recorded = candidate.files[name];
  if (recorded === undefined) {
    failures.push(`${name}: present in candidate but not recorded in ${CANDIDATE_FILE}`);
    continue;
  }
  const bytes = await readFile(absolute);
  const sha256 = createHash("sha256").update(bytes).digest("hex");
  check(
    bytes.byteLength === recorded.sizeBytes,
    `${name}: size ${bytes.byteLength} does not match recorded ${recorded.sizeBytes}`,
  );
  check(sha256 === recorded.sha256, `${name}: sha256 ${sha256} does not match recorded ${recorded.sha256}`);
}
for (const name of Object.keys(candidate.files)) {
  check(present.has(name), `${name}: recorded in ${CANDIDATE_FILE} but missing from the candidate`);
}
check(
  candidate.npm?.filename !== undefined && present.has(candidate.npm.filename),
  `npm tarball ${candidate.npm?.filename} is missing from the candidate`,
);

if (failures.length !== 0) {
  for (const failure of failures) console.error(`candidate: ${failure}`);
  process.exit(1);
}
console.log(
  `candidate verified: ${tag} from ${commit}, ${Object.keys(candidate.files).length} files, npm ${candidate.npm.filename}`,
);
