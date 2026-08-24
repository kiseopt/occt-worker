import { mkdir, readFile, writeFile } from "node:fs/promises";

const source = JSON.parse(await readFile(new URL("../protocol/errors.json", import.meta.url), "utf8"));
const operations = JSON.parse(await readFile(new URL("../protocol/operations.json", import.meta.url), "utf8"));
const contracts = JSON.parse(await readFile(new URL("../protocol/operation-contracts.json", import.meta.url), "utf8"));
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
const quoted = source.errors.map((code) => `"${code}"`).join(" | ");
const cppItems = source.errors.map((code) => `  ${code},`).join("\n");
const cppOperations = operations.operations.map((operation) => `  "${operation}",`).join("\n");
const cppHistory = Object.entries(operations.historySupport)
  .map(([operation, support]) => `  {"${operation}", "${support}"},`)
  .join("\n");

await mkdir(new URL("../kernel/include/", import.meta.url), { recursive: true });
await mkdir(new URL("../ts/src/", import.meta.url), { recursive: true });

await writeFile(
  new URL("../ts/src/generated.ts", import.meta.url),
  `// Generated from protocol/errors.json, protocol/operations.json, and protocol/operation-contracts.json.\nexport const PROTOCOL_VERSION = "${source.protocolVersion}" as const;\nexport type KernelErrorCode = ${quoted};\nexport const OPERATIONS = ${JSON.stringify(operations.operations)} as const;\nexport type OperationName = typeof OPERATIONS[number];\nexport type HistorySupportLevel = "full" | "partial" | "unsupported";\nexport const HISTORY_SUPPORT = ${JSON.stringify(operations.historySupport)} as const satisfies Record<OperationName, HistorySupportLevel>;\nexport const PROTOCOL_SEMANTICS = ${JSON.stringify(contracts.semantics)} as const;\nexport const OPERATION_CONTRACTS = ${JSON.stringify(contracts.operations)} as const;\n`,
);

await writeFile(
  new URL("../kernel/include/generated.hpp", import.meta.url),
  `// Generated from protocol/errors.json and protocol/operations.json.\n#pragma once\n\n#include <array>\n#include <string_view>\n\nnamespace occt_worker {\ninline constexpr std::string_view kProtocolVersion = "${source.protocolVersion}";\nenum class ErrorCode {\n${cppItems}\n};\ninline constexpr std::array<const char*, ${operations.operations.length}> kOperationNames = {\n${cppOperations}\n};\nstruct HistorySupportEntry { const char* operation; const char* support; };\ninline constexpr std::array<HistorySupportEntry, ${Object.keys(operations.historySupport).length}> kHistorySupport = {{\n${cppHistory}\n}};\n}\n`,
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
const errorSchema = schema.$defs.error;
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
