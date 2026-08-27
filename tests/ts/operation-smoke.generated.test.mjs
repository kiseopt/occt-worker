// Generated protocol-level smoke coverage: one executable fixture per frozen
// protocol operation (see scripts/verify-modules.mjs and protocol/modules.json).
//
// Every operation must appear exactly once below; adding an operation to
// protocol/operations.json without a smoke fixture fails this suite.
//
// Run: node --test tests/ts/operation-smoke.generated.test.mjs

import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";
import { OPERATIONS } from "../../dist/generated.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

const request = async (op, args) => client.request(op, args);

// ---------------------------------------------------------------------------
// Shared fixture context (built once in before hooks, consumed lazily)
// ---------------------------------------------------------------------------

let client;
let scope;
let readyPromise;
const ctx = {};

function ensureReady() {
  if (!readyPromise) {
    readyPromise = (async () => {
      client = await DirectClient.create(wasm);
      scope = await client.beginScope();
      await buildCoreFixtures();
      ctx.tempBufferId = (await request("createBuffer", { byteLength: 64 })).bufferId;
    })();
  }
  return readyPromise;
}

async function buildCoreFixtures() {
  ctx.box = await scope.makeBox([10, 10, 10]);
  ctx.cylinder = await scope.makeCylinder(2, 12, { origin: [3, 3, -1] });
  ctx.cutShape = (await scope.booleanCut(ctx.box, [ctx.cylinder])).shape;
  ctx.pipeBox = await scope.makeBox([10, 100, 10]);
  ctx.glueNew = await scope.makeBox([4, 4, 4], [3, 3, 10]);

  ctx.edgeLine = await scope.makeEdgeLine([0, 0, 0], [10, 0, 0]);
  ctx.edgeLineY = await scope.makeEdgeLine([0, 0, 0], [0, 10, 0]);
  ctx.edgeCircle = await scope.makeEdgeCircle(4);
  ctx.edgeBSpline = await scope.makeEdgeBSpline(
    [[0, 0, 0], [2, 3, 0], [5, 1, 1], [8, 3, 0], [10, 0, 0]],
    { mode: "controlPoints", degree: 3 },
  );

  ctx.squareWire = await scope.makePolygon([[2, 2, 0], [8, 2, 0], [8, 8, 0], [2, 8, 0]], true);
  ctx.planarFace = await scope.makeFace(ctx.squareWire);
  ctx.verticalFace = await scope.makeFace(
    await scope.makePolygon([[3, 5, -2], [7, 5, -2], [7, 5, 2], [3, 5, 2]], true),
  );
  ctx.topFace = await scope.getSubShape(ctx.box, "face", 5);
  ctx.piercingLine = await scope.makeEdgeLine([0, 5, 2], [10, 5, -2]);

  ctx.surfaceBSpline = await scope.makeSurfaceBSpline(
    [
      [[0, 0, 0], [4, 0, 1], [8, 0, 0]],
      [[0, 5, 1], [4, 5, 2], [8, 5, 1]],
      [[0, 10, 0], [4, 10, 1], [8, 10, 0]],
    ],
    { uDegree: 2, vDegree: 2 },
  );

  // Format payloads produced by the kernel itself (round-trip fixtures).
  ctx.brepData = (await request("exportBREP", { shape: ctx.box })).data.data;
  ctx.stepData = (await request("exportSTEP", { shape: ctx.box })).data.data;
  ctx.igesData = (await request("exportIGES", { shape: ctx.box })).data.data;
  const documentNodes = [{ kind: "part", shape: ctx.box, name: "smoke" }];
  ctx.xcafData = (await request("exportXCAF", { nodes: documentNodes, roots: [0] })).data.data;
  ctx.stepDocumentData = (
    await request("exportSTEPDocument", {
      nodes: [{ kind: "part", shape: ctx.box, name: "smoke-step-doc" }],
      roots: [0],
    })
  ).data.data;
  ctx.igesDocumentData = (
    await request("exportIGESDocument", {
      nodes: [{ kind: "part", shape: ctx.box, name: "smoke-iges-doc" }],
      roots: [0],
    })
  ).data.data;
  ctx.stlData = (await request("exportSTL", { shape: ctx.box })).data.data;
  ctx.objData = (await request("exportOBJ", { shape: ctx.box })).data.data;
  ctx.plyData = (await request("exportPLY", { shape: ctx.box })).data.data;
  ctx.gltfData = (await request("exportGLTF", { shape: ctx.box })).data.data;
  ctx.vrmlData = (await request("exportVRML", { shape: ctx.box })).data.data;

  // A planar BSpline surface whose degree is exactly reducible to 1.
  ctx.flatSurfaceBSpline = await scope.makeSurfaceBSpline(
    [
      [[0, 0, 0], [4, 0, 0], [8, 0, 0]],
      [[0, 5, 0], [4, 5, 0], [8, 5, 0]],
      [[0, 10, 0], [4, 10, 0], [8, 10, 0]],
    ],
    { uDegree: 2, vDegree: 2 },
  );

  // Triangulate the planar face once for triangulation-aware operations.
  await request("tessellate", { shape: ctx.planarFace, linearDeflection: 0.05, angularDeflection: 0.3 });
}

const midDomain = async (shapeHandle) => {
  const domain = await request("surfaceDomain", { shape: shapeHandle });
  return {
    u: domain.uFirst + (domain.uLast - domain.uFirst) / 2,
    v: domain.vFirst + (domain.vLast - domain.vFirst) / 2,
  };
};

// Wrap a raw ArrayBuffer for the protocol's bufferRef argument kind.
const bin = (buffer) => ({ $inputBuffer: buffer });

// ---------------------------------------------------------------------------
// Per-operation argument factories. Keys must cover OPERATIONS exhaustively.
// ---------------------------------------------------------------------------

const ARGS = {
  // runtime
  capabilities: () => ({}),
  beginScope: () => ({}),
  stats: () => ({}),
  batch: () => ({
    scopeId: scope.scopeId,
    ops: [
      { op: "makeVertex", args: { point: [1, 2, 3] } },
      { op: "makeBox", args: { size: [2, 2, 2] } },
    ],
  }),

  // modeling primitives & construction
  makeBox: () => ({ scopeId: scope.scopeId, size: [2, 3, 4] }),
  makeCylinder: () => ({ scopeId: scope.scopeId, radius: 2, height: 5 }),
  makeCone: () => ({ scopeId: scope.scopeId, radius1: 3, radius2: 1, height: 6 }),
  makeSphere: () => ({ scopeId: scope.scopeId, radius: 3 }),
  makeTorus: () => ({ scopeId: scope.scopeId, majorRadius: 5, minorRadius: 1 }),
  makeWedge: () => ({ scopeId: scope.scopeId, size: [4, 4, 4], ltx: 2 }),
  makeVertex: () => ({ scopeId: scope.scopeId, point: [1, 1, 1] }),
  makeCompound: () => ({ scopeId: scope.scopeId, shapes: [ctx.box, ctx.cylinder] }),
  makeHalfSpace: () => ({ scopeId: scope.scopeId, face: ctx.topFace, referencePoint: [5, 5, 20] }),
  extrude: () => ({ scopeId: scope.scopeId, shape: ctx.squareWire, vector: [0, 0, 3] }),
  revolve: async () => ({
    scopeId: scope.scopeId,
    shape: await scope.makePolygon([[2, -1, 0], [4, -1, 0], [4, 1, 0], [2, 1, 0]], true),
    origin: [0, 0, 0],
    direction: [0, 0, 1],
    angle: Math.PI * 2,
  }),
  loft: async () => ({
    scopeId: scope.scopeId,
    sections: [
      await scope.makePolygon([[-1, -1, 0], [1, -1, 0], [1, 1, 0], [-1, 1, 0]], true),
      await scope.makePolygon([[-1, -1, 6], [1, -1, 6], [1, 1, 6], [-1, 1, 6]], true),
    ],
    solid: true,
  }),
  sweepPipe: async () => ({
    scopeId: scope.scopeId,
    spine: await scope.makeWire([ctx.edgeLine]),
    profile: await scope.makeWire([await scope.makeEdgeCircle(1, { center: [0, 0, 0], normal: [0, 1, 0] })]),
  }),
  sweepPipeShell: async () => ({
    scopeId: scope.scopeId,
    spine: await scope.makeWire([ctx.edgeLine]),
    profiles: [await scope.makeWire([await scope.makeEdgeCircle(1, { center: [0, 0, 0], normal: [0, 1, 0] })])],
    mode: "frenet",
  }),
  fillet: () => ({ scopeId: scope.scopeId, shape: ctx.box, edgeIndices: [1], radius: 0.5 }),
  chamfer: () => ({ scopeId: scope.scopeId, shape: ctx.box, edgeIndices: [1], distance: 0.5 }),
  hollow: () => ({ scopeId: scope.scopeId, shape: ctx.box, thickness: 1 }),
  draftAngle: async () => {
    // Pick any vertical side face of the box.
    for (let index = 0; index < 6; ++index) {
      const face = await scope.getSubShape(ctx.box, "face", index);
      const bounds = await request("bbox", { shape: face });
      if (Math.abs(bounds.max[0] - bounds.min[0]) < 1e-5 || Math.abs(bounds.max[1] - bounds.min[1]) < 1e-5) {
        return {
          scopeId: scope.scopeId,
          shape: ctx.box,
          faceIndices: [index],
          direction: [0, 0, 1],
          angle: 5,
          neutralPlane: { origin: [0, 0, 0], normal: [0, 0, 1] },
        };
      }
    }
    throw new Error("no vertical face found for draftAngle fixture");
  },
  cylindricalHole: () => ({
    scopeId: scope.scopeId,
    shape: ctx.box,
    origin: [5, 5, 15],
    direction: [0, 0, -1],
    radius: 1.5,
    mode: "throughAll",
  }),
  defeature: async () => {
    const holed = (
      await request("cylindricalHole", {
        scopeId: scope.scopeId,
        shape: ctx.box,
        origin: [5, 5, 15],
        direction: [0, 0, -1],
        radius: 1.5,
        mode: "throughAll",
      })
    ).shape;
    const faceCount = (await request("topologyCounts", { shape: holed })).face;
    for (let index = 0; index < faceCount; ++index) {
      try {
        return { scopeId: scope.scopeId, shape: holed, faceIndices: [index] };
      } catch (error) {
        if (index === faceCount - 1) throw error;
      }
    }
  },
  transform: () => ({ scopeId: scope.scopeId, shape: ctx.box, translation: [1, 2, 3], copy: true }),
  translate: () => ({ scopeId: scope.scopeId, shape: ctx.box, translation: [2, 0, 0], copy: true }),
  rotate: () => ({
    scopeId: scope.scopeId,
    shape: ctx.box,
    rotation: { origin: [5, 5, 5], direction: [0, 0, 1], angle: Math.PI / 6 },
    copy: true,
  }),
  scale: () => ({ scopeId: scope.scopeId, shape: ctx.box, factor: 2, origin: [0, 0, 0], copy: true }),
  mirror: () => ({ scopeId: scope.scopeId, shape: ctx.box, normal: [0, 0, 1], copy: true }),
  generalTransform: () => ({
    scopeId: scope.scopeId,
    shape: ctx.box,
    matrix: [1, 0, 0, 2, 0, 1, 0, 0, 0, 0, 1, 0],
    copy: true,
  }),
  batchTransformCopy: () => ({ scopeId: scope.scopeId, shape: ctx.box, mode: "linear", count: 3, translation: [15, 0, 0] }),
  localPrism: () => ({
    scopeId: scope.scopeId,
    base: ctx.box,
    faceIndices: [5],
    direction: [0, 0, 1],
    length: 2,
    mode: "length",
    operation: "add",
  }),
  localRevolution: () => ({
    scopeId: scope.scopeId,
    base: ctx.box,
    faceIndices: [5],
    origin: [0, 0, 10],
    direction: [0, 1, 0],
    angle: Math.PI / 2,
  }),
  linearForm: async () => ({
    scopeId: scope.scopeId,
    base: ctx.box,
    profile: await scope.makeWire([
      await scope.makeEdgeLine([5, 5, 12], [5, 5, 5]),
      await scope.makeEdgeLine([5, 5, 5], [12, 5, 5]),
    ]),
    planeOrigin: [5, 5, 5],
    planeNormal: [0, -1, 0],
    direction: [0, 3, 0],
    direction1: [0, 0, 0],
    modify: false,
    operation: "add",
  }),
  revolutionForm: async () => ({
    scopeId: scope.scopeId,
    base: await scope.makeCylinder(5, 10),
    profile: await scope.makeWire([
      await scope.makeEdgeLine([5, 0, 4], [6, 0, 5]),
      await scope.makeEdgeLine([6, 0, 5], [5, 0, 6]),
    ]),
    planeOrigin: [0, 0, 0],
    planeNormal: [0, 1, 0],
    origin: [0, 0, 0],
    direction: [0, 0, 1],
    height1: 1,
    height2: 1,
    operation: "add",
  }),

  // algorithms
  booleanCommon: () => ({ scopeId: scope.scopeId, base: ctx.box, tools: [ctx.cylinder] }),
  booleanCut: () => ({ scopeId: scope.scopeId, base: ctx.box, tools: [ctx.cylinder] }),
  booleanFuse: () => ({ scopeId: scope.scopeId, base: ctx.box, tools: [ctx.cylinder] }),
  section: () => ({ scopeId: scope.scopeId, first: ctx.box, second: ctx.cylinder }),
  split: () => ({ scopeId: scope.scopeId, objects: [ctx.box], tools: [ctx.cylinder] }),
  generalFuse: () => ({ scopeId: scope.scopeId, shapes: [ctx.box, ctx.cylinder] }),
  selectGeneralFuseCells: () => ({ scopeId: scope.scopeId, shapes: [ctx.box, ctx.cylinder], rules: [{ take: [0] }] }),
  distance: () => ({ first: ctx.box, second: ctx.cylinder }),
  extremaCurveCurve: () => ({ first: ctx.edgeLine, second: ctx.edgeLineY }),
  extremaCurveSurface: () => ({ curve: ctx.edgeLine, surface: ctx.planarFace }),
  extremaSurfaceSurface: () => ({ first: ctx.planarFace, second: ctx.topFace }),
  intersectCurveCurve: () => ({ scopeId: scope.scopeId, first: ctx.edgeLine, second: ctx.edgeLineY }),
  intersectCurveSurface: () => ({ curve: ctx.piercingLine, surface: ctx.planarFace }),
  intersectSurfaceSurface: () => ({ scopeId: scope.scopeId, first: ctx.planarFace, second: ctx.verticalFace }),
  projectPointCurve: () => ({ shape: ctx.edgeLine, point: [5, 1, 0] }),
  projectPointSurface: () => ({ shape: ctx.planarFace, point: [5, 5, 3] }),
  projectHLR: () => ({ scopeId: scope.scopeId, shape: ctx.box, direction: [0, -1, 0], up: [0, 0, 1] }),
  classifyPoint: () => ({ shape: ctx.box, point: [5, 5, 5] }),
  sectionAnalysis: () => ({ scopeId: scope.scopeId, first: ctx.planarFace, second: ctx.verticalFace }),
  massProps: () => ({ shape: ctx.box, kind: "volume" }),
  middlePath: async () => {
    const faces = await Promise.all(
      Array.from({ length: 6 }, (_, index) => scope.getSubShape(ctx.pipeBox, "face", index)),
    );
    const bounds = await Promise.all(faces.map((face) => request("bbox", { shape: face })));
    const startIndex = bounds.findIndex(({ min, max }) => Math.abs(min[1]) < 1e-5 && Math.abs(max[1]) < 1e-5);
    const endIndex = bounds.findIndex(({ min, max }) => Math.abs(min[1] - 100) < 1e-5 && Math.abs(max[1] - 100) < 1e-5);
    return { scopeId: scope.scopeId, shape: ctx.pipeBox, start: faces[startIndex], end: faces[endIndex] };
  },
  offsetShape: () => ({ scopeId: scope.scopeId, shape: ctx.box, offset: 1 }),
  offsetWire2D: () => ({ scopeId: scope.scopeId, wire: ctx.squareWire, offset: 0.5 }),
  fixShape: () => ({ scopeId: scope.scopeId, shape: ctx.box }),
  unifySameDomain: () => ({ scopeId: scope.scopeId, shape: ctx.box }),
  shapeUpgrade: () => ({ scopeId: scope.scopeId, shape: ctx.box, mode: "continuity", boundaryCriterion: "c1" }),
  glue: () => ({ scopeId: scope.scopeId, newShape: ctx.glueNew, baseShape: ctx.box, faceBindings: [{ newIndex: 4, baseIndex: 5 }] }),
  sew: async () => ({
    scopeId: scope.scopeId,
    shapes: [
      ctx.planarFace,
      await scope.makeFace(await scope.makePolygon([[2, 2, 0], [8, 2, 0], [8, 8, 0], [2, 8, 0]], true)),
    ],
    tolerance: 1e-6,
  }),

  // geometry-topology queries on existing shapes
  bbox: () => ({ shape: ctx.box }),
  obb: () => ({ shape: ctx.box }),
  shapeType: () => ({ shape: ctx.box }),
  topologyCounts: () => ({ shape: ctx.box }),
  isValid: () => ({ shape: ctx.box }),
  isSameShape: () => ({ first: ctx.box, second: ctx.box }),
  getAdjacency: () => ({ shape: ctx.topFace, from: "face", to: "edge" }),
  getSubShape: () => ({ scopeId: scope.scopeId, shape: ctx.box, type: "face", index: 1 }),
  getSubShapes: () => ({ shape: ctx.box, type: "face" }),
  diagnoseShape: () => ({ shape: ctx.box }),
  inspectTolerances: () => ({ shape: ctx.box }),
  setTolerance: () => ({ scopeId: scope.scopeId, shape: ctx.box, tolerance: 1e-5, type: "all" }),

  // curves & surfaces
  approximateCurveBSpline: () => ({
    scopeId: scope.scopeId,
    points: Array.from({ length: 9 }, (_, i) => [i, i * i * 0.25, 0]),
  }),
  approximateSurfaceBSpline: () => ({
    scopeId: scope.scopeId,
    points: [
      [[0, 0, 0], [2, 0, 0.5], [4, 0, 0]],
      [[0, 2, 0.5], [2, 2, 1], [4, 2, 0.5]],
      [[0, 4, 0], [2, 4, 0.5], [4, 4, 0]],
    ],
  }),
  convertCurveToBSpline: () => ({ scopeId: scope.scopeId, shape: ctx.edgeCircle }),
  convertSurfaceToBSpline: () => ({ scopeId: scope.scopeId, shape: ctx.planarFace }),
  curveContinuity: () => ({ shape: ctx.edgeBSpline }),
  curveControlData: () => ({ shape: ctx.edgeBSpline }),
  curveDomain: () => ({ shape: ctx.edgeCircle }),
  curveGeometry: () => ({ shape: ctx.edgeCircle }),
  evaluateCurve: async () => {
    const domain = await request("curveDomain", { shape: ctx.edgeCircle });
    return { shape: ctx.edgeCircle, parameter: domain.first + (domain.last - domain.first) / 2 };
  },
  editCurveBSpline: async () => ({
    scopeId: scope.scopeId,
    shape: await scope.makeEdgeBSpline(
      [[0, 0, 0], [2, 3, 0], [5, 1, 1], [8, 3, 0], [10, 0, 0]],
      { mode: "controlPoints", degree: 3 },
    ),
    action: "insertKnot",
    knot: 0.5,
    multiplicity: 1,
  }),
  extendCurve: () => ({ scopeId: scope.scopeId, shape: ctx.edgeBSpline, point: [12, 1, 0] }),
  reparameterizeCurve: () => ({ scopeId: scope.scopeId, shape: ctx.edgeBSpline, first: 0, last: 5 }),
  reduceCurveDegree: () => ({ scopeId: scope.scopeId, shape: ctx.edgeBSpline, degree: 2, tolerance: 1e-3 }),
  trimCurve: async () => {
    const domain = await request("curveDomain", { shape: ctx.edgeCircle });
    return {
      scopeId: scope.scopeId,
      shape: ctx.edgeCircle,
      first: domain.first,
      last: domain.first + (domain.last - domain.first) / 2,
    };
  },
  updateCurvePole: async () => ({
    scopeId: scope.scopeId,
    shape: await scope.makeEdgeBSpline(
      [[0, 0, 0], [2, 3, 0], [5, 1, 1], [8, 3, 0], [10, 0, 0]],
      { mode: "controlPoints", degree: 3 },
    ),
    index: 2,
    point: [5, 1, 2],
  }),
  makeEdgeArc: () => ({ scopeId: scope.scopeId, start: [0, 0, 0], point: [5, 3, 0], end: [10, 0, 0] }),
  makeEdgeBezier: () => ({ scopeId: scope.scopeId, poles: [[0, 0, 0], [2, 3, 0], [5, 1, 1], [8, 3, 0], [10, 0, 0]] }),
  makeEdgeBSpline: () => ({
    scopeId: scope.scopeId,
    poles: [[0, 0, 0], [2, 3, 0], [5, 1, 1], [8, 3, 0], [10, 0, 0]],
    mode: "controlPoints",
    degree: 3,
  }),
  makeEdgeCircle: () => ({ scopeId: scope.scopeId, radius: 3 }),
  makeEdgeEllipse: () => ({ scopeId: scope.scopeId, majorRadius: 5, minorRadius: 3 }),
  makeEdgeHelix: () => ({ scopeId: scope.scopeId, radius: 3, pitch: 1, turns: 2 }),
  makeEdgeHyperbola: () => ({ scopeId: scope.scopeId, majorRadius: 4, minorRadius: 2, firstParameter: -1, lastParameter: 1 }),
  makeEdgeLine: () => ({ scopeId: scope.scopeId, start: [0, 0, 0], end: [5, 5, 0] }),
  makeEdgeOffset: () => ({ scopeId: scope.scopeId, shape: ctx.edgeLine, offset: 1, direction: [0, 0, 1] }),
  makeEdgeParabola: () => ({ scopeId: scope.scopeId, focal: 2, firstParameter: -2, lastParameter: 2 }),
  evaluateSurface: async () => ({ shape: ctx.planarFace, ...(await midDomain(ctx.planarFace)) }),
  surfaceContinuity: () => ({ shape: ctx.surfaceBSpline }),
  surfaceControlData: () => ({ shape: ctx.surfaceBSpline }),
  surfaceDomain: () => ({ shape: ctx.planarFace }),
  surfaceGeometry: () => ({ shape: ctx.planarFace }),
  surfaceIsoCurve: async () => ({
    scopeId: scope.scopeId,
    surface: ctx.planarFace,
    direction: "u",
    parameter: (await midDomain(ctx.planarFace)).u,
  }),
  editSurfaceBSpline: () => ({
    scopeId: scope.scopeId,
    shape: ctx.surfaceBSpline,
    action: "increaseDegree",
    uDegree: 3,
    vDegree: 3,
  }),
  extendSurface: () => ({ scopeId: scope.scopeId, shape: ctx.surfaceBSpline, length: 1, direction: "u", side: "after" }),
  reparameterizeSurface: () => ({ scopeId: scope.scopeId, shape: ctx.surfaceBSpline, uFirst: 0, uLast: 16, vFirst: 0, vLast: 20 }),
  reduceSurfaceDegree: () => ({
    scopeId: scope.scopeId,
    shape: ctx.flatSurfaceBSpline,
    uDegree: 1,
    vDegree: 1,
    uContinuity: "c0",
    vContinuity: "c0",
    tolerance: 1e-3,
  }),
  trimSurface: async () => {
    const domain = await request("surfaceDomain", { shape: ctx.surfaceBSpline });
    return {
      scopeId: scope.scopeId,
      shape: ctx.surfaceBSpline,
      uFirst: domain.uFirst,
      uLast: domain.uLast,
      vFirst: domain.vFirst,
      vLast: domain.vFirst + (domain.vLast - domain.vFirst) / 2,
    };
  },
  updateSurfacePole: () => ({
    scopeId: scope.scopeId,
    shape: ctx.surfaceBSpline,
    uIndex: 1,
    vIndex: 1,
    point: [4, 5, 4],
  }),
  makeSurfaceBSpline: () => ({
    scopeId: scope.scopeId,
    poles: [
      [[0, 0, 0], [4, 0, 1], [8, 0, 0]],
      [[0, 5, 1], [4, 5, 2], [8, 5, 1]],
      [[0, 10, 0], [4, 10, 1], [8, 10, 0]],
    ],
    uDegree: 2,
    vDegree: 2,
  }),
  makeSurfaceBezier: () => ({
    scopeId: scope.scopeId,
    poles: [
      [[0, 0, 0], [4, 0, 2], [8, 0, 0]],
      [[0, 6, 2], [4, 6, 4], [8, 6, 2]],
      [[0, 12, 0], [4, 12, 2], [8, 12, 0]],
    ],
  }),
  makeSurfaceExtrusion: () => ({ scopeId: scope.scopeId, profile: ctx.edgeLine, direction: [0, 0, 1], vFirst: 0, vLast: 4 }),
  makeSurfaceFace: () => ({
    scopeId: scope.scopeId,
    surface: { type: "plane", origin: [0, 0, 40], direction: [0, 0, 1] },
    uFirst: -5,
    uLast: 5,
    vFirst: -5,
    vLast: 5,
  }),
  makeSurfaceFilling: () => ({ scopeId: scope.scopeId, boundary: ctx.squareWire }),
  makeSurfaceOffset: () => ({ scopeId: scope.scopeId, shape: ctx.planarFace, offset: 2 }),
  makeSurfaceRevolution: async () => ({
    scopeId: scope.scopeId,
    profile: await scope.makeEdgeLine([3, 0, 0], [3, 0, 4]),
    origin: [0, 0, 0],
    direction: [0, 0, 1],
    angle: Math.PI * 2,
  }),
  makeSurfaceRuled: async () => ({
    scopeId: scope.scopeId,
    first: await scope.makeEdgeLine([0, 0, 0], [6, 0, 0]),
    second: await scope.makeEdgeLine([0, 4, 3], [6, 4, 3]),
  }),
  makePolygon: () => ({ scopeId: scope.scopeId, points: [[0, 0, 50], [4, 0, 50], [4, 4, 50]], close: true }),
  makeWire: async () => ({
    scopeId: scope.scopeId,
    edges: [
      await scope.makeEdgeLine([0, 0, 60], [4, 0, 60]),
      await scope.makeEdgeLine([4, 0, 60], [4, 4, 60]),
    ],
  }),
  makeFace: async () => ({
    scopeId: scope.scopeId,
    outer: await scope.makePolygon([[20, 20, 0], [26, 20, 0], [26, 26, 0], [20, 26, 0]], true),
  }),
  makeFaceOnSurface: async () => {
    const surface = await request("makeSurfaceFace", {
      scopeId: scope.scopeId,
      surface: { type: "plane", origin: [0, 0, 70], direction: [0, 0, 1] },
      uFirst: -10,
      uLast: 10,
      vFirst: -10,
      vLast: 10,
    });
    return {
      scopeId: scope.scopeId,
      surface: surface.shape,
      outer: await scope.makePolygon([[1, 1, 70], [5, 1, 70], [5, 5, 70], [1, 5, 70]], true),
    };
  },
  makeShell: async () => ({
    scopeId: scope.scopeId,
    faces: await Promise.all(Array.from({ length: 6 }, (_, index) => scope.getSubShape(ctx.box, "face", index))),
  }),
  makeSolidFromShell: async () => {
    const shell = await request("makeShell", {
      scopeId: scope.scopeId,
      faces: await Promise.all(Array.from({ length: 6 }, (_, index) => scope.getSubShape(ctx.box, "face", index))),
    });
    return { scopeId: scope.scopeId, shell: shell.shape };
  },
  makeCompSolid: async () => ({
    scopeId: scope.scopeId,
    solids: [
      await scope.getSubShape(ctx.box, "solid", 0),
      await scope.getSubShape(await scope.makeBox([10, 10, 10]), "solid", 0),
    ],
  }),

  // mesh
  tessellate: () => ({ shape: ctx.box, linearDeflection: 0.1, angularDeflection: 0.5 }),
  tessellateEdges: () => ({ shape: ctx.box, linearDeflection: 0.1, angularDeflection: 0.5 }),
  triangulationData: () => ({ shape: ctx.planarFace }),
  validateTriangulation: () => ({ shape: ctx.planarFace }),
  replaceTriangulation: async () => ({
    scopeId: scope.scopeId,
    shape: await scope.makeFace(await scope.makePolygon([[40, 40, 0], [46, 40, 0], [46, 46, 0], [40, 46, 0]], true)),
    positions: [[40, 40, 0], [46, 40, 0], [46, 46, 0], [40, 46, 0]],
    indices: [0, 1, 2, 0, 2, 3],
    normals: [[0, 0, 1], [0, 0, 1], [0, 0, 1], [0, 0, 1]],
  }),
  repairTriangulation: () => ({ scopeId: scope.scopeId, shape: ctx.planarFace, linearDeflection: 0.05, angularDeflection: 0.3 }),

  // exchange-mesh round trips (kernel-produced payloads)
  exportSTL: () => ({ shape: ctx.box }),
  importSTL: () => ({ scopeId: scope.scopeId, data: bin(ctx.stlData) }),
  exportOBJ: () => ({ shape: ctx.box }),
  importOBJ: () => ({ scopeId: scope.scopeId, data: bin(ctx.objData) }),
  exportPLY: () => ({ shape: ctx.box }),
  importPLY: () => ({ scopeId: scope.scopeId, data: bin(ctx.plyData) }),
  exportGLTF: () => ({ shape: ctx.box }),
  importGLTF: () => ({ scopeId: scope.scopeId, data: bin(ctx.gltfData) }),
  exportVRML: () => ({ shape: ctx.box }),
  importVRML: () => ({ scopeId: scope.scopeId, data: bin(ctx.vrmlData) }),

  // exchange-cad round trips
  exportBREP: () => ({ shape: ctx.box }),
  importBREP: () => ({ scopeId: scope.scopeId, data: bin(ctx.brepData) }),
  exportSTEP: () => ({ shape: ctx.box }),
  importSTEP: () => ({ scopeId: scope.scopeId, data: bin(ctx.stepData) }),
  exportIGES: () => ({ shape: ctx.box }),
  importIGES: () => ({ scopeId: scope.scopeId, data: bin(ctx.igesData) }),
  exportXCAF: () => ({ nodes: [{ kind: "part", shape: ctx.box, name: "smoke-xcaf" }], roots: [0] }),
  importXCAF: () => ({ scopeId: scope.scopeId, data: bin(ctx.xcafData) }),
  exportSTEPDocument: () => ({ nodes: [{ kind: "part", shape: ctx.box, name: "smoke-step-doc2" }], roots: [0] }),
  importSTEPDocument: () => ({ scopeId: scope.scopeId, data: bin(ctx.stepDocumentData) }),
  exportIGESDocument: () => ({ nodes: [{ kind: "part", shape: ctx.box, name: "smoke-iges-doc2" }], roots: [0] }),
  importIGESDocument: () => ({ scopeId: scope.scopeId, data: bin(ctx.igesDocumentData) }),
  probeFormat: () => ({ data: bin(ctx.brepData) }),

  // lifecycle terminators (executed last). createBuffer/freeBuffer exercise
  // the explicit-buffer wrapper because the protocol materializes buffer
  // descriptors into byte payloads, hiding the raw bufferId by design.
  createBuffer: async () => {
    ctx.directBuffer = await client.createBuffer(32);
    ctx.directBuffer.view().fill(7);
    return { skipped: true };
  },
  freeBuffer: async () => {
    await client.freeBuffer(ctx.directBuffer);
    return { skipped: true };
  },
  release: async () => ({ shape: await scope.makeVertex([99, 99, 99]) }),
  endScope: async () => {
    const disposable = await client.beginScope();
    await request("makeVertex", { scopeId: disposable.scopeId, point: [0, 0, 98] });
    return { scopeId: disposable.scopeId };
  },
  releaseAll: () => ({}),
};

const TAIL_ORDER = ["createBuffer", "freeBuffer", "release", "endScope", "releaseAll"];
const HEAD_ORDER = ["capabilities", "beginScope"];

const ORDER = (() => {
  const entries = Object.keys(ARGS);
  const tail = new Set(TAIL_ORDER);
  const head = new Set(HEAD_ORDER);
  const ordered = [...head].filter((op) => entries.includes(op));
  ordered.push(...entries.filter((op) => !head.has(op) && !tail.has(op)));
  ordered.push(...[...tail]);
  return ordered;
})();

test("operation smoke catalog matches the frozen protocol", async () => {
  await ensureReady();
  assert.equal(OPERATIONS.length, 161);
  const missing = OPERATIONS.filter((op) => !(op in ARGS));
  assert.deepEqual(missing, [], "operations without a smoke fixture must be implemented");
  const unknown = Object.keys(ARGS).filter((op) => !OPERATIONS.includes(op));
  assert.deepEqual(unknown, [], "smoke fixtures must not reference unknown operations");
});

for (const op of ORDER) {
  test(`smoke ${op}`, async () => {
    await ensureReady();
    const factory = ARGS[op];
    assert.equal(typeof factory, "function");
    const args = await factory();
    if (args.skipped) return;
    const result = await request(op, args);
    assert.ok(result !== undefined && result !== null, `${op} returned no result`);

    // Structural checks for representative result families.
    if (op === "bbox") {
      for (const key of ["min", "max"]) {
        const vector = result[key];
        assert.ok(Array.isArray(vector) && vector.every(Number.isFinite), `bbox.${key} must be finite vec3`);
      }
    }
    if (op === "tessellate") {
      assert.ok((result.positions.data ?? result.positions).byteLength > 0);
      assert.ok((result.indices.data ?? result.indices).byteLength > 0);
    }
    if (op === "massProps") {
      assert.ok(Math.abs(result.mass - 1000) < 1e-6);
      assert.ok(result.center.every(Number.isFinite));
    }
    if (op === "isSameShape") assert.equal(result.same, true);
    if (op === "isValid") assert.equal(result.valid, true);
    if (op === "batch") {
      assert.ok(Array.isArray(result.results) && result.results.length === 2);
      assert.ok(result.results.every((entry) => entry.ok !== false));
    }
    if (op === "capabilities") {
      assert.equal(result.protocolVersion, "1.2.0");
      assert.ok(Array.isArray(result.ops) && result.ops.length === 161);
    }
    if (op === "beginScope") {
      assert.ok(typeof result.scopeId === "number");
      await request("endScope", { scopeId: result.scopeId });
    }
    if (op.startsWith("import") && op.endsWith("Document")) {
      assert.ok(typeof result.rootCount === "number" && result.rootCount >= 1);
      assert.ok(Array.isArray(result.nodes) && result.nodes.length >= 1);
      assert.ok(Array.isArray(result.roots) && result.roots.length >= 1);
    }
    if (op === "sew") {
      assert.ok(typeof result.freeEdges === "number");
      assert.ok(typeof result.multipleEdges === "number");
    }
    if (op === "stats") {
      assert.ok(typeof result.liveShapeHandles === "number");
      assert.ok(typeof result.liveBufferBytes === "number");
    }
    if (op === "releaseAll") {
      const stats = await request("stats", {});
      assert.equal(stats.liveShapeHandles, 0);
      assert.equal(stats.liveBufferBytes, 0);
    }
  });
}
