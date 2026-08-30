import { mkdir, readFile, writeFile } from "node:fs/promises";

const source = JSON.parse(await readFile(new URL("../protocol/errors.json", import.meta.url), "utf8"));
const operations = JSON.parse(await readFile(new URL("../protocol/operations.json", import.meta.url), "utf8"));
const contracts = JSON.parse(await readFile(new URL("../protocol/operation-contracts.json", import.meta.url), "utf8"));
const modules = JSON.parse(await readFile(new URL("profile-topology.generated.json", import.meta.url), "utf8"));
const artifacts = JSON.parse(await readFile(new URL("../protocol/artifacts.json", import.meta.url), "utf8"));
const buildFlags = JSON.parse(await readFile(new URL("../protocol/build-flags.json", import.meta.url), "utf8"));
const occtSource = artifacts.sources?.occt;
const emsdkSource = artifacts.sources?.emsdk;
if (artifacts.protocolVersion !== source.protocolVersion) {
  throw new Error("artifacts.json protocolVersion must match errors.json");
}
if (occtSource?.version === undefined || emsdkSource?.version === undefined) {
  throw new Error("artifacts.json must define OCCT and emsdk source identities");
}
if (contracts.semantics?.matrix12 === undefined || contracts.semantics?.defaults === undefined) {
  throw new Error("operation-contracts.json must define frozen protocol semantics and defaults");
}
const operationNames = [...operations.operations];
const contractNames = Object.keys(contracts.operations);
const historyNames = Object.keys(operations.historySupport);
if (JSON.stringify(contractNames.sort()) !== JSON.stringify([...operationNames].sort())) {
  throw new Error("operation-contracts.json must define every operation exactly once");
}
if (JSON.stringify(historyNames.sort()) !== JSON.stringify([...operationNames].sort())) {
  throw new Error("historySupport must classify every operation exactly once");
}
const moduleOperationNames = Object.values(modules.semanticModules).flat();
if (JSON.stringify([...moduleOperationNames].sort()) !== JSON.stringify([...operationNames].sort())) {
  throw new Error("modules.json semantic modules must define every operation exactly once");
}
const policies = modules.operationPolicies ?? {};
const cooperativeOperations = policies.cooperativeCancellation ?? [];
const failurePolicies = Object.entries(policies.failureCodes ?? {})
  .flatMap(([code, names]) => names.map((operation) => ({ operation, code })));
const documentPolicies = Object.entries(policies.documentDispatch ?? {});
const policyOperationNames = [
  ...cooperativeOperations,
  ...failurePolicies.map(({ operation }) => operation),
  ...documentPolicies.map(([operation]) => operation),
];
for (const operation of policyOperationNames) {
  if (!operationNames.includes(operation)) throw new Error(`modules.json policy references unknown operation '${operation}'`);
}
for (const { code } of failurePolicies) {
  if (!source.errors.includes(code)) throw new Error(`modules.json policy references unknown error code '${code}'`);
}
const quoted = source.errors.map((code) => `"${code}"`).join(" | ");
const cppItems = source.errors.map((code) => `  ${code},`).join("\n");
const cppOperations = operations.operations.map((operation) => `  "${operation}",`).join("\n");
const cppHistory = Object.entries(operations.historySupport)
  .map(([operation, support]) => `  {"${operation}", "${support}"},`)
  .join("\n");
const cppFailurePolicies = failurePolicies
  .map(({ operation, code }) => `  {"${operation}", ErrorCode::${code}},`)
  .join("\n");
const cppDocumentPolicies = documentPolicies
  .map(([operation, policy]) => `  {"${operation}", "${policy.argument}", DocumentDispatchKind::${policy.kind === "obj" ? "Obj" : "Gltf"}},`)
  .join("\n");
const transferOperations = modules.transferOperations ?? [];
const cppModuleOperations = Object.entries(modules.semanticModules)
  .map(([moduleName, names]) => {
    const handlerNames = moduleName === "runtime"
      ? [...names, ...transferOperations]
      : names.filter((operation) => !transferOperations.includes(operation));
    const identifier = moduleName
      .split("-")
      .map((part) => part[0].toUpperCase() + part.slice(1))
      .join("");
    const macro = `OCCT_${moduleName.replaceAll("-", "_").toUpperCase()}_OPERATION_TABLE`;
    const entries = handlerNames.map((operation, index) => (
      `  X(${operation})${index + 1 === handlerNames.length ? "" : " \\"}`
    )).join("\n");
    return `#define ${macro}(X) \\\n${entries}\n\n#define OCCT_OPERATION_NAME(theOperation) #theOperation,\ninline constexpr std::array<const char*, ${handlerNames.length}> k${identifier}OperationNames = {\n  ${macro}(OCCT_OPERATION_NAME)\n};\n#undef OCCT_OPERATION_NAME`;
  })
  .join("\n");
const cppSideIdentities = Object.entries(modules.artifactModuleCandidates)
  .map(([artifact, semanticModules]) => {
    return semanticModules.map((semanticModule) => {
      const token = semanticModule.replaceAll("-", "_");
      return `#define OCCT_SIDE_NAME_${token} "${semanticModule}"\n#define OCCT_SIDE_SEMANTIC_MODULE_${token} "${semanticModule}"`;
    }).join("\n");
  })
  .flatMap((value) => value)
  .join("\n");

const artifactDescriptors = Object.fromEntries(Object.entries(artifacts.artifacts).map(([name, descriptor]) => [
  name,
  { name, ...descriptor },
]));
const resolvedProfiles = Object.fromEntries(Object.entries(modules.profiles).map(([profileId, profile]) => {
  if (profile.aliasOf === undefined) return [profileId, profile];
  const target = modules.profiles[profile.aliasOf];
  if (target === undefined || target.aliasOf !== undefined) {
    throw new Error(`profile '${profileId}' has an invalid alias target '${profile.aliasOf}'`);
  }
  return [profileId, target];
}));
const operationSides = Object.fromEntries(Object.entries(modules.artifactModuleCandidates).flatMap(([artifact, semanticModules]) => (
  semanticModules.flatMap((moduleName) => (
    (modules.semanticModules[moduleName] ?? []).map((operation) => [operation, artifact])
  ))
)));
const sharedSides = Object.keys(modules.artifactModuleCandidates).map((name) => artifactDescriptors[name]);
const profileConfigs = Object.fromEntries(Object.entries(resolvedProfiles).map(([profileId, profile]) => {
  const artifact = artifactDescriptors[profile.artifact];
  if (artifact === undefined) throw new Error(`profile '${profileId}' references unknown artifact '${profile.artifact}'`);
  return [profileId, {
    profileId,
    buildFamily: artifacts.buildFamilies.isolated,
    artifact,
  }];
}));
const sharedMainJs = artifactDescriptors["shared-main.mjs"];
const sharedMainWasm = artifactDescriptors["shared-main.wasm"];
if (sharedMainJs === undefined || sharedMainWasm === undefined || sharedSides.some((descriptor) => descriptor === undefined)) {
  throw new Error("artifacts.json must define the shared Main and every Side artifact");
}

const primitiveType = (name) => ({
  boolean: "boolean",
  integer: "number",
  null: "null",
  number: "number",
  string: "string",
}[name]);

const definitionMapName = (mode) => mode === "input"
  ? "ProtocolInputDefinitions"
  : "ProtocolOutputDefinitions";

const schemaType = (schema, mode) => {
  if (typeof schema === "string") {
    return primitiveType(schema)
      ?? `${definitionMapName(mode)}<Shape, Scope, BufferHandle, Buffer>[${JSON.stringify(schema)}]`;
  }
  if (schema === true) return "unknown";
  if (schema === false) return "never";
  if (schema === null || typeof schema !== "object") return "unknown";
  if (schema.$ref !== undefined) {
    const name = schema.$ref.replace("#/$defs/", "");
    return `${definitionMapName(mode)}<Shape, Scope, BufferHandle, Buffer>[${JSON.stringify(name)}]`;
  }
  if (schema.const !== undefined) return JSON.stringify(schema.const);
  if (schema.enum !== undefined) return schema.enum.map((value) => JSON.stringify(value)).join(" | ");
  if (schema.oneOf !== undefined && schema.properties === undefined && schema.type === undefined) {
    return schema.oneOf.map((item) => schemaType(item, mode)).join(" | ");
  }
  if (schema.anyOf !== undefined && schema.properties === undefined && schema.type === undefined) {
    return schema.anyOf.map((item) => schemaType(item, mode)).join(" | ");
  }
  if (schema.allOf !== undefined && schema.properties === undefined && schema.type === undefined) {
    return schema.allOf.map((item) => schemaType(item, mode)).join(" & ");
  }
  if (schema.prefixItems !== undefined) {
    const tuple = `[${schema.prefixItems.map((item) => schemaType(item, mode)).join(", ")}]`;
    return mode === "input" ? `readonly ${tuple}` : tuple;
  }
  if (schema.type === "array" || schema.items !== undefined) {
    const item = schema.items === undefined || schema.items === false ? "unknown" : schemaType(schema.items, mode);
    if (schema.minItems === schema.maxItems && Number.isInteger(schema.minItems) && schema.minItems <= 16) {
      const tuple = `[${Array.from({ length: schema.minItems }, () => item).join(", ")}]`;
      return mode === "input" ? `readonly ${tuple}` : tuple;
    }
    return mode === "input" ? `readonly (${item})[]` : `(${item})[]`;
  }
  if (schema.type === "object" || schema.properties !== undefined) {
    if (schema.properties === undefined) {
      if (schema.additionalProperties !== undefined && schema.additionalProperties !== false) {
        return `{ [name: string]: ${schemaType(schema.additionalProperties, mode)}; }`;
      }
      return schema.maxProperties === 0 ? "Record<string, never>" : "Record<string, unknown>";
    }
    const renderObject = (properties, requiredNames, forbiddenNames = []) => {
      const required = new Set(requiredNames);
      const forbidden = new Set(forbiddenNames);
      const members = Object.entries(properties).map(([name, property]) => (
        forbidden.has(name)
          ? `${JSON.stringify(name)}?: never;`
          : `${JSON.stringify(name)}${required.has(name) ? "" : "?"}: ${schemaType(property, mode)};`
      ));
      if (schema.additionalProperties !== undefined && schema.additionalProperties !== false) {
        members.push(`[name: string]: ${schemaType(schema.additionalProperties, mode)};`);
      }
      return `{ ${members.join(" ")} }`;
    };
    if (schema.oneOf !== undefined) {
      const variantRequired = (variant) => new Set([...(schema.required ?? []), ...(variant.required ?? [])]);
      const variantForbidden = (variant) => {
        const names = new Set();
        const exclusions = variant.not?.anyOf ?? (variant.not === undefined ? [] : [variant.not]);
        for (const exclusion of exclusions) {
          if (exclusion.required?.length === 1) names.add(exclusion.required[0]);
        }
        return names;
      };
      const constrainedValues = (property) => {
        if (property?.const !== undefined) return [property.const];
        return property?.enum;
      };
      const variantsAreExclusive = (left, right) => {
        const leftRequired = variantRequired(left);
        const rightRequired = variantRequired(right);
        const leftForbidden = variantForbidden(left);
        const rightForbidden = variantForbidden(right);
        if ([...leftForbidden].some((name) => rightRequired.has(name))
            || [...rightForbidden].some((name) => leftRequired.has(name))) return true;
        for (const name of Object.keys(left.properties ?? {})) {
          const leftValues = constrainedValues(left.properties[name]);
          const rightValues = constrainedValues(right.properties?.[name]);
          if (leftValues === undefined || rightValues === undefined
              || (!leftRequired.has(name) && !rightRequired.has(name))) continue;
          const rightKeys = new Set(rightValues.map((value) => JSON.stringify(value)));
          if (leftValues.every((value) => !rightKeys.has(JSON.stringify(value)))) return true;
        }
        return false;
      };
      return schema.oneOf.map((variant, variantIndex) => {
        const required = variantRequired(variant);
        let forbiddenAlternatives = [[...variantForbidden(variant)]];
        for (const [otherIndex, other] of schema.oneOf.entries()) {
          if (otherIndex === variantIndex || variantsAreExclusive(variant, other)) continue;
          const choices = (other.required ?? []).filter((name) => !required.has(name));
          if (choices.length === 0) {
            throw new Error("oneOf variants overlap without an exclusive property or required field");
          }
          forbiddenAlternatives = forbiddenAlternatives.flatMap((forbidden) => (
            choices.map((name) => [...forbidden, name])
          ));
        }
        const properties = { ...schema.properties, ...(variant.properties ?? {}) };
        return [...new Set(forbiddenAlternatives.map((forbidden) => (
          renderObject(properties, required, forbidden)
        )))].join(" | ");
      }).join(" | ");
    }
    return renderObject(schema.properties, schema.required ?? []);
  }
  if (schema.type !== undefined) {
    return primitiveType(schema.type)
      ?? `${definitionMapName(mode)}<Shape, Scope, BufferHandle, Buffer>[${JSON.stringify(schema.type)}]`;
  }
  return "unknown";
};

const protocolDefinitionType = (name, definition, mode) => {
  if (name === "shapeHandle") return "Shape";
  if (name === "scopeHandle") return "Scope";
  if (name === "bufferHandle") return "BufferHandle";
  if (name === "bufferRef" || name === "bufferDescriptor") return "Buffer";
  if (name === "jsonValue") return "ProtocolJsonValue";
  if (name === "jsonObject") return "ProtocolJsonObject";
  return schemaType(definition, mode);
};

const protocolDefinitionMap = (mode) => Object.entries(contracts.$defs)
  .map(([name, definition]) => `  ${JSON.stringify(name)}: ${protocolDefinitionType(name, definition, mode)};`)
  .join("\n");

const protocolOperationMap = (member, mode) => Object.entries(contracts.operations)
  .map(([name, contract]) => {
    const schema = member === "args" ? { type: "object", ...contract.args } : contract.result;
    return `  ${JSON.stringify(name)}: ${schemaType(schema, mode)};`;
  })
  .join("\n");

const collectHandlePaths = (schema, path = [], definitions = new Set()) => {
  if (typeof schema === "string") {
    if (schema === "shapeHandle") return [path];
    if (schema === "scopeHandle" || schema === "bufferHandle"
        || schema === "bufferRef" || schema === "bufferDescriptor") return [];
    if (definitions.has(schema)) return [];
    return collectHandlePaths(contracts.$defs[schema], path, new Set([...definitions, schema]));
  }
  if (schema === null || typeof schema !== "object") return [];
  if (schema.$ref !== undefined) {
    const name = schema.$ref.replace("#/$defs/", "");
    if (name === "scopeHandle" || name === "bufferHandle"
        || name === "bufferRef" || name === "bufferDescriptor") return [];
    return collectHandlePaths(name, path, definitions);
  }
  const paths = [];
  for (const [name, property] of Object.entries(schema.properties ?? {})) {
    paths.push(...collectHandlePaths(property, [...path, name], definitions));
  }
  if (schema.items !== undefined && schema.items !== false) {
    paths.push(...collectHandlePaths(schema.items, [...path, "*"], definitions));
  }
  for (const item of schema.prefixItems ?? []) {
    paths.push(...collectHandlePaths(item, [...path, "*"], definitions));
  }
  for (const keyword of ["oneOf", "anyOf", "allOf"]) {
    for (const item of schema[keyword] ?? []) paths.push(...collectHandlePaths(item, path, definitions));
  }
  return paths;
};

const operationResultHandlePaths = Object.fromEntries(Object.entries(contracts.operations).map(([name, contract]) => [
  name,
  [...new Map(collectHandlePaths(contract.result).map((path) => [JSON.stringify(path), path])).values()],
]));

await mkdir(new URL("../kernel/include/", import.meta.url), { recursive: true });
await mkdir(new URL("../ts/src/", import.meta.url), { recursive: true });

await writeFile(
  new URL("../ts/src/build-identity.generated.ts", import.meta.url),
  `// Generated from protocol/artifacts.json and protocol/build-flags.json. Do not edit.\nexport const BUILD_IDENTITY = ${JSON.stringify({ protocolVersion: artifacts.protocolVersion, pluginAbiVersion: artifacts.pluginAbiVersion, occtVersion: occtSource.version, emsdkVersion: emsdkSource.version, releaseBaseUrl: artifacts.releaseBaseUrl, buildFamilies: artifacts.buildFamilies, buildFlags })} as const;\n`,
);

await writeFile(
  new URL("../ts/src/runtime-manifest.generated.ts", import.meta.url),
  `// Generated from protocol/modules.json and protocol/artifacts.json. Do not edit.\nimport type { RuntimeConfig } from "./runtime-config.js";\n\nexport const RUNTIME_CONFIG = ${JSON.stringify({
    modules: {
      semanticModules: modules.semanticModules,
      artifactModuleCandidates: modules.artifactModuleCandidates,
      profiles: resolvedProfiles,
      transferOperations: modules.transferOperations ?? [],
    },
    shared: {
      buildFamily: artifacts.buildFamilies.shared,
      mainJs: sharedMainJs,
      mainWasm: sharedMainWasm,
      sides: sharedSides,
      operationSides,
    },
    profiles: profileConfigs,
  })} as const satisfies RuntimeConfig;\n\nexport type RuntimeProfileId = keyof typeof RUNTIME_CONFIG.profiles;\n`,
);

await writeFile(
  new URL("../kernel/include/build_identity.generated.h", import.meta.url),
  `// Generated from protocol/artifacts.json and protocol/build-flags.json. Do not edit.\n#pragma once\n\n#define OCCT_PLUGIN_ABI_VERSION_V1 ${artifacts.pluginAbiVersion}u\n#define OCCT_SHARED_BUILD_FAMILY "${artifacts.buildFamilies.shared}"\n#define OCCT_SHARED_OCCT_VERSION "${occtSource.version}"\n#define OCCT_SHARED_EMSDK_VERSION "${emsdkSource.version}"\n#define OCCT_BUILD_THREADS ${buildFlags.threads ? 1 : 0}\n#define OCCT_BUILD_SIMD ${buildFlags.simd ? 1 : 0}\n#define OCCT_BUILD_WASM_EXCEPTIONS ${buildFlags.wasmExceptions ? 1 : 0}\n${cppSideIdentities}\n`,
);

await writeFile(
  new URL("../ts/src/generated.ts", import.meta.url),
  `// Generated from protocol/errors.json, protocol/operations.json, protocol/modules.json, and protocol/operation-contracts.json.\nexport const PROTOCOL_VERSION = "${source.protocolVersion}" as const;\nexport type KernelErrorCode = ${quoted};\nexport const OPERATIONS = ${JSON.stringify(operations.operations)} as const;\nexport type OperationName = typeof OPERATIONS[number];\nexport const COOPERATIVE_OPERATIONS = ${JSON.stringify(cooperativeOperations)} as const satisfies readonly OperationName[];\nexport type HistorySupportLevel = "full" | "partial" | "unsupported";\nexport const HISTORY_SUPPORT = ${JSON.stringify(operations.historySupport)} as const satisfies Record<OperationName, HistorySupportLevel>;\nexport const PROTOCOL_SEMANTICS = ${JSON.stringify(contracts.semantics)} as const;\nexport const OPERATION_CONTRACTS = ${JSON.stringify(contracts.operations)} as const;\nexport const OPERATION_RESULT_HANDLE_PATHS = ${JSON.stringify(operationResultHandlePaths)} as const satisfies Record<OperationName, readonly (readonly string[])[]>;\n`,
);

await writeFile(
  new URL("../ts/src/protocol-types.generated.ts", import.meta.url),
  `// Generated from protocol/operation-contracts.json. Do not edit.\n\nexport type ProtocolJsonValue = null | boolean | number | string | readonly ProtocolJsonValue[] | ProtocolJsonObject;\nexport interface ProtocolJsonObject { readonly [name: string]: ProtocolJsonValue }\n\nexport interface ProtocolInputDefinitions<Shape, Scope, BufferHandle, Buffer> {\n  "error": { "code": ${quoted}; "message": string; "details"?: unknown; };\n${protocolDefinitionMap("input")}\n}\n\nexport interface ProtocolOutputDefinitions<Shape, Scope, BufferHandle, Buffer> {\n  "error": { "code": ${quoted}; "message": string; "details"?: unknown; };\n${protocolDefinitionMap("output")}\n}\n\nexport interface ProtocolRequestMap<Shape, Scope, BufferHandle, Buffer> {\n${protocolOperationMap("args", "input")}\n}\n\nexport interface ProtocolResultMap<Shape, Scope, BufferHandle, Buffer> {\n${protocolOperationMap("result", "output")}\n}\n`,
);

await writeFile(
  new URL("../kernel/include/generated.hpp", import.meta.url),
  `// Generated from protocol/errors.json, protocol/operations.json, and protocol/modules.json.\n#pragma once\n\n#include <array>\n#include <string_view>\n\nnamespace occt_worker {\ninline constexpr std::string_view kProtocolVersion = "${source.protocolVersion}";\nenum class ErrorCode {\n${cppItems}\n};\ninline constexpr std::array<const char*, ${operations.operations.length}> kOperationNames = {\n${cppOperations}\n};\n${cppModuleOperations}\nstruct OperationFailurePolicy { const char* operation; ErrorCode code; };\ninline constexpr std::array<OperationFailurePolicy, ${failurePolicies.length}> kOperationFailurePolicies = {{\n${cppFailurePolicies}\n}};\ninline constexpr ErrorCode operationFailureCode(const std::string_view theOperation)\n{\n  for (const auto& aPolicy : kOperationFailurePolicies)\n    if (theOperation == aPolicy.operation) return aPolicy.code;\n  return ErrorCode::KernelError;\n}\nenum class DocumentDispatchKind { None, Obj, Gltf };\nstruct DocumentDispatchPolicy { const char* operation; const char* argument; DocumentDispatchKind kind; };\ninline constexpr std::array<DocumentDispatchPolicy, ${documentPolicies.length}> kDocumentDispatchPolicies = {{\n${cppDocumentPolicies}\n}};\ninline constexpr DocumentDispatchPolicy documentDispatchPolicy(const std::string_view theOperation)\n{\n  for (const auto& aPolicy : kDocumentDispatchPolicies)\n    if (theOperation == aPolicy.operation) return aPolicy;\n  return {"", "", DocumentDispatchKind::None};\n}\nstruct HistorySupportEntry { const char* operation; const char* support; };\ninline constexpr std::array<HistorySupportEntry, ${Object.keys(operations.historySupport).length}> kHistorySupport = {{\n${cppHistory}\n}};\n}\n`,
);

const schemaUrl = new URL("../protocol/protocol.schema.json", import.meta.url);
const schema = JSON.parse(await readFile(schemaUrl, "utf8"));
schema.$id = `https://occt-worker.dev/protocol/${source.protocolVersion}/schema.json`;
schema.title = `occt-worker protocol v${source.protocolVersion}`;
const expandSchema = (value) => {
  if (typeof value === "string") return { $ref: `#/$defs/${value}` };
  if (Array.isArray(value)) return value.map(expandSchema);
  if (value === null || typeof value !== "object") return value;
  const expanded = { ...value };
  if (value.properties !== undefined) {
    expanded.properties = Object.fromEntries(
      Object.entries(value.properties).map(([name, property]) => [name, expandSchema(property)]),
    );
  }
  for (const keyword of ["items", "contains", "not", "if", "then", "else"]) {
    if (value[keyword] !== undefined && typeof value[keyword] !== "boolean") {
      expanded[keyword] = expandSchema(value[keyword]);
    }
  }
  for (const keyword of ["oneOf", "anyOf", "allOf", "prefixItems"]) {
    if (value[keyword] !== undefined) expanded[keyword] = value[keyword].map(expandSchema);
  }
  if (value.type === "object" && value.properties !== undefined && value.additionalProperties === undefined) {
    expanded.additionalProperties = false;
  }
  return expanded;
};
const objectSchema = (contract) => expandSchema({
  type: "object",
  required: contract.required ?? [],
  additionalProperties: false,
  properties: contract.properties ?? {},
  ...(contract.oneOf === undefined ? {} : { oneOf: contract.oneOf }),
});
const errorSchema = {
  type: "object",
  required: ["code", "message"],
  additionalProperties: false,
  properties: {
    code: { enum: [...source.errors] },
    message: { type: "string" },
    details: {},
  },
};
schema.$defs = {
  error: errorSchema,
  ...Object.fromEntries(Object.entries(contracts.$defs).map(([name, definition]) => [name, expandSchema(definition)])),
};
for (const operation of operationNames) {
  schema.$defs[`${operation}Args`] = objectSchema(contracts.operations[operation].args);
  schema.$defs[`${operation}Result`] = expandSchema(contracts.operations[operation].result);
}
schema.$defs.request = {
  oneOf: operationNames.map((operation) => ({
    type: "object",
    required: ["id", "op", "args"],
    additionalProperties: false,
    properties: {
      id: { type: "integer", minimum: 0 },
      op: { const: operation },
      args: { $ref: `#/$defs/${operation}Args` },
    },
  })),
};
schema.$defs.response = {
  oneOf: [
    ...operationNames.map((operation) => ({
      type: "object",
      required: ["id", "ok", "result"],
      additionalProperties: false,
      properties: {
        id: { type: "integer" },
        ok: { const: true },
        result: { $ref: `#/$defs/${operation}Result` },
      },
    })),
    {
      type: "object",
      required: ["id", "ok", "error"],
      additionalProperties: false,
      properties: {
        id: { type: "integer" },
        ok: { const: false },
        error: { $ref: "#/$defs/error" },
      },
    },
  ],
};
schema["x-operationContracts"] = Object.fromEntries(operationNames.map((operation) => [operation, {
  args: `#/$defs/${operation}Args`,
  result: `#/$defs/${operation}Result`,
}]));
schema["x-protocolSemantics"] = contracts.semantics;
await writeFile(schemaUrl, `${JSON.stringify(schema, null, 2)}\n`);
