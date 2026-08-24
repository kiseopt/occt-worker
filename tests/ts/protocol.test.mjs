import assert from "node:assert/strict";
import { createHash } from "node:crypto";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { ShapeHandle } from "../../dist/client.js";
import { DirectClient } from "../../dist/direct-client.js";
import { HISTORY_SUPPORT, OPERATION_CONTRACTS, OPERATIONS, PROTOCOL_SEMANTICS } from "../../dist/generated.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("protocol contracts and history classifications cover the frozen catalog", async () => {
  const contractsBytes = await readFile(new URL("../../protocol/operation-contracts.json", import.meta.url));
  const operationsBytes = await readFile(new URL("../../protocol/operations.json", import.meta.url));
  const errorsBytes = await readFile(new URL("../../protocol/errors.json", import.meta.url));
  const contracts = JSON.parse(contractsBytes);
  const schema = JSON.parse(await readFile(new URL("../../protocol/protocol.schema.json", import.meta.url)));
  const snapshot = JSON.parse(await readFile(new URL("../../protocol/compatibility.snapshot.json", import.meta.url)));
  assert.deepEqual(Object.keys(contracts.operations).sort(), [...OPERATIONS].sort());
  assert.deepEqual(Object.keys(HISTORY_SUPPORT).sort(), [...OPERATIONS].sort());
  assert.deepEqual(OPERATION_CONTRACTS, contracts.operations);
  assert.deepEqual(PROTOCOL_SEMANTICS, contracts.semantics);
  assert.deepEqual(schema["x-protocolSemantics"], contracts.semantics);
  assert.deepEqual(Object.keys(schema["x-operationContracts"]).sort(), [...OPERATIONS].sort());
  const assertSchemaNodesAreObjects = (value, path = "schema") => {
    assert.notEqual(typeof value, "string", `${path} must be a JSON Schema object or boolean`);
    if (value === null || typeof value !== "object") return;
    for (const [key, child] of Object.entries(value)) {
      if (["properties", "$defs"].includes(key)) {
        for (const [name, definition] of Object.entries(child)) {
          assertSchemaNodesAreObjects(definition, `${path}.${key}.${name}`);
        }
      } else if (["items", "contains", "not", "if", "then", "else"].includes(key)) {
        assertSchemaNodesAreObjects(child, `${path}.${key}`);
      } else if (["oneOf", "anyOf", "allOf", "prefixItems"].includes(key)) {
        child.forEach((definition, index) => assertSchemaNodesAreObjects(definition, `${path}.${key}[${index}]`));
      }
    }
  };
  assertSchemaNodesAreObjects(schema);
  assert.equal(schema.$defs.request.oneOf.length, OPERATIONS.length);
  assert.equal(schema.$defs.makeEdgeBSplineArgs.properties.mode.enum.join(","), "interpolate,controlPoints");
  assert.equal(schema.$defs.exportXCAFArgs.required.includes("format"), false);
  assert.equal(schema.$defs.importXCAFArgs.required.includes("format"), false);
  assert.equal(PROTOCOL_SEMANTICS.matrix12.order, "row-major 3x4");
  assert.equal(PROTOCOL_SEMANTICS.massProperties.inertiaOrder.join(","), "Ixx,Ixy,Ixz,Iyx,Iyy,Iyz,Izx,Izy,Izz");
  assert.match(PROTOCOL_SEMANTICS.meshGroups.faceGroups, /scalar u32 entries/);
  assert.match(PROTOCOL_SEMANTICS.brep.callerResponsibility, /wasm SHA-256/);
  assert.deepEqual({
    boundary: PROTOCOL_SEMANTICS.defaults["shapeUpgrade.boundaryCriterion"],
    pcurve: PROTOCOL_SEMANTICS.defaults["shapeUpgrade.pcurveCriterion"],
    surface: PROTOCOL_SEMANTICS.defaults["shapeUpgrade.surfaceCriterion"],
  }, { boundary: "c1", pcurve: "c1", surface: "c1" });
  assert.equal(Object.hasOwn(PROTOCOL_SEMANTICS.defaults, "shapeUpgrade.continuityCriterion"), false);
  assert.deepEqual({
    stl: [PROTOCOL_SEMANTICS.defaults["exportSTL.encoding"], PROTOCOL_SEMANTICS.defaults["importSTL.includeMesh"]],
    ply: [PROTOCOL_SEMANTICS.defaults["exportPLY.encoding"], PROTOCOL_SEMANTICS.defaults["importPLY.includeMesh"]],
    vrml: [PROTOCOL_SEMANTICS.defaults["exportVRML.version"], PROTOCOL_SEMANTICS.defaults["importVRML.includeMesh"]],
    documents: [PROTOCOL_SEMANTICS.defaults["importOBJ.includeDocument"], PROTOCOL_SEMANTICS.defaults["importGLTF.includeDocument"]],
    xcaf: [PROTOCOL_SEMANTICS.defaults["exportXCAF.format"], PROTOCOL_SEMANTICS.defaults["importXCAF.format"]],
  }, {
    stl: ["binary", false], ply: ["ascii", false], vrml: [2, false],
    documents: [false, false], xcaf: ["bin", "bin"],
  });
  const digest = createHash("sha256")
    .update(errorsBytes)
    .update(operationsBytes)
    .update(contractsBytes)
    .digest("hex");
  assert.equal(snapshot.protocolVersion, "1.0.0");
  assert.equal(digest, snapshot.sha256);
});

test("generated operation catalog matches capabilities and rejects unsupported history", async () => {
  const client = await DirectClient.create(wasm);
  const capabilities = await client.initialize();
  assert.deepEqual(capabilities.ops, [...OPERATIONS]);
  assert.deepEqual(capabilities.historySupport, HISTORY_SUPPORT);

  const scope = await client.beginScope();
  const box = await scope.makeBox([10, 10, 10]);
  const fillet = await client.request("fillet", {
    scopeId: scope.scopeId,
    shape: box,
    edgeIndices: [0],
    radius: 0.5,
    includeHistory: true,
  });
  assert.ok(fillet.history);
  assert.deepEqual(Object.keys(fillet.history).sort(), ["deleted", "generated", "modified", "retained"]);
  await assert.rejects(
    client.request("bbox", { shape: box, includeHistory: true }),
    (error) => error.code === "InvalidArgs" && /History is unsupported/.test(error.message),
  );
  await scope.end();
});

test("batch preserves its completed prefix and memory growth leaves no live buffers", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const batch = await client.request("batch", {
    scopeId: scope.scopeId,
    ops: [
      { op: "makeBox", args: { size: [1, 1, 1] } },
      { op: "unknownOperation", args: {} },
      { op: "makeSphere", args: { radius: 1 } },
    ],
  });
  assert.equal(batch.results.length, 1);
  assert.equal(batch.error.code, "ProtocolError");
  assert.deepEqual(batch.error.details, {
    operation: "batch",
    nestedOperation: "unknownOperation",
  });
  assert.equal((await client.stats()).liveShapeHandles, 1);
  assert.deepEqual(
    (await client.request("bbox", { shape: batch.results[0].shape })).max.map(Math.round),
    [1, 1, 1],
  );
  await scope.end();

  const before = await client.stats();
  const buffer = await client.request("createBuffer", { byteLength: before.wasmMemorySize + 65536 });
  assert.equal(buffer.data.byteLength, before.wasmMemorySize + 65536);
  const after = await client.stats();
  assert.ok(after.wasmMemorySize > before.wasmMemorySize);
  assert.equal(after.liveBufferBytes, 0);
});

test("DirectClient explicit buffers survive memory growth and support reusable protocol input", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const poles = await client.createBuffer(6 * Float64Array.BYTES_PER_ELEMENT);
  new Float64Array(poles.view().buffer, poles.view().byteOffset, 6).set([0, 0, 0, 2, 1, 0]);

  const before = await client.stats();
  const growth = await client.createBuffer(before.wasmMemorySize + 65536);
  assert.deepEqual(
    [...new Float64Array(poles.view().buffer, poles.view().byteOffset, 6)],
    [0, 0, 0, 2, 1, 0],
  );

  const edge = await client.request("makeEdgeBSpline", {
    scopeId: scope.scopeId,
    poles: { bufferId: poles.bufferId },
  });
  const domain = await client.request("curveDomain", { shape: edge.shape });
  assert.ok(domain.last > domain.first);
  assert.equal((await client.stats()).liveBufferBytes, poles.byteLength + growth.byteLength);

  await client.freeBuffer(poles);
  await client.freeBuffer(growth);
  assert.throws(() => poles.view(), /released/);
  assert.equal((await client.stats()).liveBufferBytes, 0);
  await scope.end();
});

test("typed batch results preserve shape provenance and raw handles cannot be forged", async () => {
  const client = await DirectClient.create(wasm);
  assert.equal(client.shape, undefined);
  assert.throws(() => new ShapeHandle(client, client.epoch, 1), TypeError);
  assert.throws(() => client.shapeFromKernel(1), TypeError);
  const scope = await client.beginScope();
  const batch = await scope.batch([
    { op: "makeBox", args: { size: [2, 3, 4] } },
    { op: "translate", args: { shape: { $ref: 0 }, translation: [1, 0, 0] } },
  ]);
  assert.equal(batch.error, undefined);
  assert.ok(batch.results[1].shape instanceof ShapeHandle);
  assert.deepEqual((await client.bbox(batch.results[1].shape)).max.map(Math.round), [3, 3, 4]);
  await scope.end();
});

test("BSpline rejects modes outside the frozen protocol enum", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  await assert.rejects(
    scope.makeEdgeBSpline([[0, 0, 0], [1, 1, 0]], { mode: "approximate" }),
    TypeError,
  );
  await assert.rejects(
    client.request("makeEdgeBSpline", {
      scopeId: scope.scopeId,
      poles: [[0, 0, 0], [1, 1, 0]],
      mode: "approximate",
    }),
    TypeError,
  );
  await scope.end();
});
