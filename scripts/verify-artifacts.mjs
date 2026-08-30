import { createHash } from "node:crypto";
import { readFile, stat, writeFile } from "node:fs/promises";

const root = new URL("../", import.meta.url);
const write = process.argv.includes("--write");
const writeFamilyIndex = process.argv.indexOf("--write-family");
const writeFamily = writeFamilyIndex === -1 ? undefined : process.argv[writeFamilyIndex + 1];
if (writeFamilyIndex !== -1 && !["shared", "isolated"].includes(writeFamily)) {
  throw new Error("--write-family requires 'shared' or 'isolated'");
}
if (writeFamily !== undefined && !write) {
  throw new Error("--write-family requires --write");
}
const releaseBaseIndex = process.argv.indexOf("--release-base-url");
const releaseBaseUrl = releaseBaseIndex === -1 ? undefined : process.argv[releaseBaseIndex + 1];
if (releaseBaseIndex !== -1 && (releaseBaseUrl === undefined || !releaseBaseUrl.endsWith("/"))) {
  throw new Error("--release-base-url requires a URL ending with '/'");
}
const manifestUrl = new URL("protocol/artifacts.json", root);
const [manifest, modules, errors] = await Promise.all([
  readFile(manifestUrl, "utf8").then(JSON.parse),
  readFile(new URL("protocol/modules.json", root), "utf8").then(JSON.parse),
  readFile(new URL("protocol/errors.json", root), "utf8").then(JSON.parse),
]);

const failures = [];
const check = (condition, message) => { if (!condition) failures.push(message); };
if (releaseBaseUrl !== undefined) {
  manifest.releaseBaseUrl = releaseBaseUrl;
  for (const [name, descriptor] of Object.entries(manifest.artifacts ?? {})) {
    descriptor.url = new URL(name, releaseBaseUrl).href;
  }
}
check(manifest.protocolVersion === errors.protocolVersion, "artifact protocolVersion does not match protocol/errors.json");
check(Number.isInteger(manifest.pluginAbiVersion) && manifest.pluginAbiVersion > 0, "pluginAbiVersion must be a positive integer");
check(typeof manifest.releaseBaseUrl === "string" && manifest.releaseBaseUrl.endsWith("/"), "releaseBaseUrl must end with '/'");

const artifactEntries = Object.entries(manifest.artifacts ?? {});
const expectedArtifacts = new Map([
  ["shared-main.mjs", { kind: "shared-glue", buildFamily: manifest.buildFamilies?.shared }],
  ["shared-main.wasm", { kind: "shared-main", buildFamily: manifest.buildFamilies?.shared }],
  ...Object.entries(modules.artifactModuleCandidates ?? {}).map(([name, semanticModules]) => [
    name,
    { kind: "shared-side", buildFamily: manifest.buildFamilies?.shared, semanticModules },
  ]),
  ...Object.entries(modules.profiles ?? {}).filter(([, definition]) => definition.aliasOf === undefined).map(([profile, definition]) => [
    definition.artifact,
    { kind: "isolated-profile", buildFamily: manifest.buildFamilies?.isolated, profile },
  ]),
]);
for (const name of expectedArtifacts.keys()) {
  check(manifest.artifacts?.[name] !== undefined, `${name}: required artifact descriptor missing`);
}
for (const [name, descriptor] of artifactEntries) {
  const expected = expectedArtifacts.get(name);
  check(expected !== undefined, `${name}: artifact is not part of the module topology`);
  if (expected !== undefined) {
    check(descriptor.kind === expected.kind, `${name}: kind must be ${expected.kind}`);
    check(descriptor.buildFamily === expected.buildFamily, `${name}: buildFamily does not match module topology`);
    if (expected.semanticModules !== undefined) {
      const actual = Array.isArray(descriptor.semanticModules) ? [...descriptor.semanticModules].sort() : [];
      const required = [...expected.semanticModules].sort();
      check(JSON.stringify(actual) === JSON.stringify(required), `${name}: semantic module identity mismatch`);
    }
    if (expected.profile !== undefined) {
      check(descriptor.profile === expected.profile, `${name}: profile identity mismatch`);
    }
  }
}
for (const [name, descriptor] of artifactEntries) {
  check(descriptor.url === new URL(name, manifest.releaseBaseUrl).href, `${name}: URL does not match releaseBaseUrl`);
  check(/^[0-9a-f]{64}$/.test(descriptor.sha256 ?? ""), `${name}: invalid SHA-256`);
  check(typeof descriptor.buildFamily === "string" && descriptor.buildFamily.length > 0, `${name}: missing buildFamily`);
  let expectedBuildFamily;
  let knownKind = true;
  switch (descriptor.kind) {
    case "shared-glue":
    case "shared-main":
    case "shared-side":
      expectedBuildFamily = manifest.buildFamilies?.shared;
      break;
    case "isolated-profile":
      expectedBuildFamily = manifest.buildFamilies?.isolated;
      break;
    default:
      knownKind = false;
      failures.push(`${name}: unknown artifact kind ${JSON.stringify(descriptor.kind)}`);
      break;
  }
  if (knownKind) {
    check(descriptor.buildFamily === expectedBuildFamily, `${name}: buildFamily does not match ${descriptor.kind} family`);
  }
  if (write && writeFamily !== undefined && descriptor.buildFamily !== manifest.buildFamilies?.[writeFamily]) {
    continue;
  }
  const path = new URL(`artifacts/${name}`, root);
  try {
    const [bytes, info] = await Promise.all([readFile(path), stat(path)]);
    const actual = createHash("sha256").update(bytes).digest("hex");
    if (write) {
      descriptor.sizeBytes = info.size;
      descriptor.sha256 = actual;
    } else {
      check(info.size === descriptor.sizeBytes, `${name}: size ${info.size} does not match ${descriptor.sizeBytes}`);
      check(actual === descriptor.sha256, `${name}: SHA-256 mismatch`);
    }
  } catch (error) {
    failures.push(`${name}: ${error instanceof Error ? error.message : String(error)}`);
  }
}

if (failures.length > 0) {
  console.error(`verify-artifacts: ${failures.length} error(s)`);
  for (const failure of failures) console.error(`  - ${failure}`);
  process.exit(1);
}
if (write) await writeFile(manifestUrl, `${JSON.stringify(manifest, null, 2)}\n`);
console.log(`verify-artifacts: ok (${artifactEntries.length} artifacts)`);
