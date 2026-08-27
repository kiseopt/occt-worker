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

