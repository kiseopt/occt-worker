// Machine-checks protocol/modules.json against the frozen operation catalog:
//   - every operation appears exactly once across semantic modules
//   - no unknown operation names or omissions
//   - artifact module candidates cover every non-runtime semantic module exactly once
//   - profile capability (derived from its artifacts) is consistent: no orphan
//     operations unreachable by any profile, no duplicate artifact entries
//
// Usage: node scripts/verify-modules.mjs [--quiet]

import { readFile } from "node:fs/promises";

const quiet = process.argv.includes("--quiet");
const readJson = async (path) => JSON.parse(await readFile(new URL(path, import.meta.url), "utf8"));

const operationsDoc = await readJson("../protocol/operations.json");
const modulesDoc = await readJson("../protocol/modules.json");

const errors = [];
const check = (condition, message) => {
  if (!condition) errors.push(message);
};

// 1. Semantic partition.
const catalog = [...operationsDoc.operations];
const catalogSet = new Set(catalog);
const semanticModules = modulesDoc.semanticModules ?? {};

const seen = new Map();
for (const [moduleName, ops] of Object.entries(semanticModules)) {
  for (const op of ops) {
    if (seen.has(op)) {
      errors.push(`operation '${op}' appears in both '${seen.get(op)}' and '${moduleName}'`);
    } else {
      seen.set(op, moduleName);
    }
    if (!catalogSet.has(op)) {
      errors.push(`unknown operation '${op}' in semantic module '${moduleName}' (not in protocol/operations.json)`);
    }
  }
}
for (const op of catalog) {
  if (!seen.has(op)) errors.push(`operation '${op}' is missing from semantic modules`);
}
if (seen.size !== catalog.length) {
  errors.push(`semantic modules cover ${seen.size} unique operations, catalog has ${catalog.length}`);
}

const actualCounts = Object.fromEntries(
  Object.entries(semanticModules).map(([name, ops]) => [name, ops.length]),
);
check(Array.isArray(semanticModules.runtime), "semanticModules must define the runtime module");

// 2. Operation policies reference catalog operations exactly once per policy.
const policies = modulesDoc.operationPolicies ?? {};
for (const operation of policies.cooperativeCancellation ?? []) {
  check(catalogSet.has(operation), `cooperative cancellation references unknown operation '${operation}'`);
}
const failureAssignments = new Map();
for (const [code, operations] of Object.entries(policies.failureCodes ?? {})) {
  for (const operation of operations) {
    check(catalogSet.has(operation), `failure code '${code}' references unknown operation '${operation}'`);
    if (failureAssignments.has(operation)) {
      errors.push(`operation '${operation}' has both '${failureAssignments.get(operation)}' and '${code}' failure codes`);
    }
    failureAssignments.set(operation, code);
  }
}
for (const [operation, dispatch] of Object.entries(policies.documentDispatch ?? {})) {
  check(catalogSet.has(operation), `document dispatch references unknown operation '${operation}'`);
  check(typeof dispatch.argument === "string" && dispatch.argument.length > 0,
    `document dispatch for '${operation}' must name its discriminator argument`);
  check(["obj", "gltf"].includes(dispatch.kind),
    `document dispatch for '${operation}' has unknown kind '${dispatch.kind}'`);
}

// 3. Artifact candidates cover every non-runtime module exactly once.
const candidates = modulesDoc.artifactModuleCandidates ?? {};
const candidateNames = Object.keys(candidates);
check(candidateNames.length > 0, "artifactModuleCandidates must not be empty");
{
  const assigned = new Map();
  for (const [artifact, mods] of Object.entries(candidates)) {
    check(Array.isArray(mods) && mods.length === 1, `artifact candidate '${artifact}' must reference exactly one semantic module`);
    for (const mod of mods) {
      check(mod in semanticModules, `artifact candidate '${artifact}' references unknown semantic module '${mod}'`);
      if (assigned.has(mod)) {
        errors.push(`semantic module '${mod}' is claimed by both '${assigned.get(mod)}' and '${artifact}'`);
      } else {
        assigned.set(mod, artifact);
      }
    }
  }
  for (const moduleName of Object.keys(semanticModules)) {
    if (moduleName === "runtime") continue;
    if (!assigned.has(moduleName)) {
      errors.push(`semantic module '${moduleName}' is not covered by any artifact candidate`);
    }
  }
}

// 4. Profiles derive capabilities from artifacts; runtime is always present.
const profiles = modulesDoc.profiles ?? {};
check(Object.keys(profiles).length > 0, "profiles must not be empty");
const profileOps = {};
const profileArtifacts = new Set();
for (const [profileId, profile] of Object.entries(profiles)) {
  check(typeof profile.artifact === "string" && profile.artifact.endsWith(".wasm"), `profile '${profileId}' must name its standalone artifact`);
  check(!profileArtifacts.has(profile.artifact), `standalone artifact '${profile.artifact}' is assigned to more than one profile`);
  profileArtifacts.add(profile.artifact);
  const artifacts = profile.artifacts ?? [];
  check(Array.isArray(artifacts) && artifacts.length > 0, `profile '${profileId}' must list at least one artifact`);
  const seenArtifacts = new Set();
  const derived = new Set();
  for (const artifact of artifacts) {
    check(!seenArtifacts.has(artifact), `profile '${profileId}' lists artifact '${artifact}' more than once`);
    seenArtifacts.add(artifact);
    check(artifact in candidates, `profile '${profileId}' references unknown artifact '${artifact}'`);
    for (const mod of candidates[artifact] ?? []) {
      for (const op of semanticModules[mod] ?? []) derived.add(op);
    }
  }
  for (const op of semanticModules.runtime ?? []) derived.add(op);
  for (const op of modulesDoc.transferOperations ?? []) derived.add(op);
  profileOps[profileId] = derived;
}

// Every non-runtime operation must be reachable through at least one profile.
for (const [moduleName, ops] of Object.entries(semanticModules)) {
  if (moduleName === "runtime") continue;
  for (const op of ops) {
    const reachable = Object.values(profileOps).some((set) => set.has(op));
    if (!reachable) errors.push(`operation '${op}' (${moduleName}) is unreachable by every profile`);
  }
}

// Runtime operations must exist in every profile.
for (const [profileId, set] of Object.entries(profileOps)) {
  for (const op of semanticModules.runtime ?? []) {
    check(set.has(op), `profile '${profileId}' must always expose runtime operation '${op}'`);
  }
}

if (errors.length > 0) {
  console.error(`verify-modules: ${errors.length} error(s):`);
  for (const message of errors) console.error(`  - ${message}`);
  process.exit(1);
}

if (!quiet) {
  console.log("verify-modules: ok");
  console.log(JSON.stringify({
    operations: catalog.length,
    semanticModules: actualCounts,
    artifactCandidates: candidateNames,
    profiles: Object.fromEntries(Object.entries(profileOps).map(([id, set]) => [id, set.size])),
  }, null, 2));
}
