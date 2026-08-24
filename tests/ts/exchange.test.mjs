import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("binary STL export and memory-stream import round-trip a tessellated shape", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const stl = await client.exportSTL(box, { linearDeflection: 0.05 });
  assert.ok(stl.byteLength > 84);
  assert.deepEqual(await client.probeFormat(stl), {
    format: "stl", encoding: "binary", confidence: "exact",
  });
  const imported = await scope.importSTL(stl);
  const counts = await client.topologyCounts(imported);
  assert.ok(counts.face > 0);
  assert.equal(await client.isValid(imported), true);
  const importedMesh = await scope.importSTL(stl, { includeMesh: true });
  assert.ok(importedMesh.positions.length > 0);
  assert.equal(importedMesh.positions.length, importedMesh.normals.length);
  assert.equal(importedMesh.uvs, undefined);
  assert.ok(importedMesh.indices.length > 0);
  assert.equal(importedMesh.indices.length % 3, 0);
  const meshSTL = await client.exportSTL(importedMesh);
  assert.deepEqual(await client.probeFormat(meshSTL), {
    format: "stl", encoding: "binary", confidence: "exact",
  });
  const restoredMesh = await scope.importSTL(meshSTL, { includeMesh: true });
  assert.equal(restoredMesh.indices.length, importedMesh.indices.length);
  const customHeader = Uint8Array.from({ length: 80 }, (_, index) => index);
  const headedSTL = await client.exportSTL(box, { binaryHeader: customHeader });
  const headedImport = await scope.importSTL(headedSTL, { includeMesh: true });
  assert.deepEqual(headedImport.binaryHeader, customHeader);
  const asciiSTL = await client.exportSTL(box, {
    encoding: "ascii", linearDeflection: 0.05, solidName: "named-part",
  });
  assert.ok(asciiSTL.byteLength > 0);
  const asciiText = new TextDecoder().decode(asciiSTL);
  assert.match(asciiText, /^solid named-part\n/);
  assert.match(asciiText, /endsolid named-part\n$/);
  assert.deepEqual(await client.probeFormat(asciiSTL), {
    format: "stl", encoding: "text", confidence: "likely",
  });
  const asciiImported = await scope.importSTL(asciiSTL);
  assert.equal(await client.isValid(asciiImported), true);
  const asciiImportedMesh = await scope.importSTL(asciiSTL, { includeMesh: true });
  assert.equal(asciiImportedMesh.solidName, "named-part");
  assert.equal(asciiImportedMesh.binaryHeader, undefined);
  const asciiMeshSTL = await client.exportSTL(importedMesh, { encoding: "ascii" });
  assert.deepEqual(await client.probeFormat(asciiMeshSTL), {
    format: "stl", encoding: "text", confidence: "likely",
  });
  const smallSphere = await scope.makeSphere(1);
  const largeSphere = await scope.makeSphere(100);
  const smallRelative = await client.exportSTL(smallSphere, {
    linearDeflection: 0.05, angularDeflection: 0.5, relative: true,
  });
  const largeRelative = await client.exportSTL(largeSphere, {
    linearDeflection: 0.05, angularDeflection: 0.5, relative: true,
  });
  const largeAbsolute = await client.exportSTL(largeSphere, {
    linearDeflection: 0.05, angularDeflection: 0.5, relative: false,
  });
  assert.equal(smallRelative.byteLength, largeRelative.byteLength);
  assert.ok(largeAbsolute.byteLength > largeRelative.byteLength);
  await assert.rejects(
    client.exportSTL({
      positions: new Float32Array([0, 0, 0, 1, 0, 0, 0, 1, 0]),
      indices: new Uint32Array([0, 1, 3]),
    }),
    (error) => error.code === "InvalidArgs" && /out of range/.test(error.message),
  );
  await assert.rejects(
    client.exportSTL({
      positions: new Float32Array([0, 0, 0, 1, 0, 0, 2, 0, 0]),
      indices: new Uint32Array([0, 1, 2]),
    }),
    (error) => error.code === "InvalidArgs" && /degenerate/.test(error.message),
  );
  await assert.rejects(
    client.exportSTL(box, { binaryHeader: new Uint8Array(79) }),
    (error) => error.code === "InvalidArgs" && /exactly 80 bytes/.test(error.message),
  );
  await assert.rejects(
    client.exportPLY({
      positions: new Float32Array([0, 0, 0, 1, 0, 0, 2, 0, 0]),
      indices: new Uint32Array([0, 1, 2]),
    }),
    (error) => error.code === "InvalidArgs" && /degenerate/.test(error.message),
  );
  const brep = await client.exportBREP(box);
  assert.deepEqual(await client.probeFormat(brep), {
    format: "brep", encoding: "text", confidence: "exact",
  });
  assert.deepEqual(await client.probeFormat(new TextEncoder().encode("not cad data").buffer), {
    format: "unknown", confidence: "none",
  });
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("mesh exchange formats report progress and support cooperative cancellation", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const shape = await scope.makeBox([2, 3, 4]);
  const formats = [
    ["exportOBJ", "importOBJ", {}],
    ["exportPLY", "importPLY", { encoding: "ascii" }],
    ["exportGLTF", "importGLTF", { format: "glb" }],
  ];
  for (const [exportOperation, importOperation, options] of formats) {
    const exportProgress = [];
    const exported = await client.request(exportOperation, { shape, ...options }, {
      onProgress: (event) => exportProgress.push(event.fraction),
    });
    assert.equal(exportProgress[0], 0);
    assert.equal(exportProgress.at(-1), 1);
    assert.ok(exportProgress.some((fraction) => fraction > 0 && fraction < 1));
    const importProgress = [];
    await client.request(importOperation, {
      scopeId: scope.scopeId,
      data: { $inputBuffer: exported.data.data.slice(0) },
    }, { onProgress: (event) => importProgress.push(event.fraction) });
    assert.equal(importProgress[0], 0);
    assert.equal(importProgress.at(-1), 1);
    assert.ok(importProgress.some((fraction) => fraction > 0 && fraction < 1));
  }
  const controller = new AbortController();
  const cancellationProgress = [];
  await assert.rejects(
    client.request("exportOBJ", { shape }, {
      signal: controller.signal,
      onProgress: (event) => {
        cancellationProgress.push(event.fraction);
        if (event.fraction > 0 && event.fraction < 1)
          controller.abort(new Error("OBJ export cancelled"));
      },
    }),
    /OBJ export cancelled/,
  );
  assert.ok(cancellationProgress.some((fraction) => fraction > 0 && fraction < 1));
  await scope.end();
});

test("native cancellation rolls back late mesh exchange resources", async () => {
  let cancelled = false;
  let cancelAt;
  const client = await DirectClient.create(wasm, {}, {
    isCancelled: () => cancelled,
    onProgress: (fraction) => {
      if (cancelAt !== undefined && fraction >= cancelAt) cancelled = true;
    },
  });
  const scope = await client.beginScope();
  const shape = await scope.makeBox([2, 3, 4]);

  cancelled = true;
  await assert.rejects(
    client.request("exportPLY", { shape, encoding: "ascii" }),
    (error) => error.code === "Cancelled",
  );

  cancelled = false;
  cancelAt = undefined;
  const exported = await client.request("exportPLY", { shape, encoding: "ascii" });

  cancelAt = 1;
  await assert.rejects(
    client.request("exportPLY", { shape, encoding: "ascii" }),
    (error) => error.code === "Cancelled",
  );
  assert.equal((await client.stats()).liveBufferBytes, 0);

  cancelled = false;
  cancelAt = 2 / 3;
  await assert.rejects(
    client.request("importPLY", {
      scopeId: scope.scopeId,
      data: { $inputBuffer: exported.data.data.slice(0) },
      includeMesh: true,
    }),
    (error) => error.code === "Cancelled",
  );
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 1);
  assert.equal(stats.liveBufferBytes, 0);
  assert.deepEqual((await client.bbox(shape)).max.map(Math.round), [2, 3, 4]);

  await scope.end();
});

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

test("tessellation reports OCCT progress and cooperatively observes cancellation", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const boxes = [];
  for (let index = 0; index < 24; index++) {
    boxes.push(await scope.makeBox([1, 1, 1], [index * 2, 0, 0]));
  }
  const shape = await scope.makeCompound(boxes);
  const progress = [];
  const tessellated = await client.request("tessellate", { shape, linearDeflection: 0.05 }, {
    onProgress: (event) => progress.push(event.fraction),
  });
  assert.ok(tessellated.positions.data.byteLength > 0);
  assert.equal(progress[0], 0);
  assert.equal(progress.at(-1), 1);
  assert.ok(progress.some((fraction) => fraction > 0 && fraction < 1));

  const edgeProgress = [];
  const edges = await client.request("tessellateEdges", { shape }, {
    onProgress: (event) => edgeProgress.push(event.fraction),
  });
  assert.ok(edges.positions.data.byteLength > 0);
  assert.equal(edgeProgress[0], 0);
  assert.equal(edgeProgress.at(-1), 1);
  assert.ok(edgeProgress.some((fraction) => fraction > 0 && fraction < 1));

  const edgeController = new AbortController();
  await assert.rejects(
    client.request("tessellateEdges", { shape }, {
      signal: edgeController.signal,
      onProgress: (event) => {
        if (event.fraction > 0 && event.fraction < 1) {
          edgeController.abort(new Error("edge tessellation cancelled"));
        }
      },
    }),
    /edge tessellation cancelled/,
  );

  const controller = new AbortController();
  const cancellationProgress = [];
  await assert.rejects(
    client.request("tessellate", { shape, linearDeflection: 0.01 }, {
      signal: controller.signal,
      onProgress: (event) => {
        cancellationProgress.push(event.fraction);
        if (event.fraction > 0 && event.fraction < 1) {
          controller.abort(new Error("tessellation cancelled"));
        }
      },
    }),
    /tessellation cancelled/,
  );
  assert.ok(cancellationProgress.some((fraction) => fraction > 0 && fraction < 1));
  assert.equal((await client.stats()).liveShapeHandles, boxes.length + 1);

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

test("OBJ and ASCII PLY memory-stream round trips preserve triangulated faces", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);

  const obj = await client.exportOBJ(box, { linearDeflection: 0.05 });
  assert.equal((await client.probeFormat(obj)).format, "obj");
  const objText = new TextDecoder().decode(obj);
  assert.match(objText, /^vt /m);
  assert.match(objText, /^vn /m);
  const objShape = await scope.importOBJ(obj);
  const objCounts = await client.topologyCounts(objShape);
  assert.ok(obj.byteLength > 0);
  assert.ok(objCounts.face > 0);
  assert.equal(await client.isValid(objShape), true);

  const attributeObj = new TextEncoder().encode(
    "v 0 0 0\nv 1 0 0\nv 0 1 0\n"
      + "vt 0 0\nvt 1 0\nvt 0 1\n"
      + "vn 0 0 1\n"
      + "f 1/1/1 2/2/1 3/3/1\n",
  );
  const attributeShape = await scope.importOBJ(attributeObj.buffer);
  const attributeFace = await scope.getSubShape(attributeShape, "face", 0);
  const attributeMesh = await client.triangulationData(attributeFace, { includeUV: true });
  assert.equal(attributeMesh.uvs.length, 6);
  assert.deepEqual(Array.from(attributeMesh.normals.slice(0, 3)), [0, 0, 1]);
  assert.deepEqual(Array.from(attributeMesh.uvs), [0, 0, 1, 0, 0, 1]);

  const ply = await client.exportPLY(box, { linearDeflection: 0.05 });
  assert.deepEqual(await client.probeFormat(ply), {
    format: "ply", encoding: "text", confidence: "exact",
  });
  const plyText = new TextDecoder().decode(ply);
  assert.match(plyText, /property double nx/);
  assert.match(plyText, /property double s/);
  const plyShape = await scope.importPLY(ply);
  const plyCounts = await client.topologyCounts(plyShape);
  assert.ok(ply.byteLength > 0);
  assert.ok(plyCounts.face > 0);
  assert.equal(await client.isValid(plyShape), true);
  const plyFace = await scope.getSubShape(plyShape, "face", 0);
  const plyAttributes = await client.triangulationData(plyFace, { includeUV: true });
  assert.equal(plyAttributes.normals.length, plyAttributes.positions.length);
  assert.equal(plyAttributes.uvs.length, plyAttributes.positions.length / 3 * 2);

  const binaryPly = await client.exportPLY(box, {
    linearDeflection: 0.05,
    encoding: "binary_little_endian",
  });
  assert.deepEqual(await client.probeFormat(binaryPly), {
    format: "ply", encoding: "binary", confidence: "exact",
  });
  const binaryShape = await scope.importPLY(binaryPly);
  assert.ok((await client.topologyCounts(binaryShape)).face > 0);
  assert.equal(await client.isValid(binaryShape), true);
  const binaryFace = await scope.getSubShape(binaryShape, "face", 0);
  const binaryAttributes = await client.triangulationData(binaryFace, { includeUV: true });
  assert.equal(binaryAttributes.normals.length, binaryAttributes.positions.length);
  assert.equal(binaryAttributes.uvs.length, binaryAttributes.positions.length / 3 * 2);

  const binaryBigEndianPly = await client.exportPLY(box, {
    linearDeflection: 0.05,
    encoding: "binary_big_endian",
  });
  assert.deepEqual(await client.probeFormat(binaryBigEndianPly), {
    format: "ply", encoding: "binary", confidence: "exact",
  });
  const binaryBigEndianShape = await scope.importPLY(binaryBigEndianPly);
  assert.ok((await client.topologyCounts(binaryBigEndianShape)).face > 0);
  assert.equal(await client.isValid(binaryBigEndianShape), true);

  const coloredPly = new TextEncoder().encode(
    "ply\nformat ascii 1.0\n"
      + "comment source application\ncomment calibrated mesh\n"
      + "obj_info coordinate_frame machine\nobj_info units millimeter\n"
      + "element vertex 3\n"
      + "property float x\nproperty float y\nproperty float z\n"
      + "property uchar red\nproperty uchar green\nproperty uchar blue\nproperty uchar alpha\n"
      + "element face 1\nproperty list uchar uint vertex_indices\nend_header\n"
      + "0 0 0 255 0 0 255\n1 0 0 0 255 0 255\n0 1 0 0 0 255 128\n"
      + "3 0 1 2\n",
  );
  const coloredMesh = await scope.importPLY(coloredPly.buffer, { includeMesh: true });
  assert.deepEqual(coloredMesh.document, {
    comments: ["source application", "calibrated mesh"],
    objectInfo: ["coordinate_frame machine", "units millimeter"],
  });
  assert.deepEqual(Array.from(coloredMesh.colors), [255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 128]);
  assert.deepEqual(Array.from(coloredMesh.indices), [0, 1, 2]);
  const coloredAscii = await client.exportPLY(coloredMesh, { encoding: "ascii" });
  const coloredAsciiText = new TextDecoder().decode(coloredAscii);
  assert.match(coloredAsciiText, /property uchar red/);
  assert.match(coloredAsciiText, /255 0 0 255/);
  assert.match(coloredAsciiText, /comment source application\ncomment calibrated mesh\n/);
  assert.match(coloredAsciiText, /obj_info coordinate_frame machine\nobj_info units millimeter\n/);
  const coloredAsciiRoundTrip = await scope.importPLY(coloredAscii, { includeMesh: true });
  assert.deepEqual(coloredAsciiRoundTrip.document, coloredMesh.document);
  const coloredBinary = await client.exportPLY(coloredMesh, { encoding: "binary_little_endian" });
  const coloredRoundTrip = await scope.importPLY(coloredBinary, { includeMesh: true });
  assert.deepEqual(coloredRoundTrip.document, coloredMesh.document);
  assert.deepEqual(Array.from(coloredRoundTrip.colors), Array.from(coloredMesh.colors));
  assert.deepEqual(Array.from(coloredRoundTrip.indices), [0, 1, 2]);
  const coloredBigEndian = await client.exportPLY(coloredMesh, { encoding: "binary_big_endian" });
  const coloredBigEndianRoundTrip = await scope.importPLY(coloredBigEndian, { includeMesh: true });
  assert.deepEqual(coloredBigEndianRoundTrip.document, coloredMesh.document);
  assert.deepEqual(Array.from(coloredBigEndianRoundTrip.colors), Array.from(coloredMesh.colors));
  assert.deepEqual(Array.from(coloredBigEndianRoundTrip.indices), [0, 1, 2]);

  const bigHeader = new TextEncoder().encode(
    "ply\nformat binary_big_endian 1.0\nelement vertex 3\n"
      + "property float x\nproperty float y\nproperty float z\n"
      + "element face 1\nproperty list uchar uint vertex_indices\nend_header\n",
  );
  const bigBody = new Uint8Array(3 * 12 + 1 + 3 * 4);
  const bigView = new DataView(bigBody.buffer);
  [[0, 0, 0], [2, 0, 0], [0, 2, 0]].forEach((point, vertex) => {
    point.forEach((value, coordinate) => bigView.setFloat32(vertex * 12 + coordinate * 4, value, false));
  });
  bigBody[36] = 3;
  bigView.setUint32(37, 0, false);
  bigView.setUint32(41, 1, false);
  bigView.setUint32(45, 2, false);
  const bigPly = new Uint8Array(bigHeader.byteLength + bigBody.byteLength);
  bigPly.set(bigHeader);
  bigPly.set(bigBody, bigHeader.byteLength);
  const bigShape = await scope.importPLY(bigPly.buffer);
  assert.equal((await client.topologyCounts(bigShape)).face, 1);
  assert.equal(await client.isValid(bigShape), true);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("OBJ document import preserves primitive state, mesh buffers, and companion MTL data", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const obj = new TextEncoder().encode(
    "mtllib materials.mtl\n"
      + "o first\n"
      + "g body front\n"
      + "usemtl red\n"
      + "s 1\n"
      + "v 0 0 0\n"
      + "v 1 0 0\n"
      + "v 0 1 0\n"
      + "v 0 0 1\n"
      + "vt 0 0\n"
      + "vt 1 0\n"
      + "vt 0 1\n"
      + "vn 0 0 1\n"
      + "f 1/1/1 2/2/1 3/3/1 # first triangle\n"
      + "o second\n"
      + "g cap\n"
      + "usemtl blue\n"
      + "s off\n"
      + "f 1/1/1 3/3/1 4/2/1\n",
  );
  const mtl = new TextEncoder().encode(
    "newmtl red\n"
      + "Ka 0.1 0.2 0.3\n"
      + "Kd 1 0 0\n"
      + "Ks 0.4 0.5 0.6\n"
      + "Ke 0.01 0.02 0.03\n"
      + "d 0.8\n"
      + "Ns 32\n"
      + "Ni 1.5\n"
      + "illum 2\n"
      + "map_Kd -clamp on red-diffuse.png\n"
      + "map_Ks red-specular.png\n"
      + "map_d red-opacity.png\n"
      + "bump -bm 0.5 red-normal.png\n"
      + "newmtl blue\n"
      + "Kd 0 0 1\n"
      + "Tr 0.25\n",
  );

  const imported = await scope.importOBJ(obj.buffer, {
    includeDocument: true,
    resources: { "materials.mtl": mtl.buffer },
  });
  assert.equal(await client.isValid(imported.shape), true);
  assert.equal((await client.topologyCounts(imported.shape)).face, 2);
  assert.deepEqual(Array.from(imported.positions), [
    0, 0, 0,
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
  ]);
  assert.deepEqual(Array.from(imported.indices), [0, 1, 2, 0, 2, 3]);
  assert.deepEqual(Array.from(imported.normals), [
    0, 0, 1,
    0, 0, 1,
    0, 0, 1,
    0, 0, 1,
  ]);
  assert.deepEqual(Array.from(imported.uvs), [0, 0, 1, 0, 0, 1, 1, 0]);
  assert.deepEqual(imported.document.materialLibraries, ["materials.mtl"]);
  assert.deepEqual(imported.document.primitives, [
    {
      indexStart: 0,
      indexCount: 3,
      object: "first",
      groups: ["body", "front"],
      material: "red",
      smoothingGroup: "1",
    },
    {
      indexStart: 3,
      indexCount: 3,
      object: "second",
      groups: ["cap"],
      material: "blue",
      smoothingGroup: "off",
    },
  ]);
  assert.deepEqual(imported.document.materials, [
    {
      name: "red",
      ambient: [0.1, 0.2, 0.3],
      diffuse: [1, 0, 0],
      specular: [0.4, 0.5, 0.6],
      emissive: [0.01, 0.02, 0.03],
      opacity: 0.8,
      shininess: 32,
      opticalDensity: 1.5,
      illuminationModel: 2,
      diffuseMap: "red-diffuse.png",
      specularMap: "red-specular.png",
      opacityMap: "red-opacity.png",
      bumpMap: "red-normal.png",
    },
    { name: "blue", diffuse: [0, 0, 1], opacity: 0.75 },
  ]);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("OBJ document export round-trips primitive and MTL metadata", async () => {
  const client = await DirectClient.create(wasm);
  const mesh = {
    positions: new Float32Array([
      0, 0, 0,
      1, 0, 0,
      0, 1, 0,
      0, 0, 1,
    ]),
    indices: new Uint32Array([0, 1, 2, 0, 2, 3]),
    normals: new Float32Array([
      0, 0, 1,
      0, 0, 1,
      0, 0, 1,
      0, 0, 1,
    ]),
    uvs: new Float32Array([0, 0, 1, 0, 0, 1, 1, 0]),
    document: {
      materialLibraries: ["source.mtl"],
      primitives: [
        { indexStart: 0, indexCount: 3, object: "body", groups: ["front"], material: "red", smoothingGroup: "1" },
        { indexStart: 3, indexCount: 3, object: "cap", groups: ["back"], material: "blue", smoothingGroup: "off" },
      ],
      materials: [
        { name: "red", ambient: [0.1, 0.2, 0.3], diffuse: [1, 0, 0], shininess: 16, diffuseMap: "red.png" },
        { name: "blue", diffuse: [0, 0, 1], opacity: 0.5, illuminationModel: 2 },
      ],
    },
  };

  const exported = await client.exportOBJ(mesh, { materialLibrary: "scene.mtl" });
  assert.deepEqual(Object.keys(exported.resources), ["scene.mtl"]);
  const objText = new TextDecoder().decode(exported.data);
  const mtlText = new TextDecoder().decode(exported.resources["scene.mtl"]);
  assert.match(objText, /^# occt-worker OBJ\nmtllib scene\.mtl\n/);
  assert.match(objText, /o body\ng front\nusemtl red\ns 1\nf 1\/1\/1 2\/2\/2 3\/3\/3\n/);
  assert.match(objText, /o cap\ng back\nusemtl blue\ns off\nf 1\/1\/1 3\/3\/3 4\/4\/4\n/);
  assert.match(mtlText, /newmtl red\nKa \S+ \S+ \S+\nKd 1 0 0\nNs 16\nmap_Kd red\.png\n/);

  const bare = await client.exportOBJ({ positions: mesh.positions, indices: mesh.indices });
  assert.deepEqual(bare.resources, {});
  assert.doesNotMatch(new TextDecoder().decode(bare.data), /^mtllib /m);
  await assert.rejects(
    client.exportOBJ({
      ...mesh,
      document: {
        ...mesh.document,
        primitives: [{ indexStart: 3, indexCount: 3, groups: [] }],
      },
    }),
    (error) => error.code === "InvalidArgs" && /partition/.test(error.message),
  );

  const scope = await client.beginScope();
  const imported = await scope.importOBJ(exported.data, {
    includeDocument: true,
    resources: exported.resources,
  });
  assert.deepEqual(imported.document.primitives, mesh.document.primitives);
  assert.deepEqual(imported.document.materials, mesh.document.materials);
  assert.deepEqual(imported.document.materialLibraries, ["scene.mtl"]);
  assert.deepEqual(Array.from(imported.indices), Array.from(mesh.indices));
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("OBJ and PLY imports reject malformed mesh payloads", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  await assert.rejects(
    scope.importOBJ(new TextEncoder().encode("v 0 0 0\nf 1 2 3\n").buffer),
    (error) => error.code === "ImportExportFailed",
  );
  await assert.rejects(
    scope.importPLY(new TextEncoder().encode("ply\nformat ascii 1.0\nend_header\n").buffer),
    (error) => error.code === "ImportExportFailed",
  );
  await scope.end();
});

test("glTF 2.0 JSON and GLB memory streams preserve shape bounds", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);

  const glb = await client.exportGLTF(box, { format: "glb", linearDeflection: 0.05 });
  assert.equal(new TextDecoder().decode(glb.slice(0, 4)), "glTF");
  assert.deepEqual(await client.probeFormat(glb), {
    format: "gltf", encoding: "glb", confidence: "exact",
  });
  const fromGlb = await scope.importGLTF(glb);
  const glbBounds = await client.bbox(fromGlb);
  assert.ok(glbBounds.min.every((value) => Math.abs(value) < 1e-6));
  assert.ok(glbBounds.max.every((value, index) => Math.abs(value - [2, 3, 4][index]) < 1e-6));
  assert.equal(await client.isValid(fromGlb), true);
  const glbFace = await scope.getSubShape(fromGlb, "face", 0);
  const glbAttributes = await client.triangulationData(glbFace, { includeUV: true });
  assert.equal(glbAttributes.normals.length, glbAttributes.positions.length);
  assert.equal(glbAttributes.uvs.length, glbAttributes.positions.length / 3 * 2);

  const gltf = await client.exportGLTF(box, { format: "gltf", linearDeflection: 0.05 });
  assert.deepEqual(await client.probeFormat(gltf), {
    format: "gltf", encoding: "json", confidence: "exact",
  });
  const document = JSON.parse(new TextDecoder().decode(gltf));
  assert.equal(document.asset.version, "2.0");
  assert.deepEqual(document.meshes[0].primitives[0].attributes, { POSITION: 0, NORMAL: 1, TEXCOORD_0: 2 });
  assert.match(document.buffers[0].uri, /^data:application\/octet-stream;base64,/);
  const fromGltf = await scope.importGLTF(gltf);
  const gltfBounds = await client.bbox(fromGltf);
  assert.ok(gltfBounds.min.every((value) => Math.abs(value) < 1e-6));
  assert.ok(gltfBounds.max.every((value, index) => Math.abs(value - [2, 3, 4][index]) < 1e-6));
  const gltfFace = await scope.getSubShape(fromGltf, "face", 0);
  const gltfAttributes = await client.triangulationData(gltfFace, { includeUV: true });
  assert.equal(gltfAttributes.normals.length, gltfAttributes.positions.length);
  assert.equal(gltfAttributes.uvs.length, gltfAttributes.positions.length / 3 * 2);

  document.nodes[0].translation = [5, 7, -3];
  const transformed = await scope.importGLTF(new TextEncoder().encode(JSON.stringify(document)).buffer);
  const transformedBounds = await client.bbox(transformed);
  assert.ok(Math.abs(transformedBounds.min[0] - 5) < 1e-6);
  assert.ok(Math.abs(transformedBounds.min[1] - 3) < 1e-6);
  assert.ok(Math.abs(transformedBounds.min[2] - 7) < 1e-6);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF document import preserves scenes, instances, materials, and resolved resources", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const mesh = new ArrayBuffer(42);
  const meshView = new DataView(mesh);
  [0, 0, 0, 1, 0, 0, 0, 1, 0].forEach((value, index) => meshView.setFloat32(index * 4, value, true));
  [0, 1, 2].forEach((value, index) => meshView.setUint16(36 + index * 2, value, true));
  const source = {
    asset: { version: "2.0", extras: { source: "fixture" } },
    extensionsUsed: ["VENDOR_metadata"],
    scene: 1,
    scenes: [{ nodes: [0] }, { nodes: [1], extras: { active: true } }],
    nodes: [
      { camera: 0 },
      { translation: [5, 0, 0], children: [2] },
      { mesh: 0, scale: [-1, 1, 1], extras: { instance: true } },
    ],
    meshes: [{ primitives: [
      { attributes: { POSITION: 0 }, indices: 1, material: 0 },
      { attributes: { POSITION: 0 }, indices: 1, material: 1 },
    ] }],
    materials: [
      { pbrMetallicRoughness: { baseColorTexture: { index: 0 }, metallicFactor: 0.25 } },
      { alphaMode: "BLEND", doubleSided: true, extensions: { VENDOR_material: { value: 1 } } },
    ],
    samplers: [{ magFilter: 9729 }],
    textures: [{ sampler: 0, source: 0 }],
    images: [{ uri: "albedo.png", mimeType: "image/png" }],
    cameras: [{ type: "perspective", perspective: { yfov: 1, znear: 0.1 } }],
    skins: [{ joints: [2] }],
    animations: [{ samplers: [{ input: 1, output: 0 }], channels: [{ sampler: 0, target: { node: 2, path: "translation" } }] }],
    buffers: [
      { uri: "positions.bin", byteLength: 36 },
      { uri: "indices.bin", byteLength: 6 },
    ],
    bufferViews: [
      { buffer: 0, byteOffset: 0, byteLength: 36 },
      { buffer: 1, byteOffset: 0, byteLength: 6 },
    ],
    accessors: [
      { bufferView: 0, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: 1, componentType: 5123, count: 3, type: "SCALAR" },
    ],
  };
  const data = new TextEncoder().encode(JSON.stringify(source)).buffer;
  const image = Uint8Array.of(137, 80, 78, 71).buffer;
  const positions = mesh.slice(0, 36);
  const indices = mesh.slice(36);
  const imported = await scope.importGLTF(data, {
    includeDocument: true,
    resources: { "positions.bin": positions, "indices.bin": indices, "albedo.png": image },
  });
  assert.ok(imported.shape);
  assert.equal(imported.activeScene, 1);
  assert.deepEqual(imported.sceneRoots, [1]);
  assert.equal(imported.primitives.length, 2);
  assert.deepEqual(imported.primitives.map(({ nodeIndex, material, vertexStart, indexStart }) => (
    { nodeIndex, material, vertexStart, indexStart }
  )), [
    { nodeIndex: 2, material: 0, vertexStart: 0, indexStart: 0 },
    { nodeIndex: 2, material: 1, vertexStart: 3, indexStart: 3 },
  ]);
  assert.deepEqual([...imported.indices], [0, 2, 1, 3, 5, 4]);
  assert.deepEqual(imported.document, source);
  assert.deepEqual(imported.buffers.map(({ uri }) => uri), ["positions.bin", "indices.bin"]);
  assert.deepEqual([...new Uint8Array(imported.buffers[0].data)], [...new Uint8Array(positions)]);
  assert.deepEqual([...new Uint8Array(imported.buffers[1].data)], [...new Uint8Array(indices)]);
  assert.deepEqual(imported.resources.map(({ uri }) => uri), ["albedo.png"]);
  assert.deepEqual([...new Uint8Array(imported.resources[0].data)], [...new Uint8Array(image)]);

  const exportedJSON = await client.exportGLTF(imported, { format: "gltf" });
  assert.deepEqual(JSON.parse(new TextDecoder().decode(exportedJSON.data)), source);
  assert.deepEqual(Object.keys(exportedJSON.resources).sort(), ["albedo.png", "indices.bin", "positions.bin"]);
  const jsonRoundTrip = await scope.importGLTF(exportedJSON.data, {
    includeDocument: true,
    resources: exportedJSON.resources,
  });
  assert.deepEqual(jsonRoundTrip.document, source);
  assert.deepEqual(jsonRoundTrip.primitives, imported.primitives);

  const exportedGLB = await client.exportGLTF(imported, { format: "glb" });
  assert.deepEqual(Object.keys(exportedGLB.resources), ["albedo.png"]);
  assert.deepEqual(await client.probeFormat(exportedGLB.data), {
    format: "gltf", encoding: "glb", confidence: "exact",
  });
  const glbRoundTrip = await scope.importGLTF(exportedGLB.data, {
    includeDocument: true,
    resources: exportedGLB.resources,
  });
  assert.equal(glbRoundTrip.activeScene, 1);
  assert.deepEqual(glbRoundTrip.sceneRoots, [1]);
  assert.deepEqual(glbRoundTrip.document.materials, source.materials);
  assert.deepEqual(glbRoundTrip.document.animations, source.animations);
  assert.deepEqual(glbRoundTrip.document.skins, source.skins);
  assert.deepEqual(glbRoundTrip.positions, imported.positions);
  assert.deepEqual(glbRoundTrip.indices, imported.indices);

  await assert.rejects(
    scope.importGLTF(data, {
      includeDocument: true,
      resources: { "positions.bin": positions, "indices.bin": indices },
    }),
    (error) => error.code === "ImportExportFailed" && /albedo\.png/.test(error.message),
  );
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF sparse accessors override positions, normals, UVs, and indices", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const mesh = new ArrayBuffer(96);
  const view = new DataView(mesh);
  [0, 0, 0, 1, 0, 0, 0, 1, 0].forEach((value, index) => {
    view.setFloat32(index * 4, value, true);
  });
  [0, 127, 0, 0, 127, 0, 0, 127, 0].forEach((value, index) => {
    view.setInt8(36 + index, value);
  });
  [0, 0, 65535, 0, 0, 65535].forEach((value, index) => {
    view.setUint16(46 + index * 2, value, true);
  });
  [0, 0, 0].forEach((value, index) => view.setUint16(58 + index * 2, value, true));
  view.setUint8(64, 1);
  [2, 0, 0].forEach((value, index) => view.setFloat32(68 + index * 4, value, true));
  view.setUint8(80, 2);
  [127, 0, 0].forEach((value, index) => view.setInt8(81 + index, value));
  view.setUint8(84, 1);
  [32768, 65535].forEach((value, index) => view.setUint16(86 + index * 2, value, true));
  view.setUint8(90, 1);
  view.setUint8(91, 2);
  view.setUint16(92, 1, true);
  view.setUint16(94, 2, true);

  const source = {
    asset: { version: "2.0" },
    scene: 0,
    scenes: [{ nodes: [0] }],
    nodes: [{ mesh: 0 }],
    meshes: [{ primitives: [{
      attributes: { POSITION: 0, NORMAL: 1, TEXCOORD_0: 2 },
      indices: 3,
    }] }],
    buffers: [{
      byteLength: mesh.byteLength,
      uri: `data:application/octet-stream;base64,${Buffer.from(mesh).toString("base64")}`,
    }],
    bufferViews: [
      { buffer: 0, byteOffset: 0, byteLength: 36 },
      { buffer: 0, byteOffset: 36, byteLength: 9 },
      { buffer: 0, byteOffset: 46, byteLength: 12 },
      { buffer: 0, byteOffset: 58, byteLength: 6 },
      { buffer: 0, byteOffset: 64, byteLength: 1 },
      { buffer: 0, byteOffset: 68, byteLength: 12 },
      { buffer: 0, byteOffset: 80, byteLength: 1 },
      { buffer: 0, byteOffset: 81, byteLength: 3 },
      { buffer: 0, byteOffset: 84, byteLength: 1 },
      { buffer: 0, byteOffset: 86, byteLength: 4 },
      { buffer: 0, byteOffset: 90, byteLength: 2 },
      { buffer: 0, byteOffset: 92, byteLength: 4 },
    ],
    accessors: [
      {
        bufferView: 0, componentType: 5126, count: 3, type: "VEC3",
        sparse: { count: 1, indices: { bufferView: 4, componentType: 5121 }, values: { bufferView: 5 } },
      },
      {
        bufferView: 1, componentType: 5120, normalized: true, count: 3, type: "VEC3",
        sparse: { count: 1, indices: { bufferView: 6, componentType: 5121 }, values: { bufferView: 7 } },
      },
      {
        bufferView: 2, componentType: 5123, normalized: true, count: 3, type: "VEC2",
        sparse: { count: 1, indices: { bufferView: 8, componentType: 5121 }, values: { bufferView: 9 } },
      },
      {
        componentType: 5123, count: 3, type: "SCALAR",
        sparse: { count: 2, indices: { bufferView: 10, componentType: 5121 }, values: { bufferView: 11 } },
      },
    ],
  };
  const imported = await scope.importGLTF(
    new TextEncoder().encode(JSON.stringify(source)).buffer,
    { includeDocument: true },
  );
  assert.deepEqual([...imported.positions].map((value) => Math.abs(value)), [0, 0, 0, 2, 0, 0, 0, 0, 1]);
  assert.deepEqual([...imported.indices], [0, 1, 2]);
  assert.deepEqual([...imported.normals].map((value) => Math.abs(value)), [0, 0, 1, 0, 0, 1, 1, 0, 0]);
  assert.ok(Math.abs(imported.uvs[2] - 32768 / 65535) < 1e-6);
  assert.deepEqual([...imported.uvs].slice(0, 2), [0, 0]);
  assert.deepEqual([...imported.uvs].slice(3), [1, 0, 1]);
  assert.deepEqual(imported.document, source);

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF import statically evaluates sparse POSITION and dense NORMAL morph targets", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const mesh = new ArrayBuffer(124);
  const view = new DataView(mesh);
  [0, 0, 0, 1, 0, 0, 0, 1, 0].forEach((value, index) => {
    view.setFloat32(index * 4, value, true);
  });
  [0, 0, 1, 0, 0, 1, 0, 0, 1].forEach((value, index) => {
    view.setFloat32(36 + index * 4, value, true);
  });
  view.setUint8(72, 1);
  [1, 0, 0].forEach((value, index) => view.setFloat32(76 + index * 4, value, true));
  [0, 1, -1, 0, 1, -1, 0, 1, -1].forEach((value, index) => {
    view.setFloat32(88 + index * 4, value, true);
  });
  const source = {
    asset: { version: "2.0" },
    scene: 0,
    scenes: [{ nodes: [0, 1] }],
    nodes: [{ mesh: 0, weights: [0.5] }, { mesh: 0 }],
    meshes: [{
      weights: [0.25],
      primitives: [{
        attributes: { POSITION: 0, NORMAL: 1 },
        targets: [{ POSITION: 2, NORMAL: 3 }],
      }],
    }],
    buffers: [{
      byteLength: mesh.byteLength,
      uri: `data:application/octet-stream;base64,${Buffer.from(mesh).toString("base64")}`,
    }],
    bufferViews: [
      { buffer: 0, byteOffset: 0, byteLength: 36 },
      { buffer: 0, byteOffset: 36, byteLength: 36 },
      { buffer: 0, byteOffset: 72, byteLength: 1 },
      { buffer: 0, byteOffset: 76, byteLength: 12 },
      { buffer: 0, byteOffset: 88, byteLength: 36 },
    ],
    accessors: [
      { bufferView: 0, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: 1, componentType: 5126, count: 3, type: "VEC3" },
      {
        componentType: 5126, count: 3, type: "VEC3",
        sparse: { count: 1, indices: { bufferView: 2, componentType: 5121 }, values: { bufferView: 3 } },
      },
      { bufferView: 4, componentType: 5126, count: 3, type: "VEC3" },
    ],
  };
  const data = new TextEncoder().encode(JSON.stringify(source)).buffer;
  const imported = await scope.importGLTF(data, { includeDocument: true });
  assert.ok(Math.abs(imported.positions[3] - 1.5) < 1e-6);
  assert.ok(Math.abs(imported.positions[12] - 1.25) < 1e-6);
  assert.ok(Math.abs(imported.normals[1] + Math.SQRT1_2) < 1e-6);
  assert.ok(Math.abs(imported.normals[2] - Math.SQRT1_2) < 1e-6);
  assert.ok(Math.abs(imported.normals[10] + 3 / Math.sqrt(10)) < 1e-6);
  assert.ok(Math.abs(imported.normals[11] - 1 / Math.sqrt(10)) < 1e-6);
  assert.deepEqual(imported.document, source);

  const overridden = await scope.importGLTF(data, { includeDocument: true, morphWeights: [1] });
  assert.ok(Math.abs(overridden.positions[3] - 2) < 1e-6);
  assert.ok(Math.abs(overridden.positions[12] - 2) < 1e-6);
  await assert.rejects(
    scope.importGLTF(data, { morphWeights: [] }),
    (error) => error.code === "InvalidArgs" && /target count/.test(error.message),
  );

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF import samples node transforms and morph weights", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const bytes = [];
  const bufferViews = [];
  const append = (array) => {
    while (bytes.length % 4 !== 0) bytes.push(0);
    const byteOffset = bytes.length;
    bytes.push(...new Uint8Array(array.buffer, array.byteOffset, array.byteLength));
    bufferViews.push({ buffer: 0, byteOffset, byteLength: array.byteLength });
    return bufferViews.length - 1;
  };
  const positionView = append(new Float32Array([
    0, 0, 0, 1, 0, 0, 0, 1, 0,
  ]));
  const morphView = append(new Float32Array([
    1, 0, 0, 1, 0, 0, 1, 0, 0,
  ]));
  const timeView = append(new Float32Array([0, 1]));
  const translationView = append(new Float32Array([
    0, 0, 0, 10, 0, 0,
  ]));
  const halfSqrt = Math.SQRT1_2;
  const rotationView = append(new Float32Array([
    0, 0, 0, 1, 0, 0, -halfSqrt, -halfSqrt,
  ]));
  const scaleView = append(new Float32Array([
    0, 0, 0, 1, 1, 1, 1, 0, 0,
    1, 0, 0, 2, 1, 1, 0, 0, 0,
  ]));
  const weightView = append(new Float32Array([0, 1]));
  const binary = Uint8Array.from(bytes);
  const source = {
    asset: { version: "2.0" },
    scene: 0,
    scenes: [{ nodes: [0] }],
    nodes: [{ mesh: 0 }],
    meshes: [{ primitives: [{
      attributes: { POSITION: 0 },
      targets: [{ POSITION: 1 }],
    }] }],
    animations: [{
      samplers: [
        { input: 2, output: 3, interpolation: "STEP" },
        { input: 2, output: 4, interpolation: "LINEAR" },
        { input: 2, output: 5, interpolation: "CUBICSPLINE" },
        { input: 2, output: 6, interpolation: "LINEAR" },
      ],
      channels: [
        { sampler: 0, target: { node: 0, path: "translation" } },
        { sampler: 1, target: { node: 0, path: "rotation" } },
        { sampler: 2, target: { node: 0, path: "scale" } },
        { sampler: 3, target: { node: 0, path: "weights" } },
      ],
    }],
    buffers: [{
      byteLength: binary.byteLength,
      uri: `data:application/octet-stream;base64,${Buffer.from(binary).toString("base64")}`,
    }],
    bufferViews,
    accessors: [
      { bufferView: positionView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: morphView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: timeView, componentType: 5126, count: 2, type: "SCALAR" },
      { bufferView: translationView, componentType: 5126, count: 2, type: "VEC3" },
      { bufferView: rotationView, componentType: 5126, count: 2, type: "VEC4" },
      { bufferView: scaleView, componentType: 5126, count: 6, type: "VEC3" },
      { bufferView: weightView, componentType: 5126, count: 2, type: "SCALAR" },
    ],
  };
  const data = new TextEncoder().encode(JSON.stringify(source)).buffer;
  const imported = await scope.importGLTF(data, {
    includeDocument: true,
    animationIndex: 0,
    animationTime: 0.5,
  });
  assert.deepEqual(imported.document, source);
  const expected = 0.75 * Math.SQRT1_2;
  assert.ok(Math.abs(imported.positions[0] - expected) < 1e-6);
  assert.ok(Math.abs(imported.positions[1]) < 1e-6);
  assert.ok(Math.abs(imported.positions[2] - expected) < 1e-6);
  assert.ok(Math.abs(imported.positions[3] - 3 * expected) < 1e-6);
  assert.ok(Math.abs(imported.positions[5] - 3 * expected) < 1e-6);

  await assert.rejects(
    scope.importGLTF(data, { animationIndex: 0 }),
    (error) => error.code === "InvalidArgs" && /supplied together/.test(error.message),
  );
  const duplicate = structuredClone(source);
  duplicate.animations[0].channels.push(structuredClone(duplicate.animations[0].channels[0]));
  await assert.rejects(
    scope.importGLTF(new TextEncoder().encode(JSON.stringify(duplicate)).buffer, {
      animationIndex: 0,
      animationTime: 0.5,
    }),
    (error) => error.code === "ImportExportFailed" && /duplicate target/.test(error.message),
  );
  const badSampler = structuredClone(source);
  badSampler.accessors[4].count = 1;
  await assert.rejects(
    scope.importGLTF(new TextEncoder().encode(JSON.stringify(badSampler)).buffer, {
      animationIndex: 0,
      animationTime: 0.5,
    }),
    (error) => error.code === "ImportExportFailed" && /output count/.test(error.message),
  );

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF import applies static skin deformation after morph targets", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const bytes = [];
  const bufferViews = [];
  const append = (array) => {
    while (bytes.length % 4 !== 0) bytes.push(0);
    const byteOffset = bytes.length;
    bytes.push(...new Uint8Array(array.buffer, array.byteOffset, array.byteLength));
    bufferViews.push({ buffer: 0, byteOffset, byteLength: array.byteLength });
    return bufferViews.length - 1;
  };
  const positionView = append(new Float32Array([0, 0, 0, 1, 0, 0, 0, 1, 0]));
  const normalView = append(new Float32Array([0, 1, 0, 0, 1, 0, 0, 1, 0]));
  const morphPositionView = append(new Float32Array([1, 0, 0, 1, 0, 0, 1, 0, 0]));
  const morphNormalView = append(new Float32Array([1, -1, 0, 1, -1, 0, 1, -1, 0]));
  const joints8View = append(new Uint8Array([
    0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
  ]));
  const weights8View = append(new Uint8Array([
    255, 0, 0, 0, 255, 0, 0, 0, 128, 127, 0, 0,
  ]));
  const joints16View = append(new Uint16Array([
    0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
  ]));
  const weights16View = append(new Uint16Array([
    65535, 0, 0, 0, 65535, 0, 0, 0, 32768, 32767, 0, 0,
  ]));
  const weightsFloatView = append(new Float32Array([
    1, 0, 0, 0, 1, 0, 0, 0, 0.5, 0.5, 0, 0,
  ]));
  const identity = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];
  const translatedInverseBind = [...identity];
  translatedInverseBind[12] = -0.25;
  const inverseBindView = append(new Float32Array([
    ...translatedInverseBind, ...identity, ...identity,
  ]));
  const binary = Uint8Array.from(bytes);
  const commonPrimitive = {
    attributes: { POSITION: 0, NORMAL: 1 },
    targets: [{ POSITION: 2, NORMAL: 3 }],
  };
  const source = {
    asset: { version: "2.0" },
    scene: 0,
    scenes: [{ nodes: [3] }],
    nodes: [
      { mesh: 0, skin: 0, translation: [1, 0, 0], scale: [0, 0, 0] },
      { translation: [1, 2, 0], scale: [2, 1, 1] },
      { translation: [1, 0, 2] },
      { translation: [5, 0, 0], children: [0, 1, 2] },
    ],
    skins: [{ joints: [1, 2], inverseBindMatrices: 9 }],
    meshes: [{
      weights: [0.5],
      primitives: [
        { ...commonPrimitive, attributes: { ...commonPrimitive.attributes, JOINTS_0: 4, WEIGHTS_0: 5 } },
        { ...commonPrimitive, attributes: { ...commonPrimitive.attributes, JOINTS_0: 6, WEIGHTS_0: 7 } },
        { ...commonPrimitive, attributes: { ...commonPrimitive.attributes, JOINTS_0: 4, WEIGHTS_0: 8 } },
      ],
    }],
    buffers: [{
      byteLength: binary.byteLength,
      uri: `data:application/octet-stream;base64,${Buffer.from(binary).toString("base64")}`,
    }],
    bufferViews,
    accessors: [
      { bufferView: positionView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: normalView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: morphPositionView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: morphNormalView, componentType: 5126, count: 3, type: "VEC3" },
      { bufferView: joints8View, componentType: 5121, count: 3, type: "VEC4" },
      { bufferView: weights8View, componentType: 5121, normalized: true, count: 3, type: "VEC4" },
      { bufferView: joints16View, componentType: 5123, count: 3, type: "VEC4" },
      { bufferView: weights16View, componentType: 5123, normalized: true, count: 3, type: "VEC4" },
      { bufferView: weightsFloatView, componentType: 5126, count: 3, type: "VEC4" },
      { bufferView: inverseBindView, componentType: 5126, count: 3, type: "MAT4" },
    ],
  };
  const data = new TextEncoder().encode(JSON.stringify(source)).buffer;
  const imported = await scope.importGLTF(data, { includeDocument: true });
  assert.deepEqual(imported.document, source);
  assert.equal(imported.primitives.length, 3);
  for (const vertexStart of [0, 3, 6]) {
    const offset = vertexStart * 3;
    assert.ok(Math.abs(imported.positions[offset] - 6.5) < 1e-6);
    assert.ok(Math.abs(imported.positions[offset + 2] - 2) < 1e-6);
    assert.ok(Math.abs(imported.positions[offset + 3] - 7.5) < 1e-6);
    assert.ok(Math.abs(imported.positions[offset + 4] + 2) < 1e-6);
    assert.ok(Math.abs(imported.normals[offset] - 1 / Math.sqrt(5)) < 1e-6);
    assert.ok(Math.abs(imported.normals[offset + 2] - 2 / Math.sqrt(5)) < 1e-6);
  }
  assert.ok(Math.abs(imported.positions[6] - 6.5) < 1e-6);
  assert.ok(Math.abs(imported.positions[7] + 2 * 127 / 255) < 1e-6);
  assert.ok(Math.abs(imported.positions[8] - (1 + 2 * 128 / 255)) < 1e-6);
  assert.ok(Math.abs(imported.positions[15] - 6.5) < 1e-6);
  assert.ok(Math.abs(imported.positions[16] + 2 * 32767 / 65535) < 1e-6);
  assert.ok(Math.abs(imported.positions[17] - (1 + 2 * 32768 / 65535)) < 1e-6);
  assert.ok(Math.abs(imported.positions[24] - 6.5) < 1e-6);
  assert.ok(Math.abs(imported.positions[25] + 1) < 1e-6);
  assert.ok(Math.abs(imported.positions[26] - 2) < 1e-6);

  const malformed = structuredClone(source);
  delete malformed.meshes[0].primitives[0].attributes.WEIGHTS_0;
  await assert.rejects(
    scope.importGLTF(new TextEncoder().encode(JSON.stringify(malformed)).buffer),
    (error) => error.code === "ImportExportFailed" && /JOINTS_0 and WEIGHTS_0/.test(error.message),
  );

  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("glTF document import accepts non-mesh documents and validates required extensions", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const cameraOnly = {
    asset: { version: "2.0" },
    scene: 0,
    scenes: [{ nodes: [0] }],
    nodes: [{ camera: 0 }],
    cameras: [{ type: "orthographic", orthographic: { xmag: 1, ymag: 1, znear: 0, zfar: 10 } }],
    images: [{ uri: "data:application/octet-stream,%41%42" }],
  };
  const imported = await scope.importGLTF(
    new TextEncoder().encode(JSON.stringify(cameraOnly)).buffer,
    { includeDocument: true },
  );
  assert.equal(imported.shape, undefined);
  assert.equal(imported.positions.length, 0);
  assert.equal(imported.indices.length, 0);
  assert.deepEqual([...new Uint8Array(imported.resources[0].data)], [65, 66]);
  await assert.rejects(
    scope.importGLTF(new TextEncoder().encode(JSON.stringify({
      asset: { version: "2.0" },
      extensionsRequired: ["KHR_draco_mesh_compression"],
    })).buffer, { includeDocument: true }),
    (error) => error.code === "ImportExportFailed" && /KHR_draco_mesh_compression/.test(error.message),
  );
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveBufferBytes, 0);
});

test("importGLTF rejects malformed payloads", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  await assert.rejects(
    scope.importGLTF(new TextEncoder().encode('{"asset":{"version":"1.0"}}').buffer),
    (error) => error.code === "ImportExportFailed",
  );
  await scope.end();
});

test("VRML 2.0 memory streams round-trip through OCCT", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const vrml = await client.exportVRML(box, { version: 2, linearDeflection: 0.05 });
  assert.match(new TextDecoder().decode(vrml.slice(0, 32)), /^#VRML V2\.0/);
  assert.deepEqual(await client.probeFormat(vrml), {
    format: "vrml", encoding: "text", confidence: "exact",
  });
  const restored = await scope.importVRML(vrml);
  const bounds = await client.bbox(restored);
  assert.ok(bounds.min.every((value) => Math.abs(value) < 1e-6));
  assert.ok(bounds.max.every((value, index) => Math.abs(value - [2, 3, 4][index]) < 1e-6));
  assert.equal(await client.isValid(restored), true);
  const restoredMesh = await scope.importVRML(vrml, { includeMesh: true });
  assert.ok(restoredMesh.positions.length > 0);
  assert.equal(restoredMesh.positions.length, restoredMesh.normals.length);
  assert.equal(restoredMesh.indices.length % 3, 0);
  assert.ok(restoredMesh.indices.length > 0);
  const meshVRML = await client.exportVRML({
    positions: restoredMesh.positions,
    indices: restoredMesh.indices,
    normals: restoredMesh.normals,
    ...(restoredMesh.uvs === undefined ? {} : { uvs: restoredMesh.uvs }),
  }, { version: 2 });
  assert.match(new TextDecoder().decode(meshVRML.slice(0, 32)), /^#VRML V2\.0/);
  const meshRoundTrip = await scope.importVRML(meshVRML, { includeMesh: true });
  assert.equal(meshRoundTrip.indices.length, restoredMesh.indices.length);
  assert.equal(meshRoundTrip.positions.length, restoredMesh.positions.length);
  const attributedVRML = await client.exportVRML({
    positions: new Float32Array([0, 0, 0, 1, 0, 0, 0, 1, 0]),
    indices: new Uint32Array([0, 1, 2]),
    normals: new Float32Array([0, 0, 1, 0, 0, 1, 0, 0, 1]),
    uvs: new Float32Array([0, 0, 1, 0, 0, 1]),
    colors: new Uint8Array([255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255]),
  }, { version: 2 });
  const attributedRoundTrip = await scope.importVRML(attributedVRML, { includeMesh: true });
  assert.deepEqual(Array.from(attributedRoundTrip.positions), [0, 0, 0, 1, 0, 0, 0, 1, 0]);
  assert.deepEqual(Array.from(attributedRoundTrip.indices), [0, 1, 2]);
  assert.deepEqual(Array.from(attributedRoundTrip.normals), [0, 0, 1, 0, 0, 1, 0, 0, 1]);
  assert.deepEqual(Array.from(attributedRoundTrip.uvs), [0, 0, 1, 0, 0, 1]);
  assert.deepEqual(Array.from(attributedRoundTrip.colors),
    [255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255]);
  await assert.rejects(
    client.exportVRML({
      positions: new Float32Array([0, 0, 0, 1, 0, 0, 0, 1, 0]),
      indices: new Uint32Array([0, 1, 2]),
      colors: new Uint8Array([255, 0, 0, 128, 0, 255, 0, 255, 0, 0, 255, 255]),
    }),
    (error) => error.code === "InvalidArgs" && /opaque alpha/.test(error.message),
  );
  const independentlyIndexedVRML = new TextEncoder().encode(`#VRML V2.0 utf8
Shape { geometry IndexedFaceSet {
 coord Coordinate { point [ 0 0 0, 1 0 0, 1 0 -1, 0 0 -1 ] }
 coordIndex [ 0 1 2 3 -1 ]
 texCoord TextureCoordinate { point [ 0 0, 1 0, 1 1, 0 1 ] }
 texCoordIndex [ 3 2 1 0 -1 ]
 solid FALSE
} }
`).buffer;
  const independentlyIndexed = await scope.importVRML(independentlyIndexedVRML, { includeMesh: true });
  assert.deepEqual(Array.from(independentlyIndexed.indices), [0, 1, 2, 3, 4, 5]);
  assert.deepEqual(Array.from(independentlyIndexed.uvs), [0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0]);
  await scope.end();
  const stats = await client.stats();
  assert.equal(stats.liveShapeHandles, 0);
  assert.equal(stats.liveBufferBytes, 0);
});

test("VRML export reports OCCT progress", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const boxes = [];
  for (let index = 0; index < 24; index++) {
    boxes.push(await scope.makeBox([10, 20, 30], [index * 12, 0, 0]));
  }
  const shape = await scope.makeCompound(boxes);
  const exportProgress = [];
  const exported = await client.request("exportVRML", { shape, version: 2 }, {
    onProgress: (event) => exportProgress.push(event.fraction),
  });
  assert.equal(exportProgress[0], 0);
  assert.equal(exportProgress.at(-1), 1);
  assert.ok(exportProgress.some((fraction) => fraction > 0 && fraction < 1));

  const exportController = new AbortController();
  const cancellationProgress = [];
  await assert.rejects(
    client.request("exportVRML", { shape, version: 2, linearDeflection: 0.01 }, {
      signal: exportController.signal,
      onProgress: (event) => {
        cancellationProgress.push(event.fraction);
        if (event.fraction > 0 && event.fraction < 1)
          exportController.abort(new Error("VRML export cancelled"));
      },
    }),
    /VRML export cancelled/,
  );
  assert.ok(cancellationProgress.some((fraction) => fraction > 0 && fraction < 1));

  const importProgress = [];
  await client.request("importVRML", {
    scopeId: scope.scopeId,
    data: { $inputBuffer: exported.data.data.slice(0) },
  }, { onProgress: (event) => importProgress.push(event.fraction) });
  assert.equal(importProgress[0], 0);
  assert.equal(importProgress.at(-1), 1);

  await scope.end();
});

test("importVRML rejects malformed payloads", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  await assert.rejects(
    scope.importVRML(new TextEncoder().encode("not vrml").buffer),
    (error) => error.code === "ImportExportFailed",
  );
  await scope.end();
});
