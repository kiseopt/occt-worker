import type { HistorySupportLevel, OperationName, KernelErrorCode } from "./generated.js";

export interface KernelErrorData {
  code: KernelErrorCode;
  message: string;
  details?: unknown;
}

export class KernelError extends Error {
  readonly code: KernelErrorCode;
  readonly details: unknown;

  constructor(error: KernelErrorData) {
    super(error.message);
    this.name = "KernelError";
    this.code = error.code;
    this.details = error.details;
  }
}

export class TimeoutError extends Error {
  constructor(message = "Kernel worker request timed out") {
    super(message);
    this.name = "TimeoutError";
  }
}

export interface RequestProgress {
  operation: string;
  fraction: number;
}

export interface RequestOptions {
  timeoutMs?: number;
  signal?: AbortSignal;
  onProgress?: (progress: RequestProgress) => void;
  /** Materialize kernel output buffers into shared memory instead of ordinary ArrayBuffers. */
  outputBuffers?: "array" | "shared";
}

export interface Capabilities {
  protocolVersion: string;
  kernelVersion: string;
  occtVersion: string;
  ops: OperationName[];
  historySupport: Record<OperationName, HistorySupportLevel>;
  buildFlags: Record<string, boolean | string | number>;
}

export interface KernelStats {
  liveShapeHandles: number;
  liveBufferBytes: number;
  wasmMemorySize: number;
}

export interface BBox {
  min: [number, number, number];
  max: [number, number, number];
}

export interface OrientedBBox {
  center: Vec3;
  axes: readonly [Vec3, Vec3, Vec3];
  halfSizes: Vec3;
  axisAligned: boolean;
}

export interface ShapeDiagnosticIssue {
  type: "compound" | "compsolid" | "solid" | "shell" | "face" | "wire" | "edge" | "vertex";
  index: number;
  status: string;
}

export interface ShapeDiagnostics {
  valid: boolean;
  issues: ShapeDiagnosticIssue[];
}

export type BinaryBuffer = ArrayBuffer | SharedArrayBuffer;

export interface MaterializedBuffer<TBuffer extends BinaryBuffer = ArrayBuffer> {
  layout: string;
  data: TBuffer;
}

export type SharedBufferResult<T> =
  T extends ArrayBuffer ? SharedArrayBuffer
    : T extends (...args: never[]) => unknown ? T
      : T extends readonly unknown[] ? { [K in keyof T]: SharedBufferResult<T[K]> }
        : T extends object ? { [K in keyof T]: SharedBufferResult<T[K]> }
          : T;

export interface PLYMeshData {
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
  uvs?: Float32Array;
  colors?: Uint8Array;
  document?: PLYDocument;
}

export interface PLYDocument {
  comments: string[];
  objectInfo: string[];
}

export interface PLYMeshImport extends PLYMeshData {
  shape: import("./client.js").ShapeHandle;
  document: PLYDocument;
}

export interface STLMeshData {
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
}

export type LengthUnit = "mm" | "cm" | "m" | "inch" | "foot";

export interface TessellatedShapeExportOptions {
  linearDeflection?: number;
  angularDeflection?: number;
  relative?: boolean;
}

export interface STLExportOptions extends TessellatedShapeExportOptions {
  encoding?: "binary" | "ascii";
  solidName?: string;
  binaryHeader?: Uint8Array;
}

export interface STLMeshExportOptions {
  encoding?: "binary" | "ascii";
  solidName?: string;
  binaryHeader?: Uint8Array;
}

export interface STLImportOptions {
  includeMesh?: boolean;
}

export interface IGESExportOptions {
  unit?: LengthUnit;
  mode?: "faces" | "brep";
}

export interface IGESImportOptions {
  unit?: LengthUnit;
}

export interface STEPExportOptions {
  unit?: LengthUnit;
  timestamp?: string;
  schema?: "AP203" | "AP214" | "AP242";
}

export interface STEPImportOptions {
  unit?: LengthUnit;
}

export interface GLTFExportOptions extends TessellatedShapeExportOptions {
  format?: "glb" | "gltf";
}

export interface GLTFDocumentExportOptions {
  format?: "glb" | "gltf";
}

export interface OBJExportOptions extends TessellatedShapeExportOptions {}

export interface OBJMeshExportOptions {
  materialLibrary?: string;
}

export interface OBJImportOptions {
  includeDocument?: boolean;
  resources?: Readonly<Record<string, BinaryBuffer>>;
}

export interface PLYExportOptions extends TessellatedShapeExportOptions {
  encoding?: "ascii" | "binary_little_endian" | "binary_big_endian";
}

export interface PLYMeshExportOptions {
  encoding?: "ascii" | "binary_little_endian" | "binary_big_endian";
}

export interface PLYImportOptions {
  includeMesh?: boolean;
}

export interface VRMLExportOptions extends TessellatedShapeExportOptions {
  version?: 1 | 2;
}

export interface VRMLMeshData {
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
  uvs?: Float32Array;
  /** Per-vertex u8 RGBA. VRML stores RGB only, so alpha values must be 255. */
  colors?: Uint8Array;
}

export interface VRMLImportOptions {
  includeMesh?: boolean;
}

export interface STLMeshImport extends STLMeshData {
  shape: import("./client.js").ShapeHandle;
  normals: Float32Array;
  solidName?: string;
  binaryHeader?: Uint8Array;
}

export interface VRMLMeshImport {
  shape: import("./client.js").ShapeHandle;
  positions: Float32Array;
  indices: Uint32Array;
  normals: Float32Array;
  uvs?: Float32Array;
  /** Per-vertex u8 RGBA with opaque alpha reconstructed from VRML RGB colors. */
  colors?: Uint8Array;
}

export interface OBJPrimitive {
  indexStart: number;
  indexCount: number;
  object?: string;
  groups: readonly string[];
  material?: string;
  smoothingGroup?: string;
}

export interface OBJMaterial {
  name: string;
  ambient?: Vec3;
  diffuse?: Vec3;
  specular?: Vec3;
  emissive?: Vec3;
  opacity?: number;
  shininess?: number;
  opticalDensity?: number;
  illuminationModel?: number;
  diffuseMap?: string;
  specularMap?: string;
  opacityMap?: string;
  bumpMap?: string;
}

export interface OBJDocument {
  primitives: readonly OBJPrimitive[];
  materials: readonly OBJMaterial[];
  materialLibraries: readonly string[];
}

export interface OBJMeshImport {
  shape: import("./client.js").ShapeHandle;
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
  uvs?: Float32Array;
  document: OBJDocument;
}

export interface OBJMeshData {
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
  uvs?: Float32Array;
  document?: OBJDocument;
}

export interface OBJMeshExport {
  data: ArrayBuffer;
  resources: Readonly<Record<string, ArrayBuffer>>;
}

export type JSONValue = null | boolean | number | string | readonly JSONValue[] | JSONObject;
export interface JSONObject { readonly [key: string]: JSONValue }

export interface GLTFPrimitiveInstance {
  nodeIndex?: number;
  meshIndex: number;
  primitiveIndex: number;
  mode: number;
  material?: number;
  vertexStart: number;
  vertexCount: number;
  indexStart: number;
  indexCount: number;
  hasNormals: boolean;
  hasUVs: boolean;
}

export interface GLTFResolvedBuffer {
  uri?: string;
  data: BinaryBuffer;
}

export interface GLTFResolvedResource {
  uri: string;
  data: BinaryBuffer;
}

export type GLTFImportOptions = {
  includeDocument?: boolean;
  resources?: Readonly<Record<string, BinaryBuffer>>;
  morphWeights?: readonly number[];
} & (
  | { animationIndex?: never; animationTime?: never }
  | { animationIndex: number; animationTime: number }
);

export interface GLTFDocumentImport {
  shape?: import("./client.js").ShapeHandle;
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
  uvs?: Float32Array;
  document: JSONObject;
  activeScene?: number;
  sceneRoots: readonly number[];
  primitives: readonly GLTFPrimitiveInstance[];
  buffers: readonly GLTFResolvedBuffer[];
  resources: readonly GLTFResolvedResource[];
}

export interface GLTFDocumentData {
  document: JSONObject;
  buffers: readonly GLTFResolvedBuffer[];
  resources?: readonly GLTFResolvedResource[];
}

export interface GLTFDocumentExport {
  data: ArrayBuffer;
  resources: Readonly<Record<string, ArrayBuffer>>;
}

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

export type Matrix12 = readonly [
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
];

export type RGBA = readonly [number, number, number, number];

export type XCAFDocumentFormat = "bin" | "xml";

export interface XCAFMaterial {
  name: string;
  description?: string;
  density: number;
  densityName?: string;
  densityValueType?: string;
}

export interface XCAFVisualMaterial {
  name: string;
  baseColor?: RGBA;
  metallic?: number;
  roughness?: number;
  emissive?: Vec3;
  refractionIndex?: number;
}

export interface XCAFValidationProperties {
  area?: number;
  volume?: number;
  centroid?: Vec3;
}

export interface XCAFSubshapeStyle {
  topology: "face" | "edge";
  index: number;
  color: RGBA;
}

export type XCAFViewProjection = "none" | "parallel" | "central";

export interface XCAFClippingPlaneDefinition {
  name: string;
  origin: Vec3;
  normal: Vec3;
  capping?: boolean;
}

export interface XCAFViewDefinition {
  name: string;
  projection?: XCAFViewProjection;
  projectionPoint?: Vec3;
  viewDirection: Vec3;
  upDirection: Vec3;
  zoomFactor?: number;
  windowHorizontalSize?: number;
  windowVerticalSize?: number;
  frontPlaneDistance?: number;
  backPlaneDistance?: number;
  viewVolumeSidesClipping?: boolean;
  nodeIndices: readonly number[];
  clippingPlanes?: readonly XCAFClippingPlaneDefinition[];
}

export interface XCAFShuoDefinition {
  /** Component node indices from an upper usage to a next usage. */
  nodeIndices: readonly number[];
  /** Optional per-occurrence RGBA style stored on the SHUO graph node. */
  color?: RGBA;
}

export interface XCAFAnnotationPresentation {
  /** Scoped shape containing the annotation's graphical curves and symbols. */
  shape: import("./client.js").ShapeHandle;
  name?: string;
}

export interface XCAFGDTDimension {
  node: number;
  type:
    | "curveLength"
    | "diameter"
    | "sphericalDiameter"
    | "radius"
    | "sphericalRadius"
    | "toroidalMinorDiameter"
    | "toroidalMajorDiameter"
    | "toroidalMinorRadius"
    | "toroidalMajorRadius"
    | "toroidalHighMajorDiameter"
    | "toroidalLowMajorDiameter"
    | "toroidalHighMajorRadius"
    | "toroidalLowMajorRadius"
    | "thickness"
    | "angular";
  value: number;
  semanticName?: string;
  presentation?: XCAFAnnotationPresentation;
}

export interface XCAFGDTDatum {
  node: number;
  name: string;
  description?: string;
  identification: string;
  semanticName?: string;
  presentation?: XCAFAnnotationPresentation;
}

export interface XCAFGeomTolerance {
  node: number;
  type:
    | "angularity"
    | "circularRunout"
    | "circularity"
    | "coaxiality"
    | "concentricity"
    | "cylindricity"
    | "flatness"
    | "parallelism"
    | "perpendicularity"
    | "position"
    | "profileOfLine"
    | "profileOfSurface"
    | "straightness"
    | "symmetry"
    | "totalRunout";
  value: number;
  valueType?: "none" | "diameter" | "sphericalDiameter";
  semanticName?: string;
  datumIndices?: readonly number[];
  materialRequirement?: "none" | "maximum" | "least";
  zoneModifier?: "none" | "projected" | "runout" | "nonUniform";
  zoneModifierValue?: number;
  modifiers?: readonly (
    | "anyCrossSection"
    | "commonZone"
    | "eachRadialElement"
    | "freeState"
    | "leastMaterialRequirement"
    | "lineElement"
    | "majorDiameter"
    | "maximumMaterialRequirement"
    | "minorDiameter"
    | "notConvex"
    | "pitchDiameter"
    | "reciprocityRequirement"
    | "separateRequirement"
    | "statisticalTolerance"
    | "tangentPlane"
    | "allAround"
    | "allOver"
  )[];
  maxValueModifier?: number;
  presentation?: XCAFAnnotationPresentation;
}

export interface STEPDocumentNodeDefinition {
  kind: "part" | "assembly";
  shape?: import("./client.js").ShapeHandle;
  name?: string;
  children?: readonly number[];
  transform?: Matrix12;
  color?: RGBA;
  layers?: readonly string[];
  visible?: boolean;
  material?: XCAFMaterial;
  visualMaterial?: XCAFVisualMaterial;
  validationProperties?: XCAFValidationProperties;
  subshapeStyles?: readonly XCAFSubshapeStyle[];
}

export interface STEPDocumentDefinition {
  nodes: readonly STEPDocumentNodeDefinition[];
  roots: readonly number[];
  gdt?: readonly XCAFGDTDimension[];
  datums?: readonly XCAFGDTDatum[];
  geometricTolerances?: readonly XCAFGeomTolerance[];
  views?: readonly XCAFViewDefinition[];
  shuo?: readonly XCAFShuoDefinition[];
}

export interface STEPDocumentNode {
  kind: "part" | "assembly";
  shape: import("./client.js").ShapeHandle;
  name?: string;
  children: number[];
  transform: Matrix12;
  color?: RGBA;
  layers: string[];
  visible: boolean;
  material?: XCAFMaterial;
  visualMaterial?: XCAFVisualMaterial;
  validationProperties?: XCAFValidationProperties;
  subshapeStyles: XCAFSubshapeStyle[];
}

export interface STEPDocument {
  shape: import("./client.js").ShapeHandle;
  rootCount: number;
  roots: number[];
  nodes: STEPDocumentNode[];
  gdt: XCAFGDTDimension[];
  datums: XCAFGDTDatum[];
  geometricTolerances: XCAFGeomTolerance[];
  views: XCAFViewDefinition[];
  shuo: XCAFShuoDefinition[];
}

export interface FormatProbe {
  format: "brep" | "step" | "iges" | "stl" | "obj" | "ply" | "gltf" | "vrml" | "xcaf" | "unknown";
  encoding?: "text" | "binary" | "json" | "glb";
  confidence: "exact" | "likely" | "none";
  documentMetadata?: boolean;
}

export interface Adjacency {
  from: string;
  to: string;
  items: Array<{ index: number; adjacent: number[] }>;
}

export interface FilletRadiusPoint {
  parameter: number;
  radius: number;
}

export interface TopologyCounts {
  compound: number;
  compsolid: number;
  solid: number;
  shell: number;
  face: number;
  wire: number;
  edge: number;
  vertex: number;
}

export interface HistorySource {
  input: number;
  type: "face" | "edge";
  index: number;
}

export interface HistoryTarget {
  type: "face" | "edge";
  index: number;
}

export interface History {
  retained: Array<{ from: HistorySource; to: HistoryTarget }>;
  generated: Array<{ from: HistorySource; to: HistoryTarget[] }>;
  modified: Array<{ from: HistorySource; to: HistoryTarget[] }>;
  deleted: HistorySource[];
}

export interface BatchOperation {
  op: OperationName;
  args?: Record<string, unknown>;
}

export interface BatchItemResult extends Record<string, unknown> {
  shape?: import("./client.js").ShapeHandle;
  shapes?: import("./client.js").ShapeHandle[];
}

export interface BatchResult {
  results: BatchItemResult[];
  error?: KernelErrorData;
}

export type Vec3 = readonly [number, number, number];

export interface GeneralFuseCellRule {
  take: readonly number[];
  avoid?: readonly number[];
  material?: number;
}

export interface GeneralFuseCellSelectionOptions {
  fuzzyValue?: number;
  glue?: "off" | "shift" | "full";
  useOBB?: boolean;
  removeInternalBoundaries?: boolean;
  makeContainers?: boolean;
}

export interface GlueBinding {
  newIndex: number;
  baseIndex: number;
}

export interface DraftAngleOptions {
  neutralPlane: { origin: Vec3; normal: Vec3 };
  flag?: boolean;
}

interface CylindricalHoleCommonOptions {
  withControl?: boolean;
  includeHistory?: boolean;
}

export type CylindricalHoleOptions =
  | (CylindricalHoleCommonOptions & { mode?: "throughAll" | "throughNext" | "untilEnd" })
  | (CylindricalHoleCommonOptions & { mode: "blind"; length: number })
  | (CylindricalHoleCommonOptions & { mode: "between"; from: number; to: number });
