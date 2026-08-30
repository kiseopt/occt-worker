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

// 3. Implementation units own sources/toolkits and form an acyclic dependency graph.
const units = modulesDoc.implementationUnits ?? {};
check(Object.keys(units).length > 0, "implementationUnits must not be empty");
const moduleOwners = new Map();
const candidates = {};
for (const [unitId, unit] of Object.entries(units)) {
  for (const field of ["modules", "sources", "toolkits", "requires"]) {
    check(Array.isArray(unit[field]) && unit[field].every((value) => typeof value === "string"),
      `implementation unit '${unitId}' ${field} must be an array of strings`);
    check(new Set(unit[field] ?? []).size === (unit[field] ?? []).length,
      `implementation unit '${unitId}' ${field} must not contain duplicates`);
  }
  check((unit.modules ?? []).length > 0, `implementation unit '${unitId}' must own at least one semantic module`);
  for (const moduleName of unit.modules ?? []) {
    check(moduleName in semanticModules, `implementation unit '${unitId}' references unknown semantic module '${moduleName}'`);
    if (moduleOwners.has(moduleName)) {
      errors.push(`semantic module '${moduleName}' is claimed by both '${moduleOwners.get(moduleName)}' and '${unitId}'`);
    } else {
      moduleOwners.set(moduleName, unitId);
    }
    if (unit.artifact !== undefined) {
      const assigned = candidates[unit.artifact] ?? [];
      if (!assigned.includes(moduleName)) assigned.push(moduleName);
      candidates[unit.artifact] = assigned;
    }
  }
  check(unit.artifact === undefined || (typeof unit.artifact === "string" && unit.artifact.endsWith(".side.wasm")),
    `implementation unit '${unitId}' must name a .side.wasm artifact`);
  for (const dependency of unit.requires ?? []) {
    check(dependency in units, `implementation unit '${unitId}' requires unknown unit '${dependency}'`);
  }
}
for (const moduleName of Object.keys(semanticModules)) {
  check(moduleOwners.has(moduleName), `semantic module '${moduleName}' is not covered by any implementation unit`);
}

const closureCache = new Map();
const unitClosure = (unitId, stack = []) => {
  if (closureCache.has(unitId)) return closureCache.get(unitId);
  if (stack.includes(unitId)) {
    errors.push(`implementation unit dependency cycle: ${[...stack, unitId].join(" -> ")}`);
    return [];
  }
  const result = [];
  for (const dependency of units[unitId]?.requires ?? []) {
    for (const member of unitClosure(dependency, [...stack, unitId])) if (!result.includes(member)) result.push(member);
  }
  if (!result.includes(unitId)) result.push(unitId);
  closureCache.set(unitId, result);
  return result;
};
for (const unitId of Object.keys(units)) unitClosure(unitId);

// 4. Profiles derive capabilities from their complete implementation-unit closure.
const profiles = modulesDoc.profiles ?? {};
check(Object.keys(profiles).length > 0, "profiles must not be empty");
const profileOps = {};
const profileArtifacts = new Set();
for (const [profileId, profile] of Object.entries(profiles)) {
  const aliasTarget = profile.aliasOf;
  if (aliasTarget !== undefined) {
    check(typeof aliasTarget === "string" && profiles[aliasTarget] !== undefined,
      `profile '${profileId}' aliases unknown profile '${aliasTarget}'`);
    check(profile.artifact === undefined && profile.artifacts === undefined && profile.units === undefined,
      `profile alias '${profileId}' must not define artifact, artifacts, or units`);
    check(profiles[aliasTarget]?.aliasOf === undefined,
      `profile alias '${profileId}' must not point to another alias`);
  }
  const resolved = typeof aliasTarget === "string" && profiles[aliasTarget] !== undefined
    ? profiles[aliasTarget]
    : profile;
  if (aliasTarget === undefined) {
    check(typeof resolved.artifact === "string" && resolved.artifact.endsWith(".wasm"), `profile '${profileId}' must name its standalone artifact`);
    check(!profileArtifacts.has(resolved.artifact), `standalone artifact '${resolved.artifact}' is assigned to more than one profile`);
    profileArtifacts.add(resolved.artifact);
  }
  const declaredUnits = resolved.units ?? [];
  check(Array.isArray(declaredUnits) && declaredUnits.length > 0, `profile '${profileId}' must list at least one implementation unit`);
  check(new Set(declaredUnits).size === declaredUnits.length, `profile '${profileId}' must not list an implementation unit more than once`);
  const closure = [];
  for (const unitId of declaredUnits) {
    check(unitId in units, `profile '${profileId}' references unknown implementation unit '${unitId}'`);
    for (const member of unitClosure(unitId)) if (!closure.includes(member)) closure.push(member);
  }
  const derived = new Set();
  for (const unitId of closure) {
    for (const moduleName of units[unitId]?.modules ?? []) {
      for (const op of semanticModules[moduleName] ?? []) derived.add(op);
    }
  }
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
    implementationUnits: Object.keys(units),
    artifactCandidates: Object.keys(candidates),
    profiles: Object.fromEntries(Object.entries(profileOps).map(([id, set]) => [id, set.size])),
  }, null, 2));
}
