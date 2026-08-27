import type { Vec3 } from "./types-base.js";

export interface Tessellation {
  positions: Float32Array;
  normals: Float32Array;
  indices: Uint32Array;
  faceGroups: Uint32Array;
  uvs?: Float32Array;
}

export interface TriangulationReplacement {
  positions: readonly Vec3[];
  indices: readonly number[];
  normals?: readonly Vec3[];
  uvs?: readonly number[];
}

export type TriangulationIssueCode =
  | "missingTriangulation"
  | "emptyTriangulation"
  | "missingNormals"
  | "missingUV"
  | "nonFinitePosition"
  | "nonFiniteNormal"
  | "nonFiniteUV"
  | "invalidTriangle"
  | "degenerateTriangle";

export interface TriangulationIssue {
  faceIndex: number;
  code: TriangulationIssueCode;
  nodeIndex?: number;
  triangleIndex?: number;
}

export interface TriangulationValidation {
  valid: boolean;
  faces: number;
  triangulatedFaces: number;
  triangles: number;
  issues: readonly TriangulationIssue[];
}

export interface TriangulationRepairResult {
  shape: import("./client.js").ShapeHandle;
  faces: number;
  triangulatedFaces: number;
  triangles: number;
}

export interface EdgeTessellation {
  positions: Float32Array;
  edgeGroups: Uint32Array;
}

export interface MassProperties {
  mass: number;
  center: Vec3;
  inertia: readonly [number, number, number, number, number, number, number, number, number];
}

export type DistanceSupport =
  | { type: "vertex"; index?: number }
  | { type: "edge"; index?: number; parameter: number }
  | { type: "face"; index?: number; u: number; v: number };

export interface DistanceSolution {
  distance: number;
  pointOnFirst: Vec3;
  pointOnSecond: Vec3;
  supportOnFirst: DistanceSupport;
  supportOnSecond: DistanceSupport;
}

export interface DistanceResult {
  distance: number;
  pointOnFirst: Vec3;
  pointOnSecond: Vec3;
  innerSolution: boolean;
  solutions: DistanceSolution[];
}

export interface SectionEdgeAnalysis {
  index: number;
  length: number;
  firstFaceIndex?: number;
  secondFaceIndex?: number;
}

export interface SectionAnalysis {
  shape: import("./client.js").ShapeHandle;
  edges: SectionEdgeAnalysis[];
  standaloneVertices: number[];
}

export type HLRProjectionOptions =
  | { direction: Vec3; up: Vec3; projection?: "parallel" }
  | { direction: Vec3; up: Vec3; projection: "perspective"; focus: number };

export interface HLRProjection {
  visible: import("./client.js").ShapeHandle;
  hidden: import("./client.js").ShapeHandle;
}

export interface MultiRootShapeImport {
  shape: import("./client.js").ShapeHandle;
  shapes: import("./client.js").ShapeHandle[];
  rootCount: number;
}

export interface CurveDomain {
  first: number;
  last: number;
  periodic: boolean;
  period: number;
}

export type Continuity = "c0" | "g1" | "c1" | "g2" | "c2" | "c3" | "cn";

interface CurveAxisGeometry {
  center: Vec3;
  normal: Vec3;
  xDirection: Vec3;
}

export type CurveGeometry =
  | { type: "line"; origin: Vec3; direction: Vec3 }
  | (CurveAxisGeometry & { type: "circle"; radius: number })
  | (CurveAxisGeometry & { type: "ellipse" | "hyperbola"; majorRadius: number; minorRadius: number })
  | (CurveAxisGeometry & { type: "parabola"; focal: number })
  | { type: "bezier"; degree: number; poleCount: number; rational: boolean }
  | { type: "bspline"; degree: number; poleCount: number; knotCount: number; rational: boolean }
  | { type: "offset"; offset: number; direction: Vec3 }
  | { type: "other" };

export interface CurveControlData {
  type: "bezier" | "bspline";
  degree: number;
  poles: Vec3[];
  weights: number[];
  knots: number[];
  multiplicities: number[];
  periodic: boolean;
}

export type BSplineCurveEdit =
  | { action: "insertKnot"; knot: number; multiplicity?: number; tolerance?: number; add?: boolean }
  | { action: "insertKnots"; knots: readonly number[]; multiplicities: readonly number[]; tolerance?: number; add?: boolean }
  | { action: "removeKnot"; knotIndex: number; multiplicity: number; tolerance?: number }
  | { action: "increaseDegree"; degree: number }
  | { action: "setPeriodic" }
  | { action: "setNotPeriodic" }
  | { action: "setControlNet"; poles: readonly Vec3[]; weights?: readonly number[] };

export interface CurveEvaluation {
  parameter: number;
  point: Vec3;
  d1: Vec3;
  d2: Vec3;
  tangentDefined: boolean;
  tangent?: Vec3;
  curvatureDefined: boolean;
  curvature?: number;
}

export interface CurveProjectionSolution {
  parameter: number;
  point: Vec3;
  distance: number;
}

export interface CurveProjection {
  solutions: CurveProjectionSolution[];
}

export interface SurfaceProjectionSolution {
  u: number;
  v: number;
  point: Vec3;
  distance: number;
}

export interface SurfaceProjection {
  solutions: SurfaceProjectionSolution[];
}

export interface CurveCurveExtremum {
  parameter1: number;
  parameter2: number;
  point1: Vec3;
  point2: Vec3;
  distance: number;
}

export interface CurveCurveExtrema {
  solutions: CurveCurveExtremum[];
  parallel?: boolean;
}

export interface CurveSurfaceExtremum {
  curveParameter: number;
  u: number;
  v: number;
  pointOnCurve: Vec3;
  pointOnSurface: Vec3;
  distance: number;
}

export interface CurveSurfaceExtrema {
  solutions: CurveSurfaceExtremum[];
  parallel?: boolean;
}

export interface SurfaceSurfaceExtremum {
  u1: number;
  v1: number;
  u2: number;
  v2: number;
  point1: Vec3;
  point2: Vec3;
  distance: number;
}

export interface SurfaceSurfaceExtrema {
  solutions: SurfaceSurfaceExtremum[];
  parallel?: boolean;
}

export interface CurveSurfaceIntersectionPoint {
  point: Vec3;
  curveParameter: number;
  u: number;
  v: number;
}

export interface CurveSurfaceIntersectionSegment {
  u1: number;
  v1: number;
  u2: number;
  v2: number;
}

export interface CurveSurfaceIntersection {
  points: CurveSurfaceIntersectionPoint[];
  segments: CurveSurfaceIntersectionSegment[];
}

export interface SurfaceDomain {
  uFirst: number;
  uLast: number;
  vFirst: number;
  vLast: number;
  uPeriodic: boolean;
  uPeriod: number;
  vPeriodic: boolean;
  vPeriod: number;
}

export interface SurfaceBounds {
  uFirst: number;
  uLast: number;
  vFirst: number;
  vLast: number;
}

export interface SurfaceControlData {
  type: "bezier" | "bspline";
  uDegree: number;
  vDegree: number;
  poles: Vec3[][];
  weights: number[][];
  uKnots: number[];
  vKnots: number[];
  uMultiplicities: number[];
  vMultiplicities: number[];
  uPeriodic: boolean;
  vPeriodic: boolean;
}

export type BSplineSurfaceEdit =
  | { action: "insertUKnot"; uKnot: number; uMultiplicity: number; tolerance?: number; add?: boolean }
  | { action: "insertVKnot"; vKnot: number; vMultiplicity: number; tolerance?: number; add?: boolean }
  | { action: "insertUKnots"; uKnots: readonly number[]; uMultiplicities: readonly number[]; tolerance?: number; add?: boolean }
  | { action: "insertVKnots"; vKnots: readonly number[]; vMultiplicities: readonly number[]; tolerance?: number; add?: boolean }
  | { action: "removeUKnot"; uKnotIndex: number; uMultiplicity: number; tolerance?: number }
  | { action: "removeVKnot"; vKnotIndex: number; vMultiplicity: number; tolerance?: number }
  | { action: "increaseDegree"; uDegree: number; vDegree: number }
  | { action: "setUPeriodic" }
  | { action: "setVPeriodic" }
  | { action: "setUNotPeriodic" }
  | { action: "setVNotPeriodic" }
  | { action: "exchangeUV" }
  | { action: "reverseU" }
  | { action: "reverseV" }
  | { action: "setControlNet"; poles: readonly (readonly Vec3[])[]; weights?: readonly (readonly number[])[] };

export interface BSplineSurfaceOptions {
  weights?: readonly (readonly number[])[];
  uDegree?: number;
  vDegree?: number;
  uKnots?: readonly number[];
  vKnots?: readonly number[];
  uMultiplicities?: readonly number[];
  vMultiplicities?: readonly number[];
  uPeriodic?: boolean;
  vPeriodic?: boolean;
  tolerance?: number;
}

export interface SurfaceFillingOptions {
  points?: readonly Vec3[];
  constraints?: readonly SurfaceFillingConstraint[];
  degree?: number;
  pointsOnCurve?: number;
  iterations?: number;
  anisotropic?: boolean;
  tolerance2d?: number;
  tolerance3d?: number;
  angularTolerance?: number;
  curvatureTolerance?: number;
  maxDegree?: number;
  maxSegments?: number;
}

export interface SurfaceFillingConstraint {
  edgeIndex: number;
  support: import("./client.js").ShapeHandle;
  continuity: "g1" | "g2";
}

export type LocalPrismOptions =
  | { mode?: "length"; length: number; operation?: "add" | "cut" }
  | { mode: "until"; until: import("./client.js").ShapeHandle; operation?: "add" | "cut" }
  | { mode: "fromUntil"; from: import("./client.js").ShapeHandle; until: import("./client.js").ShapeHandle; operation?: "add" | "cut" }
  | { mode: "untilEnd" | "thruAll"; operation?: "add" | "cut" }
  | { mode: "fromEnd"; from: import("./client.js").ShapeHandle; operation?: "add" | "cut" };

export type LocalRevolutionOptions =
  | { mode?: "angle"; angle: number; operation?: "add" | "cut" }
  | { mode: "until"; until: import("./client.js").ShapeHandle; operation?: "add" | "cut" }
  | { mode: "fromUntil"; from: import("./client.js").ShapeHandle; until: import("./client.js").ShapeHandle; operation?: "add" | "cut" }
  | { mode: "thruAll"; operation?: "add" | "cut" };

export interface LinearFormOptions {
  planeOrigin: Vec3;
  planeNormal: Vec3;
  direction: Vec3;
  direction1?: Vec3;
  operation?: "add" | "cut";
  modify?: boolean;
}

export interface RevolutionFormOptions {
  planeOrigin: Vec3;
  planeNormal: Vec3;
  origin: Vec3;
  direction: Vec3;
  height1: number;
  height2: number;
  operation?: "add" | "cut";
  modify?: boolean;
}

export type ApproximationParameterization = "chordLength" | "centripetal" | "uniform";
export type ApproximationContinuity = "c0" | "c1" | "c2" | "c3";
export type ExtensionContinuity = "c1" | "c2" | "c3";

export interface BSplineApproximationOptions {
  parameterization?: ApproximationParameterization;
  degreeMin?: number;
  degreeMax?: number;
  continuity?: ApproximationContinuity;
  tolerance?: number;
  variationalSmoothing?: {
    length: number;
    curvature: number;
    torsion: number;
  };
}

export interface CurveDegreeReductionOptions {
  tolerance?: number;
  continuity?: ApproximationContinuity;
  maxSegments?: number;
}

export interface SurfaceDegreeReductionOptions {
  tolerance?: number;
  uContinuity?: ApproximationContinuity;
  vContinuity?: ApproximationContinuity;
  maxSegments?: number;
}

interface ShapeUpgradeDivideOptions {
  precision?: number;
  minTolerance?: number;
  maxTolerance?: number;
  surfaceSegmentMode?: boolean;
  edgeMode?: 0 | 1 | 2;
}

export type ShapeUpgradeOptions =
  | (ShapeUpgradeDivideOptions & {
      mode?: "continuity";
      boundaryCriterion?: "c0" | "c1" | "c2" | "c3" | "cn";
      pcurveCriterion?: "c0" | "c1" | "c2" | "c3" | "cn";
      surfaceCriterion?: "c0" | "c1" | "c2" | "c3" | "cn";
      tolerance?: number;
      tolerance2d?: number;
    })
  | (ShapeUpgradeDivideOptions & { mode: "angle"; maxAngle: number })
  | (ShapeUpgradeDivideOptions & { mode: "area" } & (
      | { areaMode: "maxArea"; maxArea: number }
      | { areaMode: "parts"; nbParts: number }
      | { areaMode: "uv"; uSplits: number; vSplits: number }
    ))
  | (ShapeUpgradeDivideOptions & { mode: "closedFaces"; splitPoints?: number })
  | (ShapeUpgradeDivideOptions & { mode: "closedEdges" })
  | (ShapeUpgradeDivideOptions & {
      mode: "convertToBezier";
      convert2d?: boolean;
      convert3d?: boolean;
      convertSurfaces?: boolean;
      convertLines?: boolean;
      convertCircles?: boolean;
      convertConics?: boolean;
      convertPlanes?: boolean;
      convertRevolutions?: boolean;
      convertExtrusions?: boolean;
      convertBSplines?: boolean;
    })
  | { mode: "removeInternalWires"; minArea: number; removeFaces?: boolean }
  | { mode: "removeLocations"; removeLevel?: "shape" | "compound" | "solid" | "shell" | "face" };

interface SurfaceAxisDefinition {
  origin?: Vec3;
  direction?: Vec3;
  xDirection?: Vec3;
}

export type AnalyticSurfaceDefinition =
  | (SurfaceAxisDefinition & { type: "plane" })
  | (SurfaceAxisDefinition & { type: "cylinder"; radius: number })
  | (SurfaceAxisDefinition & { type: "cone"; referenceRadius: number; semiAngle: number })
  | (SurfaceAxisDefinition & { type: "sphere"; radius: number })
  | (SurfaceAxisDefinition & { type: "torus"; majorRadius: number; minorRadius: number });

interface SurfaceAxisGeometry {
  origin: Vec3;
  direction: Vec3;
  xDirection: Vec3;
}

export type SurfaceGeometry =
  | (SurfaceAxisGeometry & { type: "plane" })
  | (SurfaceAxisGeometry & { type: "cylinder"; radius: number })
  | (SurfaceAxisGeometry & { type: "cone"; referenceRadius: number; semiAngle: number })
  | (SurfaceAxisGeometry & { type: "sphere"; radius: number })
  | (SurfaceAxisGeometry & { type: "torus"; majorRadius: number; minorRadius: number })
  | { type: "bezier"; uDegree: number; vDegree: number; uPoleCount: number; vPoleCount: number }
  | { type: "bspline"; uDegree: number; vDegree: number; uPoleCount: number; vPoleCount: number; uKnotCount: number; vKnotCount: number }
  | { type: "revolution"; origin: Vec3; direction: Vec3 }
  | { type: "extrusion"; direction: Vec3 }
  | { type: "offset"; offset: number }
  | { type: "other" };

export interface SurfaceEvaluation {
  u: number;
  v: number;
  point: Vec3;
  d1u: Vec3;
  d1v: Vec3;
  d2u: Vec3;
  d2v: Vec3;
  duv: Vec3;
  normalDefined: boolean;
  normal?: Vec3;
  curvatureDefined: boolean;
  minimumCurvature?: number;
  maximumCurvature?: number;
  meanCurvature?: number;
  gaussianCurvature?: number;
  minimumDirection?: Vec3;
  maximumDirection?: Vec3;
}
