import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient, ParametricModel, solveSketch } from "../../dist/index.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

const point = (entity, location) => ({ entity, point: location });

test("2D sketch solver satisfies geometric and dimensional constraints", () => {
  const solved = solveSketch({
    entities: [
      { id: "bottom", type: "line", start: [0.2, -0.1], end: [9.5, 0.3] },
      { id: "right", type: "line", start: [9.7, 0.1], end: [10.2, 4.7] },
      { id: "top", type: "line", start: [10.1, 5.2], end: [0.4, 4.8] },
      { id: "left", type: "line", start: [-0.2, 5.1], end: [0.1, 0.2] },
    ],
    constraints: [
      { type: "fixed", point: point("bottom", "start"), value: [0, 0] },
      { type: "coincident", first: point("bottom", "end"), second: point("right", "start") },
      { type: "coincident", first: point("right", "end"), second: point("top", "start") },
      { type: "coincident", first: point("top", "end"), second: point("left", "start") },
      { type: "coincident", first: point("left", "end"), second: point("bottom", "start") },
      { type: "horizontal", entity: "bottom" },
      { type: "horizontal", entity: "top" },
      { type: "vertical", entity: "right" },
      { type: "vertical", entity: "left" },
      { type: "length", entity: "bottom", value: 10 },
      { type: "length", entity: "right", value: 5 },
      { type: "equalLength", first: "bottom", second: "top" },
      { type: "equalLength", first: "right", second: "left" },
    ],
  });
  assert.equal(solved.diagnostics.converged, true);
  assert.ok(solved.diagnostics.maxResidual < 1e-8);
  assert.equal(solved.diagnostics.degreesOfFreedom, 0);
  assert.equal(solved.diagnostics.constraintDiagnostics.length, 13);
  assert.ok(solved.diagnostics.constraintDiagnostics.every((diagnostic) => diagnostic.satisfied));
  const [bottom, right] = solved.entities;
  assert.ok(Math.abs(bottom.end[0] - bottom.start[0] - 10) < 1e-8);
  assert.ok(Math.abs(right.end[1] - right.start[1] - 5) < 1e-8);
});

test("parametric sketch produces a constrained wire for downstream features", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: { width: 10, height: 5, depth: 3 },
    features: [
      {
        id: "profile",
        type: "sketch",
        entities: [
          { id: "bottom", type: "line", start: [0, 0], end: [9, 0] },
          { id: "right", type: "line", start: [9, 0], end: [9, 4] },
          { id: "top", type: "line", start: [9, 4], end: [0, 4] },
          { id: "left", type: "line", start: [0, 4], end: [0, 0] },
        ],
        constraints: [
          { type: "fixed", point: point("bottom", "start"), value: [0, 0] },
          { type: "coincident", first: point("bottom", "end"), second: point("right", "start") },
          { type: "coincident", first: point("right", "end"), second: point("top", "start") },
          { type: "coincident", first: point("top", "end"), second: point("left", "start") },
          { type: "coincident", first: point("left", "end"), second: point("bottom", "start") },
          { type: "horizontal", entity: "bottom" },
          { type: "horizontal", entity: "top" },
          { type: "vertical", entity: "right" },
          { type: "vertical", entity: "left" },
          { type: "length", entity: "bottom", value: "width" },
          { type: "length", entity: "right", value: "height" },
          { type: "equalLength", first: "bottom", second: "top" },
          { type: "equalLength", first: "right", second: "left" },
        ],
      },
      { id: "face", type: "face", outer: "profile" },
      { id: "solid", type: "extrude", input: "face", vector: [0, 0, "depth"] },
    ],
  });
  await model.recompute();
  assert.equal(await client.shapeType(model.getShape("profile")), "wire");
  assert.equal(await client.shapeType(model.getShape("solid")), "solid");
  assert.ok(Math.abs((await client.massProps(model.getShape("solid"))).mass - 150) < 1e-6);

  model.setParameter("width", 12);
  await model.recompute();
  assert.ok(Math.abs((await client.massProps(model.getShape("solid"))).mass - 180) < 1e-6);
  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("arc and construction sketch entities form a downstream solid", async () => {
  const solved = solveSketch({
    entities: [
      { id: "arc", type: "arc", center: [0, 0], radius: 5, startAngle: 0, endAngle: Math.PI },
      { id: "chord", type: "line", start: [-5, 0], end: [5, 0] },
      { id: "construction", type: "line", start: [0, -2], end: [0, 2], construction: true },
    ],
    constraints: [
      { type: "fixed", point: point("arc", "center"), value: [0, 0] },
      { type: "radius", entity: "arc", value: 5 },
      { type: "coincident", first: point("arc", "end"), second: point("chord", "start") },
      { type: "coincident", first: point("arc", "start"), second: point("chord", "end") },
    ],
  });
  assert.equal(solved.diagnostics.converged, true);
  assert.equal(solved.entities.find((entity) => entity.id === "construction")?.construction, true);

  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: { depth: 2 },
    features: [
      {
        id: "profile",
        type: "sketch",
        entities: [
          { id: "arc", type: "arc", center: [0, 0], radius: 5, startAngle: 0, endAngle: Math.PI },
          { id: "chord", type: "line", start: [-5, 0], end: [5, 0] },
          { id: "construction", type: "line", start: [0, -2], end: [0, 2], construction: true },
        ],
        constraints: [
          { type: "fixed", point: point("arc", "center"), value: [0, 0] },
          { type: "radius", entity: "arc", value: 5 },
          { type: "coincident", first: point("arc", "end"), second: point("chord", "start") },
          { type: "coincident", first: point("arc", "start"), second: point("chord", "end") },
        ],
      },
      { id: "face", type: "face", outer: "profile" },
      { id: "solid", type: "extrude", input: "face", vector: [0, 0, "depth"] },
    ],
  });
  await model.recompute();
  assert.equal(await client.shapeType(model.getShape("solid")), "solid");
  assert.ok(Math.abs((await client.massProps(model.getShape("solid"))).mass - 25 * Math.PI) < 1e-5);
  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("sketch supports tangent, concentric, and symmetry constraints", () => {
  const solved = solveSketch({
    entities: [
      { id: "axis", type: "line", start: [0, -5], end: [0, 5] },
      { id: "tangentLine", type: "line", start: [-5, 3], end: [5, 3] },
      { id: "first", type: "circle", center: [0, 0], radius: 3 },
      { id: "second", type: "circle", center: [0, 0], radius: 5 },
      { id: "left", type: "line", start: [-2, 1], end: [-2, 2] },
      { id: "right", type: "line", start: [2, 1], end: [2, 2] },
    ],
    constraints: [
      { type: "tangent", first: "tangentLine", second: "first" },
      { type: "concentric", first: "first", second: "second" },
      { type: "symmetry", first: point("left", "start"), second: point("right", "start"), axis: "axis" },
    ],
  });
  assert.equal(solved.diagnostics.converged, true);
  assert.ok(solved.diagnostics.maxResidual < 1e-8);
  const first = solved.entities.find((entity) => entity.id === "first");
  const second = solved.entities.find((entity) => entity.id === "second");
  assert.deepEqual(first?.type, "circle");
  assert.deepEqual(second?.type, "circle");
  assert.ok(Math.abs(first.center[0] - second.center[0]) < 1e-8);
  assert.ok(Math.abs(first.center[1] - second.center[1]) < 1e-8);
});

test("sketch rejects invalid inputs and preserves positive radii", async () => {
  assert.throws(
    () => solveSketch({
      entities: [{ id: "line", type: "line", start: [0, 0], end: [1, 0] }],
      constraints: [{ type: "fixed", point: point("line", "banana"), value: [0, 0] }],
    }),
    /Unsupported sketch point reference "banana"/,
  );

  const positiveRadii = solveSketch({
    entities: [
      { id: "first", type: "circle", center: [0, 0], radius: 5 },
      { id: "second", type: "circle", center: [2, 0], radius: 1 },
    ],
    constraints: [
      { type: "fixed", point: point("first", "center"), value: [0, 0] },
      { type: "fixed", point: point("second", "center"), value: [2, 0] },
      { type: "radius", entity: "first", value: 5 },
      { type: "tangent", first: "first", second: "second" },
    ],
  });
  assert.equal(positiveRadii.diagnostics.converged, false);
  assert.ok(positiveRadii.entities.every((entity) => entity.type !== "circle" || entity.radius > 0));

  assert.throws(
    () => solveSketch({
      entities: [
        { id: "horizontal", type: "line", start: [0, 0], end: [1, 0] },
        { id: "vertical", type: "line", start: [0, 0], end: [0, 1] },
      ],
      constraints: [{ type: "banana", first: "horizontal", second: "vertical" }],
    }),
    /Unsupported sketch constraint type "banana"/,
  );

  assert.throws(
    () => solveSketch({
      entities: [
        { id: "curve", type: "spline", poles: [[0, 0], [1, 1], [2, 0]], degree: 2 },
        { id: "line", type: "line", start: [2, 0], end: [3, 0] },
      ],
      constraints: [{ type: "tangent", first: "curve", second: "line" }],
    }),
    /Tangent constraints support line, circle, and arc entities/,
  );

  const client = await DirectClient.create(wasm);
  assert.throws(
    () => ParametricModel.fromJSON(client, {
      schemaVersion: 1,
      parameters: {},
      features: [{
        id: "profile",
        type: "sketch",
        entities: [
          { id: "horizontal", type: "line", start: [0, 0], end: [1, 0] },
          { id: "vertical", type: "line", start: [0, 0], end: [0, 1] },
        ],
        constraints: [{ type: "banana", first: "horizontal", second: "vertical" }],
      }],
    }),
    /supported sketch constraint type/,
  );

  assert.throws(
    () => ParametricModel.fromJSON(client, {
      schemaVersion: 1,
      parameters: {},
      features: [{
        id: "profile",
        type: "sketch",
        entities: [{ id: "line", type: "line", start: [0, 0], end: [1, 0] }],
        constraints: [{ type: "fixed", point: point("line", "banana"), value: [0, 0] }],
      }],
    }),
    /point must be one of start, end, center/,
  );
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("spline sketch entities can close a profile for extrusion", async () => {
  const client = await DirectClient.create(wasm);
  const model = new ParametricModel(client, {
    parameters: { depth: 1 },
    features: [
      {
        id: "profile",
        type: "sketch",
        entities: [
          { id: "curve", type: "spline", poles: [[-5, 0], [0, 5], [5, 0]], degree: 2 },
          { id: "chord", type: "line", start: [5, 0], end: [-5, 0] },
          { id: "construction", type: "spline", poles: [[-4, 1], [0, 2], [4, 1]], construction: true },
        ],
        constraints: [
          { type: "coincident", first: point("curve", "end"), second: point("chord", "start") },
          { type: "coincident", first: point("curve", "start"), second: point("chord", "end") },
        ],
      },
      { id: "face", type: "face", outer: "profile" },
      { id: "solid", type: "extrude", input: "face", vector: [0, 0, "depth"] },
    ],
  });
  await model.recompute();
  assert.equal(await client.shapeType(model.getShape("solid")), "solid");
  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});

test("multi-profile sketch publishes wires for a face with holes", async () => {
  const client = await DirectClient.create(wasm);
  const definition = {
    parameters: { depth: 2 },
    features: [
      {
        id: "profiles",
        type: "sketch",
        entities: [
          { id: "o0", type: "line", start: [0, 0], end: [10, 0] },
          { id: "o1", type: "line", start: [10, 0], end: [10, 10] },
          { id: "o2", type: "line", start: [10, 10], end: [0, 10] },
          { id: "o3", type: "line", start: [0, 10], end: [0, 0] },
          { id: "h0", type: "line", start: [3, 3], end: [3, 7] },
          { id: "h1", type: "line", start: [3, 7], end: [7, 7] },
          { id: "h2", type: "line", start: [7, 7], end: [7, 3] },
          { id: "h3", type: "line", start: [7, 3], end: [3, 3] },
        ],
        profiles: [
          { id: "outer", entities: ["o0", "o1", "o2", "o3"] },
          { id: "hole", entities: ["h0", "h1", "h2", "h3"] },
        ],
      },
      { id: "face", type: "face", outer: "profiles.outer", holes: ["profiles.hole"] },
      { id: "solid", type: "extrude", input: "face", vector: [0, 0, "depth"] },
    ],
  };
  const model = new ParametricModel(client, definition);
  await model.recompute();
  assert.equal(await client.shapeType(model.getShape("profiles")), "compound");
  assert.equal(await client.shapeType(model.getShape("profiles.outer")), "wire");
  assert.equal(await client.shapeType(model.getShape("profiles.hole")), "wire");
  assert.ok(Math.abs((await client.massProps(model.getShape("solid"))).mass - 168) < 1e-6);

  const restored = ParametricModel.fromJSON(client, JSON.stringify(model.toJSON()));
  await restored.recompute();
  assert.ok(Math.abs((await client.massProps(restored.getShape("solid"))).mass - 168) < 1e-6);
  await restored.dispose();
  await model.dispose();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
