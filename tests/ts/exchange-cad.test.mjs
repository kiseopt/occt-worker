import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("IGES export and memory-stream import preserve shape dimensions and units", async () => {
  const client = await DirectClient.create(wasm);
  const capabilities = await client.initialize();
  assert.ok(capabilities.ops.includes("exportIGES"));
  assert.ok(capabilities.ops.includes("importIGES"));

  const scope = await client.beginScope();
  const box = await scope.makeBox([10, 6, 4]);
  const iges = await client.exportIGES(box, { unit: "mm", mode: "brep" });
  assert.ok(iges.byteLength > 0);
  assert.equal((await client.probeFormat(iges)).format, "iges");

  const restored = await scope.importIGES(iges, { unit: "mm" });
  assert.ok(restored.rootCount > 0);
  const restoredBox = await client.bbox(restored.shape);
  assert.ok(Math.abs(restoredBox.max[0] - 10) < 1e-4);
  assert.ok(Math.abs(restoredBox.max[1] - 6) < 1e-4);
  assert.ok(Math.abs(restoredBox.max[2] - 4) < 1e-4);

  const inMeters = await scope.importIGES(iges, { unit: "m" });
  const meterBox = await client.bbox(inMeters.shape);
  assert.ok(Math.abs(meterBox.max[0] - 0.01) < 1e-6);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("IGES multi-root export and import preserve independent scoped root shapes", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const first = await scope.makeBox([2, 3, 4]);
  const second = await scope.makeBox([5, 6, 7], [20, 0, 0]);
  const data = await client.exportIGES([first, second], { unit: "mm", mode: "brep" });
  const imported = await scope.importIGES(data, { unit: "mm" });
  assert.equal(imported.rootCount, 2);
  assert.equal(imported.shapes.length, 2);
  const bounds = await Promise.all(imported.shapes.map((shape) => client.bbox(shape)));
  bounds.sort((left, right) => left.min[0] - right.min[0]);
  assert.ok(Math.abs(bounds[0].max[0] - 2) < 1e-4);
  assert.ok(Math.abs(bounds[1].min[0] - 20) < 1e-4);
  assert.ok(Math.abs(bounds[1].max[0] - 25) < 1e-4);
  assert.equal(await client.isValid(imported.shape), true);

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("IGES XCAF document memory streams preserve names, colors, layers, and roots", async () => {
  const client = await DirectClient.create(wasm);
  const capabilities = await client.initialize();
  assert.ok(capabilities.ops.includes("exportIGESDocument"));
  assert.ok(capabilities.ops.includes("importIGESDocument"));
  const scope = await client.beginScope();
  const box = await scope.makeBox([4, 5, 6]);
  const document = {
    nodes: [{
      kind: "part",
      shape: box,
      name: "IGES part",
      color: [0.2, 0.4, 0.8, 0.75],
      layers: ["Machining", "Inspection"],
      transform: [1, 0, 0, 3, 0, 1, 0, 0, 0, 0, 1, 0],
    }, {
      kind: "assembly",
      name: "IGES root",
      children: [0],
    }],
    roots: [1],
  };
  const data = await client.exportIGESDocument(document, { unit: "mm" });
  assert.ok(data.byteLength > 0);
  assert.equal((await client.probeFormat(data)).format, "iges");
  const restored = await scope.importIGESDocument(data, { unit: "mm" });
  assert.equal(restored.rootCount, 1);
  assert.deepEqual(restored.roots, [1]);
  assert.equal(restored.nodes.length, 2);
  assert.equal(restored.nodes[0].name, "IGES part");
  assert.equal(restored.nodes[1].name, "IGES root");
  assert.deepEqual(restored.nodes[1].children, [0]);
  assert.ok(restored.nodes[0].layers.includes("Machining"));
  assert.ok(restored.nodes[0].layers.includes("Inspection"));
  assert.ok(restored.nodes[0].color === undefined || restored.nodes[0].color.length === 4);
  const bounds = await client.bbox(restored.nodes[0].shape);
  assert.ok(Math.abs(bounds.min[0] - 3) < 1e-4);
  assert.ok(Math.abs(bounds.max[0] - 7) < 1e-4);
  await assert.rejects(
    client.exportIGESDocument({ ...document, datums: [{ node: 0, name: "A", identification: "A" }] }),
    (error) => error.code === "ImportExportFailed" && /cannot preserve XCAF/.test(error.message),
  );
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("IGES transfer reports OCCT progress and cooperatively observes cancellation", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const shape = await scope.makeBox([10, 20, 30]);
  const exportProgress = [];
  const exported = await client.request("exportIGES", { shape, unit: "mm", mode: "brep" }, {
    onProgress: (event) => exportProgress.push(event.fraction),
  });
  assert.equal(exportProgress[0], 0);
  assert.equal(exportProgress.at(-1), 1);
  assert.ok(exportProgress.some((fraction) => fraction > 0 && fraction < 1));

  const controller = new AbortController();
  const importProgress = [];
  await assert.rejects(
    client.request("importIGES", {
      scopeId: scope.scopeId,
      unit: "mm",
      data: { $inputBuffer: exported.data.data.slice(0) },
    }, {
      signal: controller.signal,
      onProgress: (event) => {
        importProgress.push(event.fraction);
        if (event.fraction > 0 && event.fraction < 1) {
          controller.abort(new Error("IGES import cancelled"));
        }
      },
    }),
    /IGES import cancelled/,
  );
  assert.ok(importProgress.some((fraction) => fraction > 0 && fraction < 1));
  assert.equal((await client.stats()).liveShapeHandles, 1);

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("STEP XCAF document round-trip preserves assembly metadata and instance transforms", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const identity = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0];
  const translated = [1, 0, 0, 5, 0, 1, 0, 0, 0, 0, 1, 0];
  const step = await client.exportSTEPDocument({
    nodes: [
      {
        kind: "part",
        shape: box,
        name: "Red box",
        transform: identity,
        visualMaterial: { name: "Red coating", baseColor: [1, 0, 0, 1] },
        layers: ["Mechanical"],
        material: {
          name: "Steel",
          description: "Structural steel",
          density: 7850,
          densityName: "kg/m^3",
          densityValueType: "mass density",
        },
      },
      {
        kind: "part",
        shape: box,
        name: "Blue box",
        transform: translated,
        color: [0, 0, 1, 1],
        visible: false,
      },
      {
        kind: "assembly",
        name: "Top assembly",
        children: [0, 1],
      },
    ],
    roots: [2],
  });
  assert.ok(step.byteLength > 0);
  assert.deepEqual(await client.probeFormat(step), {
    format: "step", encoding: "text", confidence: "exact", documentMetadata: true,
  });

  const restored = await scope.importSTEPDocument(step);
  assert.equal(restored.rootCount, 1);
  assert.equal(restored.roots.length, 1);
  const root = restored.nodes[restored.roots[0]];
  assert.equal(root.kind, "assembly");
  assert.equal(root.name, "Top assembly");
  assert.equal(root.children.length, 2);
  const children = root.children.map((index) => restored.nodes[index]);
  assert.deepEqual(children.map((node) => node.name).sort(), ["Blue box", "Red box"]);
  assert.ok(children.some((node) => Math.abs(node.transform[3] - 5) < 1e-6));
  const red = children.find((node) => node.name === "Red box");
  const blue = children.find((node) => node.name === "Blue box");
  assert.ok(red.color[0] > 0.9 && red.color[1] < 0.1 && red.color[2] < 0.1);
  assert.ok(red.layers.includes("Mechanical"));
  assert.equal(red.material.name, "Steel");
  assert.equal(red.material.description, "Structural steel");
  assert.equal(red.material.density, 7850);
  assert.equal(red.material.densityName, "kg/m^3");
  assert.equal(blue.visible, false);

  const bounds = await client.bbox(restored.shape);
  assert.ok(Math.abs(bounds.min[0]) < 1e-6);
  assert.ok(Math.abs(bounds.max[0] - 7) < 1e-5);
  assert.ok(Math.abs(bounds.max[1] - 3) < 1e-5);
  assert.ok(Math.abs(bounds.max[2] - 4) < 1e-5);
  await assert.rejects(
    client.exportSTEPDocument({
      nodes: [
        { kind: "assembly", children: [1] },
        { kind: "assembly", children: [0] },
        { kind: "part", shape: box },
      ],
      roots: [2],
    }),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("STEP XCAF document round-trip preserves face presentation styles", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const step = await client.exportSTEPDocument({
    nodes: [{
      kind: "part",
      shape: box,
      name: "Styled box",
      subshapeStyles: [{ topology: "face", index: 0, color: [0, 1, 0, 0.8] }],
    }],
    roots: [0],
  });
  const restored = await scope.importSTEPDocument(step);
  const part = restored.nodes[restored.roots[0]];
  assert.equal(part.name, "Styled box");
  assert.equal(part.subshapeStyles.length, 1);
  assert.equal(part.subshapeStyles[0].topology, "face");
  assert.ok(part.subshapeStyles[0].color[0] < 0.1);
  assert.ok(part.subshapeStyles[0].color[1] > 0.9);
  assert.ok(part.subshapeStyles[0].color[2] < 0.1);
  assert.ok(Math.abs(part.subshapeStyles[0].color[3] - 0.8) < 1e-5);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("STEP XCAF document round-trip preserves size dimensions", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const cylinder = await scope.makeCylinder(5, 20);
  const document = {
    nodes: [{ kind: "part", shape: cylinder, name: "Datum pin" }],
    roots: [0],
    gdt: [
      { node: 0, type: "diameter", value: 10, semanticName: "PIN_DIAMETER" },
      { node: 0, type: "radius", value: 5, semanticName: "PIN_RADIUS" },
      { node: 0, type: "angular", value: 0.5, semanticName: "PIN_ANGLE" },
    ],
  };
  const step = await client.exportSTEPDocument(document);
  const restored = await scope.importSTEPDocument(step);
  assert.equal(restored.gdt.length, 3);
  assert.deepEqual(restored.gdt.map(({ node, type, value, semanticName }) => ({ node, type, value, semanticName })), [
    { node: restored.roots[0], type: "diameter", value: 10, semanticName: "PIN_DIAMETER" },
    { node: restored.roots[0], type: "radius", value: 5, semanticName: "PIN_RADIUS" },
    { node: restored.roots[0], type: "angular", value: 0.5, semanticName: "PIN_ANGLE" },
  ]);
  const xcaf = await client.exportXCAF(document, "bin");
  const restoredXcaf = await scope.importXCAF(xcaf, "bin");
  assert.deepEqual(restoredXcaf.gdt, restored.gdt);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("native XCAF datum metadata and shape link round-trip", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const document = {
    nodes: [{ kind: "part", shape: box, name: "Datum part" }],
    roots: [0],
    datums: [{
      node: 0,
      name: "A",
      description: "Primary datum plane",
      identification: "DATUM-A",
      semanticName: "PRIMARY_A",
    }],
  };
  const native = await client.exportXCAF(document, "bin");
  const restoredNative = await scope.importXCAF(native, "bin");
  assert.deepEqual(restoredNative.datums, document.datums);
  await assert.rejects(() => client.exportSTEPDocument(document), /cannot preserve XCAF datums/);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("native XCAF geometric tolerances preserve values and datum links", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const document = {
    nodes: [{ kind: "part", shape: box, name: "Tolerance part" }],
    roots: [0],
    datums: [
      { node: 0, name: "A", identification: "DATUM-A", semanticName: "DATUM_A" },
      { node: 0, name: "B", identification: "DATUM-B", semanticName: "DATUM_B" },
    ],
    geometricTolerances: [{
      node: 0,
      type: "flatness",
      value: 0.25,
      valueType: "diameter",
      semanticName: "FACE_FLATNESS",
      datumIndices: [0, 1],
      materialRequirement: "maximum",
      zoneModifier: "projected",
      zoneModifierValue: 1.5,
      modifiers: ["commonZone", "allAround", "tangentPlane"],
      maxValueModifier: 0.5,
    }],
  };
  const native = await client.exportXCAF(document, "bin");
  const restoredNative = await scope.importXCAF(native, "bin");
  assert.deepEqual(restoredNative.datums, document.datums);
  assert.deepEqual(restoredNative.geometricTolerances, document.geometricTolerances);
  const xml = await client.exportXCAF({
    ...document,
    datums: [],
    geometricTolerances: [{ ...document.geometricTolerances[0], datumIndices: undefined }],
  }, "xml");
  const restoredXml = await scope.importXCAF(xml, "xml");
  assert.deepEqual(restoredXml.geometricTolerances, [{
    node: 0, type: "flatness", value: 0.25, valueType: "diameter", semanticName: "FACE_FLATNESS",
    materialRequirement: "maximum", zoneModifier: "projected", zoneModifierValue: 1.5,
    modifiers: ["commonZone", "allAround", "tangentPlane"], maxValueModifier: 0.5,
  }]);
  const xmlWithMetadata = await client.exportXCAF(document, "xml");
  const restoredXmlWithMetadata = await scope.importXCAF(xmlWithMetadata, "xml");
  assert.deepEqual(restoredXmlWithMetadata.datums, document.datums);
  assert.deepEqual(restoredXmlWithMetadata.geometricTolerances, document.geometricTolerances);
  await assert.rejects(() => client.exportSTEPDocument(document), /cannot preserve XCAF datums/);
  await assert.rejects(() => client.exportSTEPDocument({ ...document, datums: [] }), /cannot preserve XCAF geometric tolerance links/);
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("native XCAF binary and XML preserve annotation presentation shapes and names", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const dimensionPresentation = await scope.makeEdgeLine([0, 0, 0], [1, 0, 0]);
  const datumPresentation = await scope.makeEdgeLine([0, 0, 0], [0, 2, 0]);
  const tolerancePresentation = await scope.makeEdgeLine([0, 0, 0], [0, 0, 3]);
  const document = {
    nodes: [{ kind: "part", shape: box, name: "Annotated part" }],
    roots: [0],
    gdt: [{
      node: 0,
      type: "diameter",
      value: 2,
      semanticName: "DIAMETER",
      presentation: { shape: dimensionPresentation, name: "dimension-curve" },
    }],
    datums: [{
      node: 0,
      name: "A",
      identification: "DATUM-A",
      presentation: { shape: datumPresentation, name: "datum-symbol" },
    }],
    geometricTolerances: [{
      node: 0,
      type: "flatness",
      value: 0.1,
      presentation: { shape: tolerancePresentation, name: "tolerance-frame" },
    }],
  };

  for (const format of ["bin", "xml"]) {
    const data = await client.exportXCAF(document, format);
    const restored = await scope.importXCAF(data, format);
    assert.equal(restored.gdt[0].presentation.name, "dimension-curve");
    assert.equal(restored.datums[0].presentation.name, "datum-symbol");
    assert.equal(restored.geometricTolerances[0].presentation.name, "tolerance-frame");
    const dimensionBounds = await client.bbox(restored.gdt[0].presentation.shape);
    const datumBounds = await client.bbox(restored.datums[0].presentation.shape);
    const toleranceBounds = await client.bbox(restored.geometricTolerances[0].presentation.shape);
    assert.ok(Math.abs(dimensionBounds.max[0] - 1) < 1e-5);
    assert.ok(Math.abs(datumBounds.max[1] - 2) < 1e-5);
    assert.ok(Math.abs(toleranceBounds.max[2] - 3) < 1e-5);
  }

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("native XCAF binary and XML preserve every advanced geometric tolerance modifier", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const tolerance = {
    node: 0,
    type: "position",
    value: 0.2,
    semanticName: "ADVANCED_TOLERANCE",
    materialRequirement: "least",
    zoneModifier: "nonUniform",
    zoneModifierValue: 2.25,
    modifiers: [
      "anyCrossSection", "commonZone", "eachRadialElement", "freeState",
      "leastMaterialRequirement", "lineElement", "majorDiameter",
      "maximumMaterialRequirement", "minorDiameter", "notConvex", "pitchDiameter",
      "reciprocityRequirement", "separateRequirement", "statisticalTolerance",
      "tangentPlane", "allAround", "allOver",
    ],
    maxValueModifier: 0.75,
  };
  const document = {
    nodes: [{ kind: "part", shape: box, name: "Advanced tolerance part" }],
    roots: [0],
    geometricTolerances: [tolerance],
  };

  for (const format of ["bin", "xml"]) {
    const data = await client.exportXCAF(document, format);
    const restored = await scope.importXCAF(data, format);
    assert.deepEqual(restored.geometricTolerances, [tolerance]);
  }

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("native XCAF persistence round-trips bounded views and SHUO paths", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const document = {
    nodes: [
      { kind: "part", shape: box, name: "Viewed part" },
      { kind: "assembly", name: "Subassembly", children: [0] },
      { kind: "assembly", name: "Root", children: [1] },
    ],
    roots: [2],
    views: [{
      name: "Front",
      projection: "parallel",
      projectionPoint: [0, 0, 10],
      viewDirection: [0, 0, -1],
      upDirection: [0, 1, 0],
      zoomFactor: 2,
      windowHorizontalSize: 20,
      windowVerticalSize: 15,
      clippingPlanes: [{
        name: "Section A",
        origin: [0, 0, 1],
        normal: [0, 0, 1],
        capping: true,
      }],
      nodeIndices: [0],
    }],
    shuo: [{ nodeIndices: [1, 0], color: [0.2, 0.4, 0.6, 0.8] }],
  };
  for (const format of ["bin", "xml"]) {
    const data = await client.exportXCAF(document, format);
    const restored = await scope.importXCAF(data, format);
    assert.equal(restored.views.length, 1);
    assert.equal(restored.views[0].name, "Front");
    assert.equal(restored.views[0].projection, "parallel");
    assert.deepEqual(restored.views[0].nodeIndices, [2]);
    assert.ok(Math.abs(restored.views[0].zoomFactor - 2) < 1e-5);
    assert.deepEqual(restored.views[0].clippingPlanes, [{
      name: "Section A",
      origin: [0, 0, 1],
      normal: [0, 0, 1],
      capping: true,
    }]);
    assert.deepEqual(restored.shuo[0].nodeIndices, [1, 2]);
    assert.ok(restored.shuo[0].color.every((value, index) => Math.abs(value - [0.2, 0.4, 0.6, 0.8][index]) < 1e-6));
  }
  await assert.rejects(
    client.exportXCAF({
      nodes: [{ kind: "part", shape: box }],
      roots: [0],
      views: [{
        name: "Invalid",
        viewDirection: [0, 0, -1],
        upDirection: [0, 1, 0],
        nodeIndices: [0],
        clippingPlanes: [{ name: "Invalid normal", origin: [0, 0, 0], normal: [0, 0, 0] }],
      }],
    }, "bin"),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("native XCAF binary and XML persistence round-trip assembly metadata", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const translated = [1, 0, 0, 5, 0, 1, 0, 0, 0, 0, 1, 0];
  const document = {
    nodes: [
      {
        kind: "part",
        shape: box,
        name: "Stored box",
        transform: translated,
        color: [0.2, 0.4, 0.8, 1],
        layers: ["Persistence"],
        visible: false,
        material: { name: "Aluminum", density: 2700 },
        subshapeStyles: [
          { topology: "face", index: 0, color: [0.9, 0.1, 0.2, 0.8] },
          { topology: "edge", index: 0, color: [0.1, 0.9, 0.2, 0.7] },
        ],
        validationProperties: { area: 52, volume: 24, centroid: [1, 1.5, 2] },
        visualMaterial: {
          name: "Blue anodized",
          baseColor: [0.15, 0.3, 0.75, 0.9],
          metallic: 0.85,
          roughness: 0.25,
          emissive: [0.01, 0.02, 0.03],
          refractionIndex: 1.6,
        },
      },
      { kind: "assembly", name: "Stored assembly", children: [0] },
    ],
    roots: [1],
  };

  for (const format of ["bin", "xml"]) {
    const data = await client.exportXCAF(document, format);
    assert.ok(data.byteLength > 0);
    assert.deepEqual(await client.probeFormat(data), {
      format: "xcaf",
      encoding: format === "bin" ? "binary" : "text",
      confidence: "exact",
    });
    const restored = await scope.importXCAF(data, format);
    assert.equal(restored.rootCount, 1);
    const root = restored.nodes[restored.roots[0]];
    assert.equal(root.kind, "assembly");
    assert.equal(root.name, "Stored assembly");
    assert.equal(root.children.length, 1);
    const child = restored.nodes[root.children[0]];
    assert.equal(child.name, "Stored box");
    assert.ok(Math.abs(child.transform[3] - 5) < 1e-6);
    assert.ok(Math.abs(child.color[0] - 0.2) < 1e-5);
    assert.ok(Math.abs(child.color[1] - 0.4) < 1e-5);
    assert.ok(Math.abs(child.color[2] - 0.8) < 1e-5);
    assert.ok(child.layers.includes("Persistence"));
    assert.equal(child.material.name, "Aluminum");
    assert.equal(child.material.density, 2700);
    assert.equal(child.material.densityName, "kg/m^3");
    assert.equal(child.material.densityValueType, "mass density");
    assert.ok(Math.abs(child.validationProperties.area - 52) < 1e-8);
    assert.ok(Math.abs(child.validationProperties.volume - 24) < 1e-8);
    assert.deepEqual(child.validationProperties.centroid, [1, 1.5, 2]);
    assert.equal(child.visualMaterial.name, "Blue anodized");
    assert.ok(Math.abs(child.visualMaterial.baseColor[0] - 0.15) < 1e-5);
    assert.ok(Math.abs(child.visualMaterial.baseColor[1] - 0.3) < 1e-5);
    assert.ok(Math.abs(child.visualMaterial.baseColor[2] - 0.75) < 1e-5);
    assert.ok(Math.abs(child.visualMaterial.baseColor[3] - 0.9) < 1e-5);
    assert.ok(Math.abs(child.visualMaterial.metallic - 0.85) < 1e-5);
    assert.ok(Math.abs(child.visualMaterial.roughness - 0.25) < 1e-5);
    assert.deepEqual(child.visualMaterial.emissive.map((value) => Math.round(value * 100)), [1, 2, 3]);
    assert.ok(Math.abs(child.visualMaterial.refractionIndex - 1.6) < 1e-5);
    assert.deepEqual(child.subshapeStyles.map((style) => [style.topology, style.index]), [
      ["face", 0], ["edge", 0],
    ]);
    assert.ok(Math.abs(child.subshapeStyles[0].color[0] - 0.9) < 1e-5);
    assert.ok(Math.abs(child.subshapeStyles[0].color[3] - 0.8) < 1e-5);
    assert.ok(Math.abs(child.subshapeStyles[1].color[1] - 0.9) < 1e-5);
    assert.ok(Math.abs(child.subshapeStyles[1].color[3] - 0.7) < 1e-5);
    assert.equal(child.visible, false);
    const bounds = await client.bbox(restored.shape);
    assert.ok(Math.abs(bounds.min[0] - 5) < 1e-6);
    assert.ok(Math.abs(bounds.max[0] - 7) < 1e-6);
  }

  await assert.rejects(
    client.exportXCAF({
      nodes: [{ kind: "part", shape: box, visualMaterial: { name: "Invalid", metallic: 1.1 } }],
      roots: [0],
    }, "bin"),
    (error) => error.code === "InvalidArgs",
  );
  await assert.rejects(
    client.exportXCAF({
      nodes: [{ kind: "part", shape: box, validationProperties: { area: -1 } }],
      roots: [0],
    }, "bin"),
    (error) => error.code === "InvalidArgs",
  );
  await assert.rejects(
    client.exportXCAF({
      nodes: [{ kind: "part", shape: box, subshapeStyles: [
        { topology: "face", index: 99, color: [1, 0, 0, 1] },
      ] }],
      roots: [0],
    }, "bin"),
    (error) => error.code === "InvalidArgs",
  );

  await assert.rejects(
    scope.importXCAF(new TextEncoder().encode("not an XCAF document").buffer, "bin"),
    (error) => error.code === "ImportExportFailed"
      && error.details?.operation === "importXCAF"
      && error.details?.format === "xcaf",
  );
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("importIGES rejects non-IGES payloads", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  await assert.rejects(
    scope.importIGES(new TextEncoder().encode("not an iges file").buffer),
    (error) => error.code === "ImportExportFailed"
      && error.details?.operation === "importIGES"
      && error.details?.format === "iges",
  );
  await scope.end();
});

