import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import {
  DirectClient,
  ParametricModel,
  evaluateExpression,
  resolvePersistentSubshape,
  resolveParameters,
} from "../../dist/index.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("safe arithmetic expressions resolve named parameter dependencies", () => {
  assert.equal(evaluateExpression("2 + 3 * 4"), 14);
  assert.equal(evaluateExpression("2 ^ 3 ^ 2"), 512);
  assert.equal(evaluateExpression("-2 ^ 2"), -4);
  assert.ok(Math.abs(evaluateExpression("2 * pi") - 2 * Math.PI) < 1e-12);
  assert.deepEqual(resolveParameters({ width: 4, half: "width / 2", area: "width * half" }), {
    width: 4,
    half: 2,
    area: 8,
  });
  assert.throws(() => evaluateExpression("Math.random()"), SyntaxError);
  assert.throws(() => evaluateExpression("constructor"), /Unknown parameter "constructor"/);
  assert.throws(() => resolveParameters({ constructor: 1 }), /Invalid parameter name "constructor"/);
  assert.throws(
    () => new ParametricModel({}).getParameter("toString"),
    /Parameter "toString" has not been computed/,
  );
  assert.throws(() => resolveParameters({ width: "missing + 1" }), /Unknown parameter "missing"/);
  assert.throws(
    () => resolveParameters({ first: "second + 1", second: "first + 1" }),
    /Parameter dependency cycle: first -> second -> first/,
  );
});

test("persistent subshape resolution composes local histories and diagnoses topology loss", () => {
  const empty = { retained: [], generated: [], modified: [], deleted: [] };
  const first = {
    ...empty,
    retained: [{ from: { input: 0, type: "face", index: 2 }, to: { type: "face", index: 5 } }],
  };
  const second = {
    ...empty,
    modified: [{ from: { input: 0, type: "face", index: 5 }, to: [{ type: "face", index: 7 }] }],
  };
  assert.deepEqual(resolvePersistentSubshape(
    { input: 0, type: "face", index: 2 }, [first, second],
  ), { status: "resolved", type: "face", index: 7, kind: "modified" });

  const ambiguous = {
    ...empty,
    generated: [{
      from: { input: 0, type: "edge", index: 1 },
      to: [{ type: "edge", index: 3 }, { type: "edge", index: 4 }],
    }],
  };
  assert.deepEqual(resolvePersistentSubshape(
    { input: 0, type: "edge", index: 1 }, [ambiguous],
  ), {
    status: "ambiguous",
    stage: 0,
    reason: "multipleTargets",
    candidates: [{ type: "edge", index: 3 }, { type: "edge", index: 4 }],
  });

  const deleted = {
    ...empty,
    deleted: [{ input: 0, type: "face", index: 2 }],
  };
  assert.deepEqual(resolvePersistentSubshape(
    { input: 0, type: "face", index: 2 }, [deleted],
  ), { status: "missing", stage: 0, reason: "deleted" });
});

test("feature DAG rejects unknown dependencies and cycles before allocating a scope", async () => {
  const client = { beginScope: () => { throw new Error("scope must not be allocated"); } };
  const unknown = new ParametricModel(client, {
    parameters: {},
    features: [{ id: "moved", type: "translate", input: "missing", translation: [1, 0, 0] }],
  });
  await assert.rejects(unknown.recompute(), /Unknown feature "missing" referenced by "moved"/);

  const cyclic = new ParametricModel(client, {
    parameters: {},
    features: [
      { id: "first", type: "translate", input: "second", translation: [1, 0, 0] },
      { id: "second", type: "scale", input: "first", factor: 2 },
    ],
  });
  await assert.rejects(cyclic.recompute(), /Feature dependency cycle: first -> second -> first/);

  await assert.rejects(
    ParametricModel.fromJSON(
      client,
      { parameters: {}, features: [{ id: "bad", type: "script" }] },
    ).recompute(),
    /Unsupported feature type "script"/,
  );
});

test("parameter changes recompute an out-of-order feature DAG in a fresh scope", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: { width: 2, depth: "width * 2" },
    features: [
      { id: "moved", type: "translate", input: "base", translation: [1, 0, 0] },
      { id: "base", type: "box", size: ["width", "depth", 3] },
    ],
  });
  await model.recompute();
  const oldShape = model.getShape("moved");
  assert.deepEqual((await client.bbox(oldShape)).max.map(Math.round), [3, 4, 3]);
  assert.equal(model.getParameter("depth"), 4);

  model.setParameter("width", 4);
  await model.recompute();
  assert.deepEqual((await client.bbox(model.getShape("moved"))).max.map(Math.round), [5, 8, 3]);
  assert.equal(model.getParameter("depth"), 8);
  await assert.rejects(client.bbox(oldShape), (error) => error.code === "InvalidHandle");

  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("serialized subshape signatures resolve across recompute and diagnose missing or ambiguous matches", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: { width: 2 },
    features: [{ id: "base", type: "box", size: ["width", 3, 4] }],
    subshapeReferences: [
      { id: "left", feature: "base", type: "face", initialIndex: 0 },
      { id: "edge", feature: "base", type: "edge", initialIndex: 0 },
    ],
  });
  await model.recompute();
  assert.deepEqual(model.getSubshapeReferenceDiagnostics(), [
    { id: "left", feature: "base", type: "face", status: "resolved", index: 0 },
    { id: "edge", feature: "base", type: "edge", status: "resolved", index: 0 },
  ]);
  const serialized = model.toJSON();
  assert.equal(serialized.subshapeReferences[0].signature.geometry, "plane");
  assert.equal(serialized.subshapeReferences[0].signature.normalizedBounds.length, 6);
  assert.equal(serialized.subshapeReferences[1].signature.geometry, "line");
  const oldFace = model.getSubshapeReference("left");
  const oldEdge = model.getSubshapeReference("edge");
  assert.ok((await client.bbox(oldFace)).max[0] < 1e-5);
  assert.equal(await client.shapeType(oldEdge), "edge");

  model.setParameter("width", 7);
  await model.recompute();
  const recomputedFace = model.getSubshapeReference("left");
  assert.ok((await client.bbox(recomputedFace)).max[0] < 1e-5);
  await assert.rejects(client.bbox(oldFace), (error) => error.code === "InvalidHandle");
  await assert.rejects(client.bbox(oldEdge), (error) => error.code === "InvalidHandle");

  const restoredDefinition = model.toJSON();
  restoredDefinition.subshapeReferences[0].initialIndex = 5;
  const restored = ParametricModel.fromJSON(client, restoredDefinition);
  await restored.recompute();
  assert.equal(restored.getSubshapeReferenceDiagnostics()[0].index, 0);
  assert.ok((await client.bbox(restored.getSubshapeReference("left"))).max[0] < 1e-5);

  model.setFeature({ id: "base", type: "sphere", radius: 2 });
  await model.recompute();
  assert.equal(model.getSubshapeReferenceDiagnostics().every(({ status }) => status === "missing"), true);
  assert.throws(() => model.getSubshapeReference("left"), /matches the persisted signature/);

  restoredDefinition.subshapeReferences[0].tolerance = 2;
  const ambiguous = ParametricModel.fromJSON(client, restoredDefinition);
  await ambiguous.recompute();
  const ambiguousDiagnostic = ambiguous.getSubshapeReferenceDiagnostics()[0];
  assert.equal(ambiguousDiagnostic.status, "ambiguous");
  assert.deepEqual(ambiguousDiagnostic.candidateIndices, [0, 1, 2, 3, 4, 5]);
  assert.throws(() => ambiguous.getSubshapeReference("left"), /candidates/);

  const disambiguatedDefinition = structuredClone(restoredDefinition);
  disambiguatedDefinition.subshapeReferences[0].initialIndex = 0;
  disambiguatedDefinition.subshapeReferences[0].disambiguation = "initialIndex";
  const disambiguated = ParametricModel.fromJSON(client, disambiguatedDefinition);
  await disambiguated.recompute();
  assert.deepEqual(disambiguated.getSubshapeReferenceDiagnostics()[0], {
    id: "left", feature: "base", type: "face", status: "resolved", index: 0,
  });
  assert.ok((await client.bbox(disambiguated.getSubshapeReference("left"))).max[0] < 1e-5);

  await ambiguous.dispose();
  await disambiguated.dispose();
  await restored.dispose();
  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("stable subshape references drive downstream feature selections", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: { width: 2, radius: 0.2 },
    features: [
      {
        id: "rounded",
        type: "fillet",
        input: "base",
        edgeIndices: [{ reference: "round-edge" }],
        radius: "radius",
      },
      { id: "base", type: "box", size: ["width", 3, 4] },
    ],
    subshapeReferences: [
      { id: "round-edge", feature: "base", type: "edge", initialIndex: 0 },
    ],
  });

  await model.recompute();
  assert.equal((await client.topologyCounts(model.getShape("rounded"))).face > 6, true);
  assert.equal(model.getSubshapeReferenceDiagnostics()[0].status, "resolved");

  model.setParameter("width", 7);
  await model.recompute();
  assert.equal((await client.topologyCounts(model.getShape("rounded"))).face > 6, true);
  assert.equal(model.getSubshapeReferenceDiagnostics()[0].index, 0);

  const published = model.getShape("rounded");
  model.setFeature({ id: "base", type: "sphere", radius: 2 });
  await assert.rejects(model.recompute(), /not uniquely resolved/);
  assert.equal(model.getShape("rounded"), published);
  assert.equal((await client.topologyCounts(published)).face > 6, true);

  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("parametric subshape references propagate through feature histories", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: { width: 10, holeRadius: 1, filletRadius: 0.3 },
    features: [
      { id: "base", type: "box", size: ["width", 10, 2] },
      { id: "tool", type: "cylinder", radius: "holeRadius", height: 2, origin: [5, 5, 0] },
      { id: "cut", type: "booleanCut", base: "base", tools: ["tool"] },
      {
        id: "rounded",
        type: "fillet",
        input: "cut",
        edgeIndices: [{ reference: "cut-edge" }],
        radius: "filletRadius",
      },
    ],
    subshapeReferences: [
      { id: "base-edge", feature: "base", type: "edge", initialIndex: 0 },
      { id: "cut-edge", feature: "cut", type: "edge", source: "base-edge" },
      { id: "base-face", feature: "base", type: "face", initialIndex: 0 },
      { id: "rounded-face", feature: "rounded", type: "face", source: "base-face" },
      { id: "tool-face", feature: "tool", type: "face", initialIndex: 0 },
      { id: "cut-tool-face", feature: "cut", type: "face", source: "tool-face" },
    ],
  });

  await model.recompute();
  assert.deepEqual(model.getSubshapeReferenceDiagnostics().map(({ id, status }) => ({ id, status })), [
    { id: "base-edge", status: "resolved" },
    { id: "cut-edge", status: "resolved" },
    { id: "base-face", status: "resolved" },
    { id: "rounded-face", status: "resolved" },
    { id: "tool-face", status: "resolved" },
    { id: "cut-tool-face", status: "resolved" },
  ]);
  assert.equal((await client.topologyCounts(model.getShape("rounded"))).face, 8);
  assert.equal(model.toJSON().subshapeReferences[1].source, "base-edge");
  assert.equal(model.toJSON().subshapeReferences[3].source, "base-face");
  assert.equal(model.getSubshapeReferenceDiagnostics()[5].index, 6);

  model.setParameter("width", 12);
  await model.recompute();
  assert.equal(model.getSubshapeReferenceDiagnostics().every(({ status }) => status === "resolved"), true);
  assert.equal((await client.topologyCounts(model.getShape("rounded"))).face, 8);

  const restored = ParametricModel.fromJSON(client, model.toJSON());
  await restored.recompute();
  assert.equal(restored.getSubshapeReferenceDiagnostics().every(({ status }) => status === "resolved"), true);

  await restored.dispose();
  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("parametric subshape references propagate through transform histories", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: { offset: 2, factor: 1.5 },
    features: [
      { id: "base", type: "box", size: [2, 3, 4] },
      { id: "moved", type: "translate", input: "base", translation: ["offset", 0, 0] },
      { id: "rotated", type: "rotate", input: "moved", direction: [0, 0, 1], angle: 0.2 },
      { id: "scaled", type: "scale", input: "rotated", factor: "factor" },
      { id: "mirrored", type: "mirror", input: "scaled", normal: [1, 0, 0] },
      {
        id: "affine",
        type: "generalTransform",
        input: "mirrored",
        matrix: [1, 0.1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0],
      },
    ],
    subshapeReferences: [
      { id: "base-face", feature: "base", type: "face", initialIndex: 0 },
      { id: "moved-face", feature: "moved", type: "face", source: "base-face" },
      { id: "rotated-face", feature: "rotated", type: "face", source: "moved-face" },
      { id: "scaled-face", feature: "scaled", type: "face", source: "rotated-face" },
      { id: "mirrored-face", feature: "mirrored", type: "face", source: "scaled-face" },
      { id: "affine-face", feature: "affine", type: "face", source: "mirrored-face" },
    ],
  });

  await model.recompute();
  assert.equal(model.getSubshapeReferenceDiagnostics().every(({ status }) => status === "resolved"), true);
  model.setParameter("offset", 7);
  model.setParameter("factor", 2);
  await model.recompute();
  assert.equal(model.getSubshapeReferenceDiagnostics().every(({ status }) => status === "resolved"), true);

  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("parametric subshape references report extrusion history ambiguity", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: { height: 4 },
    features: [
      { id: "profile", type: "polygon", points: [[0, 0, 0], [3, 0, 0], [3, 2, 0], [0, 2, 0]] },
      { id: "face", type: "face", outer: "profile" },
      { id: "extruded", type: "extrude", input: "face", vector: [0, 0, "height"] },
    ],
    subshapeReferences: [
      { id: "source-face", feature: "face", type: "face", initialIndex: 0 },
      { id: "extruded-face", feature: "extruded", type: "face", source: "source-face" },
    ],
  });

  await model.recompute();
  assert.deepEqual(model.getSubshapeReferenceDiagnostics().map(({ status }) => status), ["resolved", "ambiguous"]);
  model.setParameter("height", 7);
  await model.recompute();
  assert.deepEqual(model.getSubshapeReferenceDiagnostics().map(({ status }) => status), ["resolved", "ambiguous"]);

  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("persistent subshape references can opt into geometry-family replacement", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: {},
    features: [
      { id: "source", type: "box", size: [2, 3, 4] },
      { id: "converted", type: "translate", input: "source", translation: [0, 0, 0] },
    ],
    subshapeReferences: [
      { id: "strict", feature: "converted", type: "face", initialIndex: 0 },
      {
        id: "replacement",
        feature: "converted",
        type: "face",
        initialIndex: 0,
        allowGeometryReplacement: true,
      },
    ],
  });
  await model.recompute();
  assert.equal((await client.surfaceGeometry(model.getSubshapeReference("replacement"))).type, "plane");

  model.setFeature({
    id: "converted",
    type: "shapeUpgrade",
    input: "source",
    mode: "convertToBezier",
  });
  await model.recompute();
  assert.deepEqual(model.getSubshapeReferenceDiagnostics().map(({ id, status, index }) =>
    ({ id, status, ...(index === undefined ? {} : { index }) })), [
    { id: "strict", status: "missing" },
    { id: "replacement", status: "resolved", index: 0 },
  ]);
  assert.equal((await client.surfaceGeometry(model.getSubshapeReference("replacement"))).type, "bezier");
  assert.equal(model.toJSON().subshapeReferences[1].allowGeometryReplacement, true);
  assert.throws(() => model.getSubshapeReference("strict"), /matches the persisted signature/);

  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("parametric affine and mirror transforms remain serializable and expression-driven", async () => {
  const client = await DirectClient.create(wasm);
  const roundedBounds = ({ min, max }) => ({
    min: min.map((value) => Math.abs(value) < 1e-6 ? 0 : Math.round(value * 1e6) / 1e6),
    max: max.map((value) => Math.abs(value) < 1e-6 ? 0 : Math.round(value * 1e6) / 1e6),
  });
  const definition = {
    parameters: { offset: 5 },
    features: [
      { id: "reflected", type: "mirror", input: "affine", normal: [1, 0, 0] },
      {
        id: "affine",
        type: "generalTransform",
        input: "base",
        matrix: [1, 0, 0, "offset", 0, 1, 0, 0, 0, 0, 1, 0],
      },
      { id: "base", type: "box", size: [2, 3, 4] },
    ],
  };
  const model = ParametricModel.fromJSON(client, JSON.stringify(definition));
  await model.recompute();
  assert.deepEqual(model.toJSON(), { ...definition, schemaVersion: 1 });
  assert.deepEqual(roundedBounds(await client.bbox(model.getShape("affine"))), {
    min: [5, 0, 0], max: [7, 3, 4],
  });
  assert.deepEqual(roundedBounds(await client.bbox(model.getShape("reflected"))), {
    min: [-7, 0, 0], max: [-5, 3, 4],
  });

  model.setParameter("offset", 8);
  await model.recompute();
  assert.deepEqual(roundedBounds(await client.bbox(model.getShape("reflected"))), {
    min: [-10, 0, 0], max: [-8, 3, 4],
  });
  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("parametric assembly maps reusable feature instances to STEP and XCAF documents", async () => {
  const client = await DirectClient.create(wasm);
  const definition = {
    parameters: { width: 2, offset: "width * 3" },
    features: [{ id: "part", type: "box", size: ["width", 3, 4] }],
    document: {
      nodes: [
        {
          kind: "part",
          feature: "part",
          name: "Primary bracket",
          color: [0.8, 0.1, 0.2, 1],
          layers: ["Mechanical"],
          material: { name: "Steel", description: "Structural", density: 7850 },
          visualMaterial: {
            name: "Painted steel", baseColor: [0.8, 0.1, 0.2, 1], metallic: 0.6, roughness: 0.3,
          },
          validationProperties: { area: 52, volume: 24, centroid: [1, 1.5, 2] },
        },
        {
          kind: "part",
          feature: "part",
          name: "Bracket instance",
          transform: [1, 0, 0, "offset", 0, 1, 0, 0, 0, 0, 1, 0],
          visible: false,
        },
        { kind: "assembly", name: "Bracket set", children: [0, 1] },
      ],
      roots: [2],
    },
  };
  const model = ParametricModel.fromJSON(client, JSON.stringify(definition));
  await model.recompute();
  assert.deepEqual(model.toJSON(), { ...definition, schemaVersion: 1 });

  let document = model.toSTEPDocumentDefinition();
  assert.equal(document.nodes[0].shape, model.getShape("part"));
  assert.equal(document.nodes[1].shape, model.getShape("part"));
  assert.equal(document.nodes[1].transform[3], 6);
  assert.equal(document.nodes[0].visualMaterial.name, "Painted steel");
  assert.equal(document.nodes[0].visualMaterial.roughness, 0.3);
  assert.equal(document.nodes[0].validationProperties.volume, 24);
  const step = await client.exportSTEPDocument(document);
  const xcaf = await client.exportXCAF(document);
  assert.ok(step.byteLength > 0);
  assert.ok(xcaf.byteLength > 0);

  const importScope = await client.beginScope();
  const restored = await importScope.importSTEPDocument(step);
  const root = restored.nodes[restored.roots[0]];
  assert.equal(root.name, "Bracket set");
  const children = root.children.map((index) => restored.nodes[index]);
  assert.deepEqual(children.map(({ name }) => name).sort(), ["Bracket instance", "Primary bracket"]);
  assert.ok(children.some(({ transform }) => Math.abs(transform[3] - 6) < 1e-6));
  assert.equal(children.find(({ name }) => name === "Primary bracket").material.name, "Steel");

  model.setParameter("width", 3);
  await model.recompute();
  document = model.toSTEPDocumentDefinition();
  assert.equal(document.nodes[1].transform[3], 9);
  await importScope.end();
  await model.dispose();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("failed recompute preserves the previous results atomically", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: { width: 2 },
    features: [{ id: "base", type: "box", size: ["width", 3, 4] }],
  });
  await model.recompute();
  const previous = model.getShape("base");
  const previousBox = await client.bbox(previous);

  model.setParameter("width", -1);
  await assert.rejects(model.recompute(), (error) => error.code === "InvalidArgs");
  assert.equal(model.getShape("base"), previous);
  assert.equal(model.getParameter("width"), 2);
  assert.deepEqual(await client.bbox(previous), previousBox);

  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("parametric suppression publishes diagnostics and preserves atomic results", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: {},
    features: [
      { id: "base", type: "box", size: [2, 3, 4] },
      { id: "moved", type: "translate", input: "base", translation: [1, 0, 0] },
      { id: "optional", type: "box", size: [1, 1, 1], suppressed: true },
    ],
  });
  await model.recompute();
  assert.deepEqual(model.getFeatureDiagnostics().map(({ id, status }) => [id, status]), [
    ["base", "ok"], ["moved", "ok"], ["optional", "suppressed"],
  ]);
  const previous = model.getShape("moved");

  model.setFeatureSuppressed("base");
  await assert.rejects(model.recompute(), /Feature "base" has not been built/);
  assert.equal(model.getShape("moved"), previous);
  assert.deepEqual(model.getFeatureDiagnostics().map(({ id, status }) => [id, status]), [
    ["base", "suppressed"], ["moved", "failed"], ["optional", "suppressed"],
  ]);
  assert.match(model.getFeatureDiagnostics()[1].message, /has not been built/);

  model.setFeatureSuppressed("base", false);
  await model.recompute();
  assert.equal(model.getFeatureDiagnostics().every(({ status }) => status === "ok" || status === "suppressed"), true);
  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("parametric diagnostics propagate through all unavailable dependents", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: {},
    features: [
      { id: "base", type: "box", size: [2, 3, 4], suppressed: true },
      { id: "first", type: "translate", input: "base", translation: [1, 0, 0] },
      { id: "second", type: "translate", input: "first", translation: [1, 0, 0] },
      { id: "independent", type: "box", size: [1, 1, 1] },
    ],
  });

  await assert.rejects(model.recompute(), /Feature "base" has not been built/);
  assert.deepEqual(model.getFeatureDiagnostics().map(({ id, status }) => [id, status]), [
    ["base", "suppressed"],
    ["first", "failed"],
    ["second", "failed"],
    ["independent", "ok"],
  ]);
  assert.match(model.getFeatureDiagnostics()[2].message, /Feature "first" has not been built/);
  assert.throws(() => model.getShape("independent"), /has not been computed/);

  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("JSON serialization restores expressions and boolean feature dependencies", async () => {
  const client = await DirectClient.create(wasm);
  const definition = {
    parameters: { width: 10, radius: "width / 4" },
    features: [
      { id: "result", type: "translate", input: "cut", translation: [2, 0, 0] },
      { id: "cut", type: "booleanCut", base: "plate", tools: ["hole"] },
      { id: "hole", type: "cylinder", radius: "radius", height: 4, origin: [5, 5, 0] },
      { id: "plate", type: "box", size: ["width", 10, 4] },
    ],
  };
  const source = new ParametricModel(client, definition);
  const json = JSON.stringify(source);
  const restored = ParametricModel.fromJSON(client, json);
  assert.deepEqual(restored.toJSON(), { ...definition, schemaVersion: 1 });
  assert.throws(
    () => ParametricModel.fromJSON(client, { schemaVersion: 2, parameters: {}, features: [] }),
    /Unsupported parametric schema version: 2/,
  );
  assert.throws(
    () => ParametricModel.fromJSON(client, {
      schemaVersion: 1,
      parameters: {},
      features: [{ id: "invalid", type: "box", size: [1, 1, 1], suppressed: "false" }],
    }),
    /Feature "invalid" suppressed must be boolean/,
  );

  await restored.recompute();
  assert.deepEqual((await client.bbox(restored.getShape("result"))).max.map(Math.round), [12, 10, 4]);
  assert.equal(restored.getParameter("radius"), 2.5);

  await restored.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("serializable parametric freeform curves and surfaces recompute expressions", async () => {
  const client = await DirectClient.create(wasm);
  const definition = {
    parameters: {
      heightBase: 1.5,
      height: "heightBase * 2",
      curveDegree: 3,
      surfaceDegree: 1,
      endKnot: 1,
      weight: 2,
      tolerance: 1e-7,
    },
    features: [
      {
        id: "bezierCurve",
        type: "bezierCurve",
        poles: [[0, 0, 0], [1, 0, "height / 2"], [2, 0, "height"]],
        weights: [1, "weight", 1],
      },
      {
        id: "bsplineCurve",
        type: "bsplineCurve",
        poles: [[0, 1, 0], [1, 1, 1], [2, 1, 2], [3, 1, "height"]],
        degree: "curveDegree",
        knots: [0, "endKnot"],
        multiplicities: ["curveDegree + 1", "curveDegree + 1"],
        weights: [1, "weight", 1, 1],
        tolerance: "tolerance",
      },
      {
        id: "bezierSurface",
        type: "bezierSurface",
        poles: [
          [[0, 2, 0], [0, 3, 0]],
          [[1, 2, 0], [1, 3, "height"]],
        ],
        weights: [[1, 1], [1, "weight"]],
        tolerance: "tolerance",
      },
      {
        id: "bsplineSurface",
        type: "bsplineSurface",
        poles: [
          [[0, 4, 0], [0, 5, 0]],
          [[1, 4, 0], [1, 5, "height"]],
        ],
        weights: [[1, 1], [1, "weight"]],
        uDegree: "surfaceDegree",
        vDegree: "surfaceDegree",
        uKnots: [0, "endKnot"],
        vKnots: [0, "endKnot"],
        uMultiplicities: ["surfaceDegree + 1", "surfaceDegree + 1"],
        vMultiplicities: ["surfaceDegree + 1", "surfaceDegree + 1"],
        uPeriodic: false,
        vPeriodic: false,
        tolerance: "tolerance",
      },
    ],
  };
  const model = ParametricModel.fromJSON(client, JSON.stringify(definition));
  assert.deepEqual(model.toJSON(), { ...definition, schemaVersion: 1 });

  await model.recompute();
  assert.equal(await client.shapeType(model.getShape("bezierCurve")), "edge");
  assert.equal(await client.shapeType(model.getShape("bsplineCurve")), "edge");
  assert.equal(await client.shapeType(model.getShape("bezierSurface")), "face");
  assert.equal(await client.shapeType(model.getShape("bsplineSurface")), "face");
  assert.deepEqual((await client.curveControlData(model.getShape("bezierCurve"))).weights, [1, 2, 1]);
  for (const id of ["bezierCurve", "bsplineCurve", "bezierSurface", "bsplineSurface"]) {
    assert.ok(Math.abs((await client.bbox(model.getShape(id))).max[2] - 3) < 1e-5, id);
  }

  model.setParameter("heightBase", 3);
  await model.recompute();
  assert.equal(model.getParameter("height"), 6);
  for (const id of ["bezierCurve", "bsplineCurve", "bezierSurface", "bsplineSurface"]) {
    assert.ok(Math.abs((await client.bbox(model.getShape(id))).max[2] - 6) < 1e-5, id);
  }

  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("parametric graph builds polygon profiles, advanced features, and local modeling", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: { size: 10, height: 5, radius: "size / 20" },
    features: [
      { id: "box", type: "box", size: ["size", "size", "size"] },
      { id: "fillet", type: "fillet", input: "box", edgeIndices: [0], radius: "radius" },
      {
        id: "lawFillet",
        type: "fillet",
        input: "box",
        edgeIndices: [0],
        radius: "radius",
        radiusLaw: [
          { parameter: 0, radius: "radius / 2" },
          { parameter: 0.5, radius: "radius * 1.5" },
          { parameter: 1, radius: "radius" },
        ],
      },
      { id: "chamfer", type: "chamfer", input: "box", edgeIndices: [0], distance: "radius" },
      {
        id: "twoDistanceChamfer",
        type: "chamfer",
        input: "box",
        edgeIndices: [0],
        distance: "radius",
        distances: ["radius / 2"],
        distances2: ["radius * 2"],
        referenceFaceIndices: [0],
      },
      { id: "hollow", type: "hollow", input: "box", thickness: -1, closingFaceIndices: [5] },
      { id: "offset", type: "offsetShape", input: "box", offset: "radius" },
      {
        id: "draft",
        type: "draftAngle",
        input: "box",
        faceIndices: [0, 1, 2, 3],
        direction: [0, 0, 1],
        angle: 0.1,
        neutralPlane: { origin: [0, 0, 0], normal: [0, 0, 1] },
      },
      { id: "boss", type: "localPrism", base: "box", faceIndices: [5], direction: [0, 0, 1], length: "height" },
      { id: "revolved", type: "localRevolution", base: "box", faceIndices: [5], origin: [0, 0, "size"], direction: [0, 1, 0], angle: "height / size" },
      { id: "revBase", type: "cylinder", radius: 50, height: 100 },
      { id: "revProfile", type: "polygon", points: [[50, 0, 40], [60, 0, 50], [50, 0, 60]], close: false },
      {
        id: "revolutionRib",
        type: "revolutionForm",
        base: "revBase",
        profile: "revProfile",
        planeOrigin: [0, 0, 0],
        planeNormal: [0, 1, 0],
        origin: [0, 0, 0],
        direction: [0, 0, 1],
        height1: "height",
        height2: "height",
        modify: false,
      },
      { id: "glueBase", type: "box", size: [10, 10, 10] },
      { id: "glueNew", type: "box", size: [4, 4, 4], origin: [3, 3, 10] },
      {
        id: "glued",
        type: "glue",
        newShape: "glueNew",
        baseShape: "glueBase",
        faceBindings: [{ newIndex: 4, baseIndex: 5 }],
      },
      { id: "hole", type: "cylindricalHole", input: "box", origin: [5, 5, -1], direction: [0, 0, 1], radius: "radius" },
      { id: "lower", type: "polygon", points: [[0, 0, 0], [4, 0, 0], [4, 4, 0], [0, 4, 0]] },
      { id: "upper", type: "polygon", points: [[1, 1, "height"], [3, 1, "height"], [3, 3, "height"], [1, 3, "height"]] },
      { id: "loft", type: "loft", sections: ["lower", "upper"], solid: true },
      { id: "spine", type: "polygon", points: [[0, 0, 0], [0, 0, "height"]], close: false },
      { id: "profile", type: "polygon", points: [[1, 0, 0], [0, 1, 0], [-1, 0, 0], [0, -1, 0]] },
      { id: "sweep", type: "sweepPipe", spine: "spine", profile: "profile" },
      { id: "wireOffset", type: "offsetWire2D", input: "lower", offset: "radius" },
    ],
  });

  await model.recompute();
  for (const id of ["fillet", "lawFillet", "chamfer", "twoDistanceChamfer", "hollow", "offset", "draft", "boss", "revolved", "revolutionRib", "glued", "hole", "loft", "sweep", "wireOffset"]) {
    assert.equal(await client.isValid(model.getShape(id)), true, id);
  }
  assert.equal(await client.shapeType(model.getShape("loft")), "solid");
  assert.equal(await client.shapeType(model.getShape("sweep")), "shell");
  assert.ok((await client.massProps(model.getShape("boss"))).mass > 1400);

  model.setParameter("height", 8);
  await model.recompute();
  assert.ok((await client.massProps(model.getShape("boss"))).mass > 1700);
  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("parametric graph includes topology repair, section, split, and sewing features", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: {},
    features: [
      { id: "base", type: "box", size: [10, 10, 10] },
      { id: "tool", type: "box", size: [4, 4, 12], origin: [3, 3, -1] },
      { id: "fixed", type: "fixShape", input: "base" },
      { id: "unified", type: "unifySameDomain", input: "base" },
      { id: "section", type: "section", first: "base", second: "tool" },
      { id: "split", type: "split", objects: ["base"], tools: ["tool"] },
      { id: "wireA", type: "polygon", points: [[0, 0, 0], [2, 0, 0], [2, 2, 0], [0, 2, 0]] },
      { id: "wireB", type: "polygon", points: [[2, 0, 0], [4, 0, 0], [4, 2, 0], [2, 2, 0]] },
      { id: "faceA", type: "face", outer: "wireA" },
      { id: "faceB", type: "face", outer: "wireB" },
      { id: "upgradeWire", type: "polygon", points: [[0, 0, 0], [10, 0, 0], [10, 10, 0], [0, 10, 0]] },
      { id: "upgradeFace", type: "face", outer: "upgradeWire" },
      { id: "upgraded", type: "shapeUpgrade", input: "upgradeFace", mode: "area", areaMode: "maxArea", maxArea: 30 },
      { id: "sewn", type: "sew", shapes: ["faceA", "faceB"] },
    ],
  });
  await model.recompute();
  for (const id of ["fixed", "unified", "section", "split", "sewn"]) {
    assert.equal(await client.isValid(model.getShape(id)), true, id);
  }
  assert.ok((await client.topologyCounts(model.getShape("sewn"))).face >= 2);
  assert.ok((await client.topologyCounts(model.getShape("upgraded"))).face > 1);
  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("concurrent recompute is ordered and dispose cannot be overtaken", async () => {
  let nextScope = 0;
  const client = {
    async beginScope() {
      const id = ++nextScope;
      return {
        async makeBox(size) {
          if (id === 1) await new Promise((resolve) => setTimeout(resolve, 20));
          return { id, size };
        },
        async end() {},
      };
    },
  };
  const model = new ParametricModel(client, {
    parameters: { width: 1 },
    features: [{ id: "box", type: "box", size: ["width", 1, 1] }],
  });
  const first = model.recompute();
  model.setParameter("width", 2);
  const second = model.recompute();
  await Promise.all([first, second]);
  assert.deepEqual(model.getShape("box").size, [2, 1, 1]);

  model.setParameter("width", 3);
  const recompute = model.recompute();
  const dispose = model.dispose();
  await Promise.all([recompute, dispose]);
  assert.throws(() => model.getShape("box"), /has not been computed/);
  await assert.rejects(model.recompute(), /has been disposed/);
});
