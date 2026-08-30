import { mkdir, readFile, writeFile } from "node:fs/promises";

const generatedFrom = "protocol/modules.json";
const modules = JSON.parse(await readFile(new URL("../protocol/modules.json", import.meta.url), "utf8"));
const artifacts = JSON.parse(await readFile(new URL("../protocol/artifacts.json", import.meta.url), "utf8"));
const semanticModules = modules.semanticModules ?? {};
const units = modules.implementationUnits ?? {};
const profiles = modules.profiles ?? {};

const fail = (message) => { throw new Error(`generate-profiles: ${message}`); };
const requireStringArray = (value, field) => {
  if (!Array.isArray(value) || value.some((item) => typeof item !== "string")) {
    fail(`${field} must be an array of strings`);
  }
  if (new Set(value).size !== value.length) fail(`${field} must not contain duplicates`);
};
const appendUnique = (target, values) => {
  for (const value of values) if (!target.includes(value)) target.push(value);
};

if (Object.keys(units).length === 0) fail("implementationUnits must not be empty");
for (const [unitId, unit] of Object.entries(units)) {
  requireStringArray(unit.modules, `implementationUnits.${unitId}.modules`);
  requireStringArray(unit.sources, `implementationUnits.${unitId}.sources`);
  requireStringArray(unit.toolkits, `implementationUnits.${unitId}.toolkits`);
  requireStringArray(unit.requires, `implementationUnits.${unitId}.requires`);
  if (unit.modules.length === 0) fail(`implementation unit '${unitId}' must own at least one semantic module`);
  for (const moduleId of unit.modules) {
    if (semanticModules[moduleId] === undefined) fail(`implementation unit '${unitId}' references unknown semantic module '${moduleId}'`);
  }
  for (const dependency of unit.requires) {
    if (units[dependency] === undefined) fail(`implementation unit '${unitId}' requires unknown unit '${dependency}'`);
  }
  if (unit.artifact !== undefined && (typeof unit.artifact !== "string" || !unit.artifact.endsWith(".side.wasm"))) {
    fail(`implementation unit '${unitId}' has an invalid side artifact`);
  }
}

const closureCache = new Map();
const unitClosure = (unitId, stack = []) => {
  if (closureCache.has(unitId)) return closureCache.get(unitId);
  if (stack.includes(unitId)) fail(`implementation unit dependency cycle: ${[...stack, unitId].join(" -> ")}`);
  const result = [];
  for (const dependency of units[unitId].requires) {
    appendUnique(result, unitClosure(dependency, [...stack, unitId]));
  }
  appendUnique(result, [unitId]);
  closureCache.set(unitId, result);
  return result;
};
for (const unitId of Object.keys(units)) unitClosure(unitId);

const artifactGroups = new Map();
for (const [unitId, unit] of Object.entries(units)) {
  if (unit.artifact === undefined) continue;
  const group = artifactGroups.get(unit.artifact) ?? { units: [], modules: [], sources: [], toolkits: [] };
  appendUnique(group.units, [unitId]);
  appendUnique(group.modules, unit.modules);
  appendUnique(group.sources, unit.sources);
  for (const dependency of unitClosure(unitId)) appendUnique(group.toolkits, units[dependency].toolkits);
  artifactGroups.set(unit.artifact, group);
}

const artifactModuleCandidates = Object.fromEntries(
  [...artifactGroups].map(([artifact, group]) => [artifact, group.modules]),
);
const normalizedProfiles = {};
const buildProfiles = [];
for (const [profileId, profile] of Object.entries(profiles)) {
  const aliasTarget = profile.aliasOf;
  let resolved = profile;
  if (aliasTarget !== undefined) {
    if (typeof aliasTarget !== "string" || profiles[aliasTarget] === undefined) {
      fail(`profile '${profileId}' aliases unknown profile '${aliasTarget}'`);
    }
    if (profile.artifact !== undefined || profile.units !== undefined) {
      fail(`profile alias '${profileId}' must not define artifact or units`);
    }
    resolved = profiles[aliasTarget];
    if (resolved.aliasOf !== undefined) fail(`profile alias '${profileId}' must not point to another alias`);
  }
  if (typeof resolved.artifact !== "string" || !resolved.artifact.endsWith(".wasm")) {
    fail(`profile '${profileId}' must name its standalone artifact`);
  }
  requireStringArray(resolved.units, `profiles.${profileId}.units`);
  const closure = [];
  for (const unitId of resolved.units) {
    if (units[unitId] === undefined) fail(`profile '${profileId}' references unknown unit '${unitId}'`);
    appendUnique(closure, unitClosure(unitId));
  }
  const sideArtifacts = [];
  const sources = [];
  const toolkits = [];
  for (const unitId of closure) {
    const unit = units[unitId];
    if (unit.artifact !== undefined) appendUnique(sideArtifacts, [unit.artifact]);
    appendUnique(sources, unit.sources);
    appendUnique(toolkits, unit.toolkits);
  }
  normalizedProfiles[profileId] = { artifact: resolved.artifact, artifacts: sideArtifacts };
  if (aliasTarget === undefined) {
    buildProfiles.push({
      id: profileId,
      artifact: resolved.artifact,
      target: `profile-${profileId}`,
      units: closure,
      sources,
      toolkits,
    });
  }
}

const sides = [...artifactGroups].map(([artifact, group]) => ({
  artifact,
  target: `shared-side-${artifact.replace(/\.side\.wasm$/, "")}`,
  ...group,
}));
const sharedMainUnits = Object.keys(units).filter((unitId) => units[unitId].artifact === undefined);
const sharedMain = { units: sharedMainUnits, sources: [], toolkits: [] };
for (const unitId of sharedMainUnits) appendUnique(sharedMain.sources, units[unitId].sources);
for (const unitId of Object.keys(units)) appendUnique(sharedMain.toolkits, units[unitId].toolkits);

const topology = {
  generatedFrom,
  semanticModules,
  operationPolicies: modules.operationPolicies ?? {},
  transferOperations: modules.transferOperations ?? [],
  artifactModuleCandidates,
  profiles: normalizedProfiles,
  buildProfiles,
  sides,
  sharedMain,
};

const cmakeIdentifier = (value) => value.replaceAll("-", "_").replaceAll(".", "_");
const cmakeList = (name, values) => `set(${name}\n${values.map((value) => `  "${value}"`).join("\n")}\n)`;
const cmakeLines = [
  "# Generated from protocol/modules.json. Do not edit.",
  cmakeList("OCCT_PROFILE_IDS", buildProfiles.map(({ id }) => id)),
  cmakeList("OCCT_SIDE_IDS", sides.map(({ artifact }) => artifact.replace(/\.side\.wasm$/, ""))),
];
for (const profile of buildProfiles) {
  const key = cmakeIdentifier(profile.id);
  cmakeLines.push(`set(OCCT_PROFILE_${key}_TARGET "${profile.target}")`);
  cmakeLines.push(`set(OCCT_PROFILE_${key}_ARTIFACT "${profile.artifact}")`);
  cmakeLines.push(cmakeList(`OCCT_PROFILE_${key}_SOURCES`, profile.sources));
  cmakeLines.push(cmakeList(`OCCT_PROFILE_${key}_TOOLKITS`, profile.toolkits));
}
for (const side of sides) {
  const id = side.artifact.replace(/\.side\.wasm$/, "");
  const key = cmakeIdentifier(id);
  cmakeLines.push(`set(OCCT_SIDE_${key}_TARGET "${side.target}")`);
  cmakeLines.push(`set(OCCT_SIDE_${key}_ARTIFACT "${side.artifact}")`);
  cmakeLines.push(cmakeList(`OCCT_SIDE_${key}_SOURCES`, side.sources));
  cmakeLines.push(cmakeList(`OCCT_SIDE_${key}_TOOLKITS`, side.toolkits));
}
cmakeLines.push(cmakeList("OCCT_SHARED_MAIN_SOURCES", sharedMain.sources));
cmakeLines.push(cmakeList("OCCT_SHARED_MAIN_TOOLKITS", sharedMain.toolkits));

const profileSemanticModules = Object.fromEntries(Object.entries(normalizedProfiles).map(([profileId, profile]) => {
  const moduleIds = ["runtime"];
  for (const artifact of profile.artifacts) {
    appendUnique(moduleIds, artifactModuleCandidates[artifact] ?? []);
  }
  return [profileId, moduleIds];
}));
const profileOperations = Object.fromEntries(Object.entries(profileSemanticModules).map(([profileId, moduleIds]) => {
  const operations = [...(modules.transferOperations ?? [])];
  for (const moduleId of moduleIds) appendUnique(operations, semanticModules[moduleId] ?? []);
  return [profileId, operations];
}));
const typeName = (value) => value.split("-").map((part) => part[0].toUpperCase() + part.slice(1)).join("");
const operationUnion = (operations) => operations.map((operation) => JSON.stringify(operation)).join(" | ") || "never";
const moduleTypeLines = [];
for (const [moduleId, operations] of Object.entries(semanticModules)) {
  const name = typeName(moduleId);
  const union = operationUnion(operations);
  moduleTypeLines.push(`export type ${name}Ops = Pick<WorkerClient & ShapeScope, Extract<${union}, keyof (WorkerClient & ShapeScope)>>;`);
  moduleTypeLines.push(`type ${name}ClientOps = Pick<WorkerClient, Exclude<Extract<${union}, keyof WorkerClient>, "beginScope">>;`);
  moduleTypeLines.push(`type ${name}ScopeOps = Pick<ShapeScope, Extract<${union}, keyof ShapeScope>>;`);
}
const transferUnion = operationUnion(modules.transferOperations ?? []);
moduleTypeLines.push(`export type TransferOps = Pick<WorkerClient & ShapeScope, Extract<${transferUnion}, keyof (WorkerClient & ShapeScope)>>;`);
moduleTypeLines.push(`type TransferClientOps = Pick<WorkerClient, Extract<${transferUnion}, keyof WorkerClient>>;`);
moduleTypeLines.push(`type TransferScopeOps = Pick<ShapeScope, Extract<${transferUnion}, keyof ShapeScope>>;`);

const profileTypeLines = [];
const profileClientEntries = [];
for (const [profileId, moduleIds] of Object.entries(profileSemanticModules)) {
  const profileName = profileId === "full-profile" ? "Full" : typeName(profileId);
  const operationsName = `${profileName}Operation`;
  const scopeName = `${profileName}Scope`;
  const clientName = `${profileName}Client`;
  const clientParts = ["TransferClientOps", ...moduleIds.map((moduleId) => `${typeName(moduleId)}ClientOps`)];
  const scopeParts = ["ProfileScopeBase", "TransferScopeOps", ...moduleIds.map((moduleId) => `${typeName(moduleId)}ScopeOps`)];
  profileTypeLines.push(`export type ${operationsName} = ${operationUnion(profileOperations[profileId])};`);
  profileTypeLines.push(`export type ${scopeName} = ${scopeParts.join(" & ")};`);
  profileTypeLines.push(`export type ${clientName} = ProfileClientBase<${scopeName}, ${operationsName}> & ${clientParts.join(" & ")};`);
  profileClientEntries.push(`  ${JSON.stringify(profileId)}: ${clientName};`);
}
const profileClients = `// Generated from protocol/modules.json. Do not edit.\n\nimport type { ProtocolRequestArgs, ProtocolResult } from "./client-contract.js";\nimport type { OperationName } from "./generated.js";\nimport type { ShapeScope } from "./shape-scope.js";\nimport type { RequestOptions } from "./types.js";\nimport type { WorkerClient } from "./worker-client.js";\n\nexport const PROFILE_OPERATIONS = ${JSON.stringify(profileOperations, null, 2)} as const;\nexport type GeneratedProfileId = keyof typeof PROFILE_OPERATIONS;\n\ntype ProfileScopeBase = Pick<ShapeScope, "scopeId" | "end">;\ntype ProfileClientBase<Scope, Operations extends OperationName> = Pick<WorkerClient, "epoch" | "initialize" | "requestUnsafe" | "close"> & {\n  beginScope(): Promise<Scope>;\n  request<K extends Operations>(\n    operation: K,\n    args: ProtocolRequestArgs<K>,\n    timeoutOrOptions?: number | RequestOptions,\n  ): Promise<ProtocolResult<K>>;\n};\n\n${moduleTypeLines.join("\n")}\n\n${profileTypeLines.join("\n")}\n\nexport interface ProfileClientMap {\n${profileClientEntries.join("\n")}\n}\n`;

const capabilityRows = Object.entries(normalizedProfiles).map(([profileId, profile]) => {
  const alias = profiles[profileId].aliasOf;
  const modulesForProfile = [];
  for (const artifact of profile.artifacts) appendUnique(modulesForProfile, artifactModuleCandidates[artifact] ?? []);
  return `| \`${profileId}\` | \`${profile.artifact}\` | ${alias === undefined ? "-" : `\`${alias}\``} | ${modulesForProfile.map((name) => `\`${name}\``).join(", ")} | ${profileOperations[profileId].length} |`;
});
const capabilityDoc = `<!-- Generated from protocol/modules.json. Do not edit. -->\n\n# Profile capabilities\n\nCompile-time capability narrowing applies only to the official profiles below. A custom wasm artifact supplied through the LGPL replacement path can differ from these types, so custom artifacts must be checked with runtime \`capabilities()\`.\n\nType narrowing does not remove methods from the underlying runtime object. Code that bypasses TypeScript can still attempt unsupported calls; the kernel capability check remains the runtime enforcement boundary.\n\n| Profile | Artifact | Alias of | Semantic modules | Operations |\n| --- | --- | --- | --- | ---: |\n${capabilityRows.join("\n")}\n`;
const sizeBudgets = {
  generatedFrom,
  profiles: Object.fromEntries(buildProfiles.map(({ id, artifact }) => [id, {
    artifact,
    sizeBytes: artifacts.artifacts?.[artifact]?.sizeBytes ?? 0,
  }])),
};

await Promise.all([
  mkdir(new URL("../kernel/generated/", import.meta.url), { recursive: true }),
  mkdir(new URL("../ts/src/", import.meta.url), { recursive: true }),
  mkdir(new URL("../docs/", import.meta.url), { recursive: true }),
  mkdir(new URL("../.github/", import.meta.url), { recursive: true }),
]);
await Promise.all([
  writeFile(new URL("profile-topology.generated.json", import.meta.url), `${JSON.stringify(topology, null, 2)}\n`),
  writeFile(new URL("../kernel/generated/profile-topology.generated.cmake", import.meta.url), `${cmakeLines.join("\n\n")}\n`),
  writeFile(new URL("../ts/src/profile-clients.generated.ts", import.meta.url), profileClients),
  writeFile(new URL("../docs/profile-capabilities.generated.md", import.meta.url), capabilityDoc),
  writeFile(new URL("../.github/profile-size-budgets.generated.json", import.meta.url), `${JSON.stringify(sizeBudgets, null, 2)}\n`),
]);
