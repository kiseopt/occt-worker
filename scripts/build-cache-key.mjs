import { createHash } from "node:crypto";
import { readFile } from "node:fs/promises";

// Cache key for the OCCT install trees that build-wasm.ps1, build-profiles.ps1 and
// build-shared.ps1 reuse when already present. A key hit has to mean the tree was
// produced from the same OCCT source, by the same Emscripten toolchain, with the same
// flags -- so all three are folded in. build-config.ps1 is the single source of the
// build flags, so hashing that file covers every flag change without restating them.
//
// Deliberately exact-match only: callers must not pass restore-keys. A near-miss here
// would silently link against archives built with different flags, and the resulting
// artifact would still pass every downstream check.

const VARIANTS = new Set(["release", "debug", "shared-pic"]);

let variant = "release";
let emitGithubOutput = false;
for (const argument of process.argv.slice(2)) {
  if (argument.startsWith("--variant=")) {
    variant = argument.slice("--variant=".length);
  } else if (argument === "--github") {
    emitGithubOutput = true;
  } else {
    throw new Error(`Unknown argument: ${argument}`);
  }
}
if (!VARIANTS.has(variant)) {
  throw new Error(`Unknown variant "${variant}"; expected one of ${[...VARIANTS].join(", ")}`);
}

const identity = JSON.parse(
  await readFile(new URL("../protocol/artifacts.json", import.meta.url), "utf8"),
);
const buildConfig = await readFile(new URL("./build-config.ps1", import.meta.url));

const occtCommit = identity.sources.occt.commit;
const emsdkVersion = identity.sources.emsdk.version;
const flagsHash = createHash("sha256").update(buildConfig).digest("hex").slice(0, 16);
const key = `occt-${variant}-src.${occtCommit.slice(0, 12)}-em.${emsdkVersion}-cfg.${flagsHash}`;

if (emitGithubOutput) {
  console.log(`key=${key}`);
} else {
  console.log(key);
}
