import { PROTOCOL_VERSION } from "./generated.js";
import { decodeGLTFCompression } from "./gltf-compression.js";
import {
  KernelError,
  type BBox,
  type OrientedBBox,
  type ShapeDiagnostics,
  type Adjacency,
  type Capabilities,
  type CurveDomain,
  type CurveGeometry,
  type CurveControlData,
  type BSplineCurveEdit,
  type Continuity,
  type CurveEvaluation,
  type CurveProjection,
  type SurfaceProjection,
  type CurveCurveExtrema,
  type CurveSurfaceExtrema,
  type SurfaceSurfaceExtrema,
  type CurveSurfaceIntersection,
  type KernelErrorData,
  type KernelStats,
  type RequestOptions,
  type MassProperties,
  type DistanceResult,
  type SectionAnalysis,
  type HLRProjection,
  type HLRProjectionOptions,
  type MultiRootShapeImport,
  type Matrix12,
  type STEPDocument,
  type STEPDocumentDefinition,
  type XCAFDocumentFormat,
  type FormatProbe,
  type MaterializedBuffer,
  type BinaryBuffer,
  type PLYMeshData,
  type PLYMeshImport,
  type STLMeshData,
  type STLExportOptions,
  type STLMeshExportOptions,
  type STLImportOptions,
  type STLMeshImport,
  type IGESExportOptions,
  type IGESImportOptions,
  type STEPExportOptions,
  type STEPImportOptions,
  type GLTFExportOptions,
  type OBJExportOptions,
  type OBJMeshExportOptions,
  type OBJImportOptions,
  type PLYExportOptions,
  type PLYImportOptions,
  type VRMLExportOptions,
  type VRMLMeshData,
  type VRMLImportOptions,
  type VRMLMeshImport,
  type OBJMeshImport,
  type OBJMeshData,
  type OBJMeshExport,
  type GLTFDocumentImport,
  type GLTFDocumentData,
  type GLTFDocumentExport,
  type GLTFImportOptions,
  type SurfaceDomain,
  type SurfaceBounds,
  type SurfaceControlData,
  type BSplineSurfaceEdit,
  type BSplineSurfaceOptions,
  type SurfaceFillingOptions,
  type LocalPrismOptions,
  type LocalRevolutionOptions,
  type LinearFormOptions,
  type RevolutionFormOptions,
  type BSplineApproximationOptions,
  type CurveDegreeReductionOptions,
  type SurfaceDegreeReductionOptions,
  type ShapeUpgradeOptions,
  type ExtensionContinuity,
  type SurfaceGeometry,
  type AnalyticSurfaceDefinition,
  type SurfaceEvaluation,
  type TopologyCounts,
  type Tessellation,
  type TriangulationReplacement,
  type TriangulationValidation,
  type TriangulationRepairResult,
  type EdgeTessellation,
  type BatchOperation,
  type BatchResult,
  type History,
  type GeneralFuseCellRule,
  type GeneralFuseCellSelectionOptions,
  type GlueBinding,
  type DraftAngleOptions,
  type CylindricalHoleOptions,
  type FilletRadiusPoint,
  type Vec3,
} from "./types.js";

let nextEpoch = 1;
const kernelShapeProvenance = Symbol("kernelShapeProvenance");

class ScopeHandle {
  readonly #owner: BaseClient;
  readonly #epoch: number;
  readonly #value: number;

  constructor(owner: BaseClient, value: number) {
    this.#owner = owner;
    this.#epoch = owner.epoch;
    this.#value = value;
  }

  encode(owner: BaseClient): number {
    if (owner !== this.#owner || owner.epoch !== this.#epoch) {
      throw new TypeError("ShapeScope belongs to an expired kernel instance");
    }
    return this.#value;
  }
}

export class ShapeHandle {
  readonly #owner: BaseClient;
  readonly #epoch: number;
  readonly #value: number;

  constructor(owner: BaseClient, epoch: number, value: number, provenance: typeof kernelShapeProvenance) {
    if (provenance !== kernelShapeProvenance) {
      throw new TypeError("ShapeHandle values can only be created from kernel results");
    }
    this.#owner = owner;
    this.#epoch = epoch;
    this.#value = value;
  }

  encode(owner: BaseClient): number {
    if (owner !== this.#owner || owner.epoch !== this.#epoch) {
      throw new TypeError("ShapeHandle belongs to a different or expired kernel instance");
    }
    return this.#value;
  }
}

function shapeFromKernel(owner: BaseClient, value: number): ShapeHandle {
  return new ShapeHandle(owner, owner.epoch, value, kernelShapeProvenance);
}

type AnnotationWithPresentation = {
  presentation?: { shape: ShapeHandle; name?: string };
};

type RawAnnotation<T extends AnnotationWithPresentation> =
  Omit<T, "presentation"> & { presentation?: { shape: number; name?: string } };

function materializeAnnotationPresentations<T extends AnnotationWithPresentation>(
  owner: BaseClient,
  items: readonly RawAnnotation<T>[],
): T[] {
  return items.map((item) => ({
    ...item,
    ...(item.presentation === undefined ? {} : {
      presentation: {
        ...item.presentation,
        shape: shapeFromKernel(owner, item.presentation.shape),
      },
    }),
  })) as T[];
}

type InputBufferDescriptor = {
  $inputBuffer: BinaryBuffer;
  byteOffset?: number;
  byteLength?: number;
};

function inputBuffer(value: BinaryBuffer | ArrayBufferView): InputBufferDescriptor {
  if (ArrayBuffer.isView(value)) {
    if (typeof SharedArrayBuffer !== "undefined" && value.buffer instanceof SharedArrayBuffer) {
      return {
        $inputBuffer: value.buffer,
        byteOffset: value.byteOffset,
        byteLength: value.byteLength,
      };
    }
    return {
      $inputBuffer: new Uint8Array(value.buffer, value.byteOffset, value.byteLength).slice().buffer,
    };
  }
  if (value instanceof ArrayBuffer) return { $inputBuffer: value.slice(0) };
  return { $inputBuffer: value };
}

export abstract class BaseClient {
  epoch = nextEpoch++;

  protected abstract send(op: string, args: Record<string, unknown>, options?: RequestOptions): Promise<unknown>;

  protected invalidateEpoch(): void {
    this.epoch = nextEpoch++;
  }

  async initialize(): Promise<Capabilities> {
    const capabilities = await this.request<Capabilities>("capabilities", {});
    if (capabilities.protocolVersion !== PROTOCOL_VERSION) {
      throw new Error(
        `Unsupported kernel protocol ${capabilities.protocolVersion}; client requires ${PROTOCOL_VERSION}`,
      );
    }
    return capabilities;
  }

  async request<T>(
    op: string,
    args: Record<string, unknown>,
    timeoutOrOptions?: number | RequestOptions,
  ): Promise<T> {
    if (
      op === "makeEdgeBSpline"
      && args.mode !== undefined
      && args.mode !== "interpolate"
      && args.mode !== "controlPoints"
    ) {
      throw new TypeError("BSpline mode must be interpolate or controlPoints");
    }
    const encode = (value: unknown): unknown => {
      if (value instanceof ShapeHandle) return value.encode(this);
      if (value instanceof ScopeHandle) return value.encode(this);
      if (
        value instanceof ArrayBuffer
        || (typeof SharedArrayBuffer !== "undefined" && value instanceof SharedArrayBuffer)
        || ArrayBuffer.isView(value)
      ) return value;
      if (Array.isArray(value)) return value.map(encode);
      if (value !== null && typeof value === "object") {
        return Object.fromEntries(Object.entries(value).map(([key, item]) => [key, encode(item)]));
      }
      return value;
    };
    const options = typeof timeoutOrOptions === "number"
      ? { timeoutMs: timeoutOrOptions }
      : timeoutOrOptions;
    return (await this.send(op, encode(args) as Record<string, unknown>, options)) as T;
  }

  async beginScope(): Promise<ShapeScope> {
    const result = await this.request<{ scopeId: number }>("beginScope", {});
    return new ShapeScope(this, new ScopeHandle(this, result.scopeId));
  }

  async release(shape: ShapeHandle): Promise<void> {
    await this.request("release", { shape });
  }

  async releaseAll(): Promise<void> {
    await this.request("releaseAll", {});
  }

  async stats(): Promise<KernelStats> {
    return this.request("stats", {});
  }

  async bbox(shape: ShapeHandle): Promise<BBox> {
    return this.request("bbox", { shape });
  }

  async obb(
    shape: ShapeHandle,
    options: { useTriangulation?: boolean; optimal?: boolean; useShapeTolerance?: boolean } = {},
  ): Promise<OrientedBBox> {
    return this.request("obb", { shape, ...options });
  }

  async diagnoseShape(
    shape: ShapeHandle,
    options: { geomControls?: boolean; exact?: boolean } = {},
  ): Promise<ShapeDiagnostics> {
    return this.request("diagnoseShape", { shape, ...options });
  }

  async inspectTolerances(shape: ShapeHandle): Promise<{
    faces: Array<{ index: number; tolerance: number }>;
    edges: Array<{ index: number; tolerance: number }>;
    vertices: Array<{ index: number; tolerance: number }>;
  }> {
    return this.request("inspectTolerances", { shape });
  }

  async massProps(shape: ShapeHandle, kind: "volume" | "surface" | "linear" = "volume"): Promise<MassProperties> {
    return this.request<MassProperties>("massProps", { shape, kind });
  }

  async topologyCounts(shape: ShapeHandle): Promise<TopologyCounts> {
    return this.request<TopologyCounts>("topologyCounts", { shape });
  }

  async isValid(shape: ShapeHandle): Promise<boolean> {
    return (await this.request<{ valid: boolean }>("isValid", { shape })).valid;
  }

  async distance(first: ShapeHandle, second: ShapeHandle): Promise<DistanceResult> {
    return this.request("distance", { first, second });
  }

  async classifyPoint(shape: ShapeHandle, point: Vec3, tolerance?: number): Promise<"inside" | "outside" | "on" | "unknown"> {
    const result = await this.request<{ state: "inside" | "outside" | "on" | "unknown" }>("classifyPoint", {
      shape, point, ...(tolerance === undefined ? {} : { tolerance }),
    });
    return result.state;
  }

  async getAdjacency(shape: ShapeHandle): Promise<Adjacency>;
  async getAdjacency(shape: ShapeHandle, from: "edge", to?: "face" | "vertex"): Promise<Adjacency>;
  async getAdjacency(shape: ShapeHandle, from: "face" | "vertex", to?: "edge"): Promise<Adjacency>;
  async getAdjacency(
    shape: ShapeHandle,
    from: "face" | "edge" | "vertex" = "edge",
    to: "face" | "edge" | "vertex" = from === "face" || from === "vertex" ? "edge" : "face",
  ): Promise<Adjacency> {
    return this.request("getAdjacency", { shape, from, to });
  }

  async shapeType(shape: ShapeHandle): Promise<string> {
    return (await this.request<{ type: string }>("shapeType", { shape })).type;
  }

  async getSubShapes(shape: ShapeHandle, type: "compound" | "compsolid" | "solid" | "shell" | "face" | "wire" | "edge" | "vertex" = "face"): Promise<Array<{ type: string; index: number }>> {
    return (await this.request<{ shapes: Array<{ type: string; index: number }> }>("getSubShapes", { shape, type })).shapes;
  }

  async curveDomain(shape: ShapeHandle): Promise<CurveDomain> {
    return this.request("curveDomain", { shape });
  }

  async curveGeometry(shape: ShapeHandle): Promise<CurveGeometry> {
    return this.request("curveGeometry", { shape });
  }

  async curveControlData(shape: ShapeHandle): Promise<CurveControlData> {
    return this.request("curveControlData", { shape });
  }

  async curveContinuity(shape: ShapeHandle): Promise<Continuity> {
    return (await this.request<{ continuity: Continuity }>("curveContinuity", { shape })).continuity;
  }

  async evaluateCurve(shape: ShapeHandle, parameter: number, resolution?: number): Promise<CurveEvaluation> {
    return this.request("evaluateCurve", {
      shape, parameter, ...(resolution === undefined ? {} : { resolution }),
    });
  }

  async projectPointCurve(shape: ShapeHandle, point: Vec3): Promise<CurveProjection> {
    return this.request("projectPointCurve", { shape, point });
  }

  async projectPointSurface(shape: ShapeHandle, point: Vec3, tolerance?: number): Promise<SurfaceProjection> {
    return this.request("projectPointSurface", {
      shape,
      point,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
  }

  async extremaCurveCurve(first: ShapeHandle, second: ShapeHandle): Promise<CurveCurveExtrema> {
    return this.request("extremaCurveCurve", { first, second });
  }

  async extremaCurveSurface(curve: ShapeHandle, surface: ShapeHandle): Promise<CurveSurfaceExtrema> {
    return this.request("extremaCurveSurface", { curve, surface });
  }

  async extremaSurfaceSurface(first: ShapeHandle, second: ShapeHandle): Promise<SurfaceSurfaceExtrema> {
    return this.request("extremaSurfaceSurface", { first, second });
  }

  async intersectCurveSurface(curve: ShapeHandle, surface: ShapeHandle, tolerance = 1e-7): Promise<CurveSurfaceIntersection> {
    return this.request("intersectCurveSurface", { curve, surface, tolerance });
  }

  async surfaceDomain(shape: ShapeHandle): Promise<SurfaceDomain> {
    return this.request("surfaceDomain", { shape });
  }

  async surfaceContinuity(shape: ShapeHandle): Promise<{ u: Continuity; v: Continuity }> {
    const result = await this.request<{ uContinuity: Continuity; vContinuity: Continuity }>("surfaceContinuity", { shape });
    return { u: result.uContinuity, v: result.vContinuity };
  }

  async surfaceGeometry(shape: ShapeHandle): Promise<SurfaceGeometry> {
    return this.request("surfaceGeometry", { shape });
  }

  async surfaceControlData(shape: ShapeHandle): Promise<SurfaceControlData> {
    return this.request("surfaceControlData", { shape });
  }

  async evaluateSurface(shape: ShapeHandle, u: number, v: number, resolution?: number): Promise<SurfaceEvaluation> {
    return this.request("evaluateSurface", {
      shape, u, v, ...(resolution === undefined ? {} : { resolution }),
    });
  }

  async isSameShape(first: ShapeHandle, second: ShapeHandle): Promise<boolean> {
    return (await this.request<{ same: boolean }>("isSameShape", { first, second })).same;
  }

  async tessellate(
    shape: ShapeHandle,
    options: { linearDeflection?: number; angularDeflection?: number; relative?: boolean; includeUV?: boolean } = {},
  ): Promise<Tessellation> {
    const result = await this.request<Record<string, MaterializedBuffer>>("tessellate", { shape, ...options });
    return {
      positions: new Float32Array(result.positions!.data),
      normals: new Float32Array(result.normals!.data),
      indices: new Uint32Array(result.indices!.data),
      faceGroups: new Uint32Array(result.faceGroups!.data),
      ...(result.uvs === undefined ? {} : { uvs: new Float32Array(result.uvs.data) }),
    };
  }

  async triangulationData(shape: ShapeHandle, options: { includeUV?: boolean } = {}): Promise<Tessellation> {
    const result = await this.request<Record<string, MaterializedBuffer>>("triangulationData", { shape, ...options });
    return {
      positions: new Float32Array(result.positions!.data),
      normals: new Float32Array(result.normals!.data),
      indices: new Uint32Array(result.indices!.data),
      faceGroups: new Uint32Array(result.faceGroups!.data),
      ...(result.uvs === undefined ? {} : { uvs: new Float32Array(result.uvs.data) }),
    };
  }

  async validateTriangulation(
    shape: ShapeHandle,
    options: { tolerance?: number; checkNormals?: boolean; checkUV?: boolean } = {},
  ): Promise<TriangulationValidation> {
    return this.request("validateTriangulation", { shape, ...options });
  }

  async tessellateEdges(shape: ShapeHandle, options: { linearDeflection?: number; angularDeflection?: number } = {}): Promise<EdgeTessellation> {
    const result = await this.request<Record<string, MaterializedBuffer>>("tessellateEdges", { shape, ...options });
    return {
      positions: new Float32Array(result.positions!.data),
      edgeGroups: new Uint32Array(result.edgeGroups!.data),
    };
  }

  async exportSTL(
    shape: ShapeHandle,
    options?: STLExportOptions,
  ): Promise<ArrayBuffer>;
  async exportSTL(mesh: STLMeshData, options?: STLMeshExportOptions): Promise<ArrayBuffer>;
  async exportSTL(
    source: ShapeHandle | STLMeshData,
    options: STLExportOptions = {},
  ): Promise<ArrayBuffer> {
    const encodedOptions = {
      ...options,
      ...(options.binaryHeader === undefined
        ? {}
        : { binaryHeader: inputBuffer(options.binaryHeader) }),
    };
    let args: Record<string, unknown>;
    if (source instanceof ShapeHandle) {
      args = { shape: source, ...encodedOptions };
    }
    else {
      args = {
        positions: inputBuffer(source.positions),
        indices: inputBuffer(source.indices),
        ...(source.normals === undefined ? {} : { normals: inputBuffer(source.normals) }),
        ...encodedOptions,
      };
    }
    const result = await this.request<{ data: MaterializedBuffer }>("exportSTL", args);
    return result.data.data;
  }

  async exportBREP(shape: ShapeHandle): Promise<ArrayBuffer> {
    const result = await this.request<{ data: MaterializedBuffer }>("exportBREP", { shape });
    return result.data.data;
  }

  async exportIGES(
    shape: ShapeHandle | readonly ShapeHandle[],
    options: IGESExportOptions = {},
  ): Promise<ArrayBuffer> {
    const result = await this.request<{ data: MaterializedBuffer }>("exportIGES", {
      ...(Array.isArray(shape) ? { shapes: shape } : { shape }),
      ...options,
    });
    return result.data.data;
  }

  async exportIGESDocument(
    document: STEPDocumentDefinition,
    options: Pick<IGESExportOptions, "unit"> = {},
  ): Promise<ArrayBuffer> {
    const result = await this.request<{ data: MaterializedBuffer }>("exportIGESDocument", {
      ...document,
      ...options,
    });
    return result.data.data;
  }

  async exportGLTF(
    shape: ShapeHandle,
    options?: GLTFExportOptions,
  ): Promise<ArrayBuffer>;
  async exportGLTF(
    document: GLTFDocumentData,
    options?: Pick<GLTFExportOptions, "format">,
  ): Promise<GLTFDocumentExport>;
  async exportGLTF(
    source: ShapeHandle | GLTFDocumentData,
    options: GLTFExportOptions = {},
  ): Promise<ArrayBuffer | GLTFDocumentExport> {
    if (source instanceof ShapeHandle) {
      const result = await this.request<{ data: MaterializedBuffer }>("exportGLTF", { shape: source, ...options });
      return result.data.data;
    }
    const result = await this.request<{
      data: MaterializedBuffer;
      resources: Array<{ uri: string; data: MaterializedBuffer }>;
    }>("exportGLTF", {
      document: source.document,
      buffers: source.buffers.map((buffer) => ({
        ...(buffer.uri === undefined ? {} : { uri: buffer.uri }),
        data: inputBuffer(buffer.data),
      })),
      resources: (source.resources ?? []).map((resource) => ({
        uri: resource.uri,
        data: inputBuffer(resource.data),
      })),
      ...(options.format === undefined ? {} : { format: options.format }),
    });
    return {
      data: result.data.data,
      resources: Object.freeze(Object.fromEntries(
        result.resources.map((resource) => [resource.uri, resource.data.data]),
      )),
    };
  }

  async exportOBJ(
    shape: ShapeHandle,
    options?: OBJExportOptions,
  ): Promise<ArrayBuffer>;
  async exportOBJ(
    mesh: OBJMeshData,
    options?: OBJMeshExportOptions,
  ): Promise<OBJMeshExport>;
  async exportOBJ(
    shape: ShapeHandle | OBJMeshData,
    options: OBJExportOptions | OBJMeshExportOptions = {},
  ): Promise<ArrayBuffer | OBJMeshExport> {
    if (shape instanceof ShapeHandle) {
      const result = await this.request<{ data: MaterializedBuffer }>("exportOBJ", { shape, ...options });
      return result.data.data;
    }
    const meshOptions = options as OBJMeshExportOptions;
    const result = await this.request<{
      data: MaterializedBuffer;
      resources: Array<{ uri: string; data: MaterializedBuffer }>;
    }>("exportOBJ", {
      positions: inputBuffer(shape.positions),
      indices: inputBuffer(shape.indices),
      ...(shape.normals === undefined ? {} : { normals: inputBuffer(shape.normals) }),
      ...(shape.uvs === undefined ? {} : { uvs: inputBuffer(shape.uvs) }),
      ...(shape.document === undefined ? {} : { document: shape.document }),
      ...(meshOptions.materialLibrary === undefined
        ? {}
        : { materialLibrary: meshOptions.materialLibrary }),
    });
    return {
      data: result.data.data,
      resources: Object.freeze(Object.fromEntries(
        result.resources.map((resource) => [resource.uri, resource.data.data]),
      )),
    };
  }

  async exportPLY(
    shape: ShapeHandle,
    options?: PLYExportOptions,
  ): Promise<ArrayBuffer>;
  async exportPLY(
    mesh: PLYMeshData,
    options?: Pick<PLYExportOptions, "encoding">,
  ): Promise<ArrayBuffer>;
  async exportPLY(
    shape: ShapeHandle | PLYMeshData,
    options: PLYExportOptions = {},
  ): Promise<ArrayBuffer> {
    let args: Record<string, unknown>;
    if (shape instanceof ShapeHandle) {
      args = { shape, ...options };
    }
    else {
      args = {
        positions: inputBuffer(shape.positions),
        indices: inputBuffer(shape.indices),
        ...(shape.normals === undefined ? {} : { normals: inputBuffer(shape.normals) }),
        ...(shape.uvs === undefined ? {} : { uvs: inputBuffer(shape.uvs) }),
        ...(shape.colors === undefined ? {} : { colors: inputBuffer(shape.colors) }),
        ...(shape.document === undefined ? {} : { document: shape.document }),
        ...(options.encoding === undefined ? {} : { encoding: options.encoding }),
      };
    }
    const result = await this.request<{ data: MaterializedBuffer }>("exportPLY", args);
    return result.data.data;
  }

  async exportSTEP(
    shape: ShapeHandle | readonly ShapeHandle[],
    options: STEPExportOptions = {},
  ): Promise<ArrayBuffer> {
    const result = await this.request<{ data: MaterializedBuffer }>("exportSTEP", {
      ...(Array.isArray(shape) ? { shapes: shape } : { shape }),
      ...options,
    });
    return result.data.data;
  }

  async exportSTEPDocument(
    document: STEPDocumentDefinition,
    options: STEPExportOptions = {},
  ): Promise<ArrayBuffer> {
    const result = await this.request<{ data: MaterializedBuffer }>("exportSTEPDocument", {
      ...document,
      ...options,
    });
    return result.data.data;
  }

  async exportXCAF(
    document: STEPDocumentDefinition,
    format: XCAFDocumentFormat = "bin",
  ): Promise<ArrayBuffer> {
    const result = await this.request<{ data: MaterializedBuffer }>("exportXCAF", {
      ...document,
      format,
    });
    return result.data.data;
  }

  async exportVRML(
    shape: ShapeHandle,
    options?: VRMLExportOptions,
  ): Promise<ArrayBuffer>;
  async exportVRML(
    mesh: VRMLMeshData,
    options?: Pick<VRMLExportOptions, "version">,
  ): Promise<ArrayBuffer>;
  async exportVRML(
    source: ShapeHandle | VRMLMeshData,
    options: VRMLExportOptions | Pick<VRMLExportOptions, "version"> = {},
  ): Promise<ArrayBuffer> {
    let args: Record<string, unknown>;
    if (source instanceof ShapeHandle) {
      args = { shape: source, ...options };
    }
    else {
      args = {
        positions: inputBuffer(source.positions),
        indices: inputBuffer(source.indices),
        ...(source.normals === undefined ? {} : { normals: inputBuffer(source.normals) }),
        ...(source.uvs === undefined ? {} : { uvs: inputBuffer(source.uvs) }),
        ...(source.colors === undefined ? {} : { colors: inputBuffer(source.colors) }),
        ...(options.version === undefined ? {} : { version: options.version }),
      };
    }
    const result = await this.request<{ data: MaterializedBuffer }>("exportVRML", args);
    return result.data.data;
  }

  async probeFormat(data: BinaryBuffer): Promise<FormatProbe> {
    return this.request("probeFormat", { data: { $inputBuffer: data } });
  }


  kernelError(error: KernelErrorData): never {
    throw new KernelError(error);
  }
}

export class ShapeScope implements AsyncDisposable {
  readonly #client: BaseClient;
  readonly #scopeId: ScopeHandle;
  #endPromise: Promise<void> | undefined;

  constructor(client: BaseClient, scopeId: ScopeHandle) {
    this.#client = client;
    this.#scopeId = scopeId;
  }

  get scopeId(): number {
    return this.#scopeId.encode(this.#client);
  }

  async batch(operations: readonly BatchOperation[]): Promise<BatchResult> {
    const result = await this.#client.request<{
      results: Array<Record<string, unknown> & { shape?: number; shapes?: number[] }>;
      error?: BatchResult["error"];
    }>("batch", { scopeId: this.#scopeId, ops: operations });
    return {
      results: result.results.map(({ shape, shapes, ...rest }) => ({
        ...rest,
        ...(shape === undefined ? {} : { shape: shapeFromKernel(this.#client, shape) }),
        ...(shapes === undefined ? {} : {
          shapes: shapes.map((value) => shapeFromKernel(this.#client, value)),
        }),
      })),
      ...(result.error === undefined ? {} : { error: result.error }),
    };
  }

  async makeBox(size: Vec3, origin: Vec3 = [0, 0, 0]): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeBox", {
      scopeId: this.#scopeId,
      size,
      origin,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeCylinder(
    radius: number,
    height: number,
    options: { origin?: Vec3; direction?: Vec3 } = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeCylinder", {
      scopeId: this.#scopeId,
      radius,
      height,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeSphere(radius: number, options: { origin?: Vec3; direction?: Vec3 } = {}): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeSphere", {
      scopeId: this.#scopeId, radius, ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeCone(radius1: number, radius2: number, height: number, options: { origin?: Vec3; direction?: Vec3 } = {}): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeCone", {
      scopeId: this.#scopeId, radius1, radius2, height, ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeTorus(majorRadius: number, minorRadius: number, options: { origin?: Vec3; direction?: Vec3 } = {}): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeTorus", {
      scopeId: this.#scopeId, majorRadius, minorRadius, ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeWedge(
    size: Vec3,
    ltx: number,
    options: { origin?: Vec3; direction?: Vec3 } = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeWedge", {
      scopeId: this.#scopeId,
      size,
      ltx,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeHalfSpace(face: ShapeHandle, referencePoint: Vec3): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeHalfSpace", {
      scopeId: this.#scopeId,
      face,
      referencePoint,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeVertex(point: Vec3): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeVertex", { scopeId: this.#scopeId, point });
    return shapeFromKernel(this.#client, result.shape);
  }

  async getSubShape(
    shape: ShapeHandle,
    type: "compound" | "compsolid" | "solid" | "shell" | "face" | "wire" | "edge" | "vertex",
    index: number,
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("getSubShape", {
      scopeId: this.#scopeId, shape, type, index,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makePolygon(points: readonly Vec3[], close = true): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makePolygon", { scopeId: this.#scopeId, points, close });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeWire(edges: readonly ShapeHandle[]): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeWire", { scopeId: this.#scopeId, edges });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeFace(outer: ShapeHandle, holes: readonly ShapeHandle[] = []): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeFace", { scopeId: this.#scopeId, outer, holes });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeFaceOnSurface(
    surface: ShapeHandle,
    outer: ShapeHandle,
    holes: readonly ShapeHandle[] = [],
    tolerance?: number,
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeFaceOnSurface", {
      scopeId: this.#scopeId,
      surface,
      outer,
      holes,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeCompound(shapes: readonly ShapeHandle[]): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeCompound", {
      scopeId: this.#scopeId,
      shapes,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeShell(faces: readonly ShapeHandle[]): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeShell", {
      scopeId: this.#scopeId,
      faces,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeCompSolid(solids: readonly ShapeHandle[]): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeCompSolid", {
      scopeId: this.#scopeId,
      solids,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeSurfaceFace(
    surface: AnalyticSurfaceDefinition,
    bounds: SurfaceBounds,
    tolerance?: number,
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeSurfaceFace", {
      scopeId: this.#scopeId,
      surface,
      ...bounds,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeSurfaceBezier(
    poles: readonly (readonly Vec3[])[],
    options: { weights?: readonly (readonly number[])[]; tolerance?: number } = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeSurfaceBezier", {
      scopeId: this.#scopeId,
      poles,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeSurfaceBSpline(
    poles: readonly (readonly Vec3[])[],
    options: BSplineSurfaceOptions = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeSurfaceBSpline", {
      scopeId: this.#scopeId,
      poles,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeSurfaceExtrusion(
    profile: ShapeHandle,
    direction: Vec3,
    vFirst: number,
    vLast: number,
    tolerance?: number,
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeSurfaceExtrusion", {
      scopeId: this.#scopeId,
      profile,
      direction,
      vFirst,
      vLast,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeSurfaceRevolution(
    profile: ShapeHandle,
    options: { origin?: Vec3; direction?: Vec3; angle?: number; tolerance?: number } = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeSurfaceRevolution", {
      scopeId: this.#scopeId,
      profile,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeSurfaceRuled(first: ShapeHandle, second: ShapeHandle): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeSurfaceRuled", {
      scopeId: this.#scopeId,
      first,
      second,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeSurfaceOffset(shape: ShapeHandle, offset: number, tolerance?: number): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeSurfaceOffset", {
      scopeId: this.#scopeId,
      shape,
      offset,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeSurfaceFilling(
    boundary: ShapeHandle,
    options: SurfaceFillingOptions = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeSurfaceFilling", {
      scopeId: this.#scopeId,
      boundary,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async updateSurfacePole(
    shape: ShapeHandle,
    uIndex: number,
    vIndex: number,
    point: Vec3,
    options: { weight?: number; tolerance?: number } = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("updateSurfacePole", {
      scopeId: this.#scopeId,
      shape,
      uIndex,
      vIndex,
      point,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async editSurfaceBSpline(shape: ShapeHandle, edit: BSplineSurfaceEdit): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("editSurfaceBSpline", {
      scopeId: this.#scopeId,
      shape,
      ...edit,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async reduceSurfaceDegree(
    shape: ShapeHandle,
    uDegree: number,
    vDegree: number,
    options: SurfaceDegreeReductionOptions = {},
  ): Promise<{ shape: ShapeHandle; maxError: number }> {
    const result = await this.#client.request<{ shape: number; maxError: number }>("reduceSurfaceDegree", {
      scopeId: this.#scopeId,
      shape,
      uDegree,
      vDegree,
      ...options,
    });
    return { shape: shapeFromKernel(this.#client, result.shape), maxError: result.maxError };
  }

  async reparameterizeSurface(
    shape: ShapeHandle,
    bounds: SurfaceBounds,
    tolerance?: number,
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("reparameterizeSurface", {
      scopeId: this.#scopeId,
      shape,
      ...bounds,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async replaceTriangulation(shape: ShapeHandle, mesh: TriangulationReplacement): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("replaceTriangulation", {
      scopeId: this.#scopeId,
      shape,
      ...mesh,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async repairTriangulation(
    shape: ShapeHandle,
    options: { linearDeflection?: number; angularDeflection?: number; relative?: boolean } = {},
  ): Promise<TriangulationRepairResult> {
    const result = await this.#client.request<{ shape: number; faces: number; triangulatedFaces: number; triangles: number }>(
      "repairTriangulation",
      { scopeId: this.#scopeId, shape, ...options },
    );
    return {
      shape: shapeFromKernel(this.#client, result.shape),
      faces: result.faces,
      triangulatedFaces: result.triangulatedFaces,
      triangles: result.triangles,
    };
  }

  async trimSurface(shape: ShapeHandle, bounds: SurfaceBounds, tolerance?: number): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("trimSurface", {
      scopeId: this.#scopeId,
      shape,
      ...bounds,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async convertSurfaceToBSpline(shape: ShapeHandle, tolerance?: number): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("convertSurfaceToBSpline", {
      scopeId: this.#scopeId,
      shape,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async surfaceIsoCurve(
    surface: ShapeHandle,
    direction: "u" | "v",
    parameter: number,
    tolerance?: number,
  ): Promise<ShapeHandle[]> {
    const result = await this.#client.request<{ shapes: number[] }>("surfaceIsoCurve", {
      scopeId: this.#scopeId,
      surface,
      direction,
      parameter,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return result.shapes.map((shape) => shapeFromKernel(this.#client, shape));
  }

  async extrude(shape: ShapeHandle, vector: Vec3, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async extrude(shape: ShapeHandle, vector: Vec3, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async extrude(shape: ShapeHandle, vector: Vec3, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("extrude", { scopeId: this.#scopeId, shape, vector, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("extrude did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async revolve(shape: ShapeHandle, origin: Vec3, direction: Vec3, angle?: number, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async revolve(shape: ShapeHandle, origin: Vec3, direction: Vec3, angle: number | undefined, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async revolve(shape: ShapeHandle, origin: Vec3, direction: Vec3, angle?: number, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("revolve", { scopeId: this.#scopeId, shape, origin, direction, ...(angle === undefined ? {} : { angle }), ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("revolve did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async localPrism(
    base: ShapeHandle,
    faceIndices: readonly number[],
    direction: Vec3,
    length: number,
    operation?: "add" | "cut",
  ): Promise<ShapeHandle>;
  async localPrism(
    base: ShapeHandle,
    faceIndices: readonly number[],
    direction: Vec3,
    options: LocalPrismOptions,
  ): Promise<ShapeHandle>;
  async localPrism(
    base: ShapeHandle,
    faceIndices: readonly number[],
    direction: Vec3,
    limit: number | LocalPrismOptions,
    operation: "add" | "cut" = "add",
  ): Promise<ShapeHandle> {
    const limitArgs = typeof limit === "number"
      ? { length: limit, operation }
      : { operation: "add" as const, ...limit };
    const result = await this.#client.request<{ shape: number }>("localPrism", {
      scopeId: this.#scopeId,
      base,
      faceIndices,
      direction,
      ...limitArgs,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async localRevolution(
    base: ShapeHandle,
    faceIndices: readonly number[],
    origin: Vec3,
    direction: Vec3,
    angle: number,
    operation?: "add" | "cut",
  ): Promise<ShapeHandle>;
  async localRevolution(
    base: ShapeHandle,
    faceIndices: readonly number[],
    origin: Vec3,
    direction: Vec3,
    options: LocalRevolutionOptions,
  ): Promise<ShapeHandle>;
  async localRevolution(
    base: ShapeHandle,
    faceIndices: readonly number[],
    origin: Vec3,
    direction: Vec3,
    limit: number | LocalRevolutionOptions,
    operation: "add" | "cut" = "add",
  ): Promise<ShapeHandle> {
    const limitArgs = typeof limit === "number"
      ? { angle: limit, operation }
      : { operation: "add" as const, ...limit };
    const result = await this.#client.request<{ shape: number }>("localRevolution", {
      scopeId: this.#scopeId,
      base,
      faceIndices,
      origin,
      direction,
      ...limitArgs,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async linearForm(
    base: ShapeHandle,
    profile: ShapeHandle,
    options: LinearFormOptions,
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("linearForm", {
      scopeId: this.#scopeId,
      base,
      profile,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async revolutionForm(
    base: ShapeHandle,
    profile: ShapeHandle,
    options: RevolutionFormOptions,
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("revolutionForm", {
      scopeId: this.#scopeId,
      base,
      profile,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async cylindricalHole(
    shape: ShapeHandle,
    origin: Vec3,
    direction: Vec3,
    radius: number,
    options?: CylindricalHoleOptions & { includeHistory?: false },
  ): Promise<{ shape: ShapeHandle }>;
  async cylindricalHole(
    shape: ShapeHandle,
    origin: Vec3,
    direction: Vec3,
    radius: number,
    options: CylindricalHoleOptions & { includeHistory: true },
  ): Promise<{ shape: ShapeHandle; history: History }>;
  async cylindricalHole(
    shape: ShapeHandle,
    origin: Vec3,
    direction: Vec3,
    radius: number,
    options: CylindricalHoleOptions = {},
  ): Promise<{ shape: ShapeHandle; history?: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("cylindricalHole", {
      scopeId: this.#scopeId,
      shape,
      origin,
      direction,
      radius,
      ...options,
    });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return { shape: resultShape };
    if (result.history === undefined) throw new Error("cylindricalHole did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async defeature(
    shape: ShapeHandle,
    faceIndices: readonly number[],
    options?: { includeHistory?: false },
  ): Promise<{ shape: ShapeHandle }>;
  async defeature(
    shape: ShapeHandle,
    faceIndices: readonly number[],
    options: { includeHistory: true },
  ): Promise<{ shape: ShapeHandle; history: History }>;
  async defeature(
    shape: ShapeHandle,
    faceIndices: readonly number[],
    options: { includeHistory?: boolean } = {},
  ): Promise<{ shape: ShapeHandle; history?: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("defeature", {
      scopeId: this.#scopeId,
      shape,
      faceIndices,
      ...options,
    });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return { shape: resultShape };
    if (result.history === undefined) throw new Error("defeature did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async loft(sections: readonly ShapeHandle[], options?: { solid?: boolean; ruled?: boolean; precision?: number; includeHistory?: false }): Promise<ShapeHandle>;
  async loft(sections: readonly ShapeHandle[], options: { solid?: boolean; ruled?: boolean; precision?: number; includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async loft(sections: readonly ShapeHandle[], options: { solid?: boolean; ruled?: boolean; precision?: number; includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("loft", { scopeId: this.#scopeId, sections, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("loft did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async sweepPipe(spine: ShapeHandle, profile: ShapeHandle, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async sweepPipe(spine: ShapeHandle, profile: ShapeHandle, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async sweepPipe(spine: ShapeHandle, profile: ShapeHandle, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("sweepPipe", { scopeId: this.#scopeId, spine, profile, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("sweepPipe did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async middlePath(
    shape: ShapeHandle,
    start: ShapeHandle,
    end: ShapeHandle,
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("middlePath", {
      scopeId: this.#scopeId,
      shape,
      start,
      end,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async sweepPipeShell(
    spine: ShapeHandle,
    profiles: readonly ShapeHandle[],
    options: "frenet" | "correctedFrenet" | "fixedAxis" | "auxiliarySpine" | {
      mode: "fixedAxis" | "auxiliarySpine";
      axis?: { origin?: Vec3; direction: Vec3; xDirection?: Vec3 };
      auxiliarySpine?: ShapeHandle;
      curvilinearEquivalence?: boolean;
      includeHistory?: false;
    },
  ): Promise<ShapeHandle>;
  async sweepPipeShell(
    spine: ShapeHandle,
    profiles: readonly ShapeHandle[],
    options: {
      mode?: "frenet" | "correctedFrenet" | "fixedAxis" | "auxiliarySpine";
      axis?: { origin?: Vec3; direction: Vec3; xDirection?: Vec3 };
      auxiliarySpine?: ShapeHandle;
      curvilinearEquivalence?: boolean;
      includeHistory: true;
    },
  ): Promise<{ shape: ShapeHandle; history: History }>;
  async sweepPipeShell(
    spine: ShapeHandle,
    profiles: readonly ShapeHandle[],
    options: "frenet" | "correctedFrenet" | "fixedAxis" | "auxiliarySpine" | {
      mode?: "frenet" | "correctedFrenet" | "fixedAxis" | "auxiliarySpine";
      axis?: { origin?: Vec3; direction: Vec3; xDirection?: Vec3 };
      auxiliarySpine?: ShapeHandle;
      curvilinearEquivalence?: boolean;
      includeHistory?: boolean;
    } = "frenet",
  ): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const requestOptions = typeof options === "string" ? { mode: options } : options;
    const result = await this.#client.request<{ shape: number; history?: History }>("sweepPipeShell", { scopeId: this.#scopeId, spine, profiles, ...requestOptions });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!requestOptions.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("sweepPipeShell did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async fillet(shape: ShapeHandle, edgeIndices: readonly number[], radius: number, options?: { radii?: readonly number[]; radius2?: number; radiusLaw?: readonly FilletRadiusPoint[]; radiusLaws?: readonly (readonly FilletRadiusPoint[])[]; includeHistory?: false }): Promise<ShapeHandle>;
  async fillet(shape: ShapeHandle, edgeIndices: readonly number[], radius: number, options: { radii?: readonly number[]; radius2?: number; radiusLaw?: readonly FilletRadiusPoint[]; radiusLaws?: readonly (readonly FilletRadiusPoint[])[]; includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async fillet(shape: ShapeHandle, edgeIndices: readonly number[], radius: number, options: { radii?: readonly number[]; radius2?: number; radiusLaw?: readonly FilletRadiusPoint[]; radiusLaws?: readonly (readonly FilletRadiusPoint[])[]; includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("fillet", { scopeId: this.#scopeId, shape, edgeIndices, radius, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("fillet did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async chamfer(shape: ShapeHandle, edgeIndices: readonly number[], distance: number, options?: { distances?: readonly number[]; distance2?: number; distances2?: readonly number[]; referenceFaceIndices?: readonly number[]; includeHistory?: false }): Promise<ShapeHandle>;
  async chamfer(shape: ShapeHandle, edgeIndices: readonly number[], distance: number, options: { distances?: readonly number[]; distance2?: number; distances2?: readonly number[]; referenceFaceIndices?: readonly number[]; includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async chamfer(shape: ShapeHandle, edgeIndices: readonly number[], distance: number, options: { distances?: readonly number[]; distance2?: number; distances2?: readonly number[]; referenceFaceIndices?: readonly number[]; includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("chamfer", { scopeId: this.#scopeId, shape, edgeIndices, distance, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("chamfer did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async draftAngle(
    shape: ShapeHandle,
    faceIndices: readonly number[],
    direction: Vec3,
    angle: number,
    options: DraftAngleOptions,
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("draftAngle", {
      scopeId: this.#scopeId,
      shape,
      faceIndices,
      direction,
      angle,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async hollow(
    shape: ShapeHandle,
    thickness: number,
    closingFaceIndices: readonly number[] = [],
    tolerance?: number,
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("hollow", {
      scopeId: this.#scopeId,
      shape,
      thickness,
      closingFaceIndices,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async offsetShape(shape: ShapeHandle, offset: number, tolerance?: number): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("offsetShape", { scopeId: this.#scopeId, shape, offset, ...(tolerance === undefined ? {} : { tolerance }) });
    return shapeFromKernel(this.#client, result.shape);
  }

  async offsetWire2D(wire: ShapeHandle, offset: number): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("offsetWire2D", { scopeId: this.#scopeId, wire, offset });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeEdgeLine(start: Vec3, end: Vec3): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeEdgeLine", { scopeId: this.#scopeId, start, end });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeEdgeArc(start: Vec3, point: Vec3, end: Vec3): Promise<ShapeHandle>;
  async makeEdgeArc(options: { center: Vec3; normal: Vec3; radius: number; startAngle: number; endAngle: number; xDirection?: Vec3 }): Promise<ShapeHandle>;
  async makeEdgeArc(
    startOrOptions: Vec3 | { center: Vec3; normal: Vec3; radius: number; startAngle: number; endAngle: number; xDirection?: Vec3 },
    point?: Vec3,
    end?: Vec3,
  ): Promise<ShapeHandle> {
    const args = Array.isArray(startOrOptions) ? { start: startOrOptions, point, end } : startOrOptions;
    const result = await this.#client.request<{ shape: number }>("makeEdgeArc", { scopeId: this.#scopeId, ...args });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeEdgeCircle(radius: number, options: { center?: Vec3; normal?: Vec3 } = {}): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeEdgeCircle", { scopeId: this.#scopeId, radius, ...options });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeEdgeEllipse(majorRadius: number, minorRadius: number, options: { center?: Vec3; normal?: Vec3 } = {}): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeEdgeEllipse", { scopeId: this.#scopeId, majorRadius, minorRadius, ...options });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeEdgeHyperbola(
    majorRadius: number,
    minorRadius: number,
    firstParameter: number,
    lastParameter: number,
    options: { center?: Vec3; normal?: Vec3; xDirection?: Vec3 } = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeEdgeHyperbola", {
      scopeId: this.#scopeId,
      majorRadius,
      minorRadius,
      firstParameter,
      lastParameter,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeEdgeParabola(
    focal: number,
    firstParameter: number,
    lastParameter: number,
    options: { center?: Vec3; normal?: Vec3; xDirection?: Vec3 } = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeEdgeParabola", {
      scopeId: this.#scopeId,
      focal,
      firstParameter,
      lastParameter,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeEdgeOffset(shape: ShapeHandle, offset: number, direction: Vec3): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeEdgeOffset", {
      scopeId: this.#scopeId,
      shape,
      offset,
      direction,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async approximateCurveBSpline(
    points: readonly Vec3[] | Float64Array,
    options: BSplineApproximationOptions = {},
  ): Promise<ShapeHandle> {
    const encodedPoints = points instanceof Float64Array
      ? { $inputBuffer: new Float64Array(points).buffer }
      : points;
    const result = await this.#client.request<{ shape: number }>("approximateCurveBSpline", {
      scopeId: this.#scopeId,
      points: encodedPoints,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async approximateSurfaceBSpline(
    points: readonly (readonly Vec3[])[],
    options: BSplineApproximationOptions = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("approximateSurfaceBSpline", {
      scopeId: this.#scopeId,
      points,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async extendCurve(
    shape: ShapeHandle,
    point: Vec3,
    options: { continuity?: ExtensionContinuity; after?: boolean } = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("extendCurve", {
      scopeId: this.#scopeId,
      shape,
      point,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async extendSurface(
    shape: ShapeHandle,
    length: number,
    direction: "u" | "v",
    side: "before" | "after",
    continuity: ExtensionContinuity = "c1",
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("extendSurface", {
      scopeId: this.#scopeId,
      shape,
      length,
      direction,
      side,
      continuity,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeEdgeBezier(
    poles: readonly Vec3[],
    options: { weights?: readonly number[] } = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeEdgeBezier", {
      scopeId: this.#scopeId,
      poles,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeEdgeBSpline(
    poles: readonly Vec3[] | Float64Array,
    options: {
      mode?: "interpolate" | "controlPoints";
      degree?: number;
      periodic?: boolean;
      tolerance?: number;
      knots?: readonly number[];
      multiplicities?: readonly number[];
      weights?: readonly number[];
    } = {},
  ): Promise<ShapeHandle> {
    const encodedPoles = poles instanceof Float64Array
      ? { $inputBuffer: new Float64Array(poles).buffer }
      : poles;
    const result = await this.#client.request<{ shape: number }>("makeEdgeBSpline", {
      scopeId: this.#scopeId,
      poles: encodedPoles,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeEdgeHelix(radius: number, pitch: number, turns: number, options: { origin?: Vec3; direction?: Vec3; handedness?: "right" | "left" } = {}): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeEdgeHelix", { scopeId: this.#scopeId, radius, pitch, turns, ...options });
    return shapeFromKernel(this.#client, result.shape);
  }

  async updateCurvePole(
    shape: ShapeHandle,
    index: number,
    point: Vec3,
    options: { weight?: number } = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("updateCurvePole", {
      scopeId: this.#scopeId,
      shape,
      index,
      point,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async editCurveBSpline(shape: ShapeHandle, edit: BSplineCurveEdit): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("editCurveBSpline", {
      scopeId: this.#scopeId,
      shape,
      ...edit,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async reduceCurveDegree(
    shape: ShapeHandle,
    degree: number,
    options: CurveDegreeReductionOptions = {},
  ): Promise<{ shape: ShapeHandle; maxError: number }> {
    const result = await this.#client.request<{ shape: number; maxError: number }>("reduceCurveDegree", {
      scopeId: this.#scopeId,
      shape,
      degree,
      ...options,
    });
    return { shape: shapeFromKernel(this.#client, result.shape), maxError: result.maxError };
  }

  async reparameterizeCurve(shape: ShapeHandle, first: number, last: number): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("reparameterizeCurve", {
      scopeId: this.#scopeId,
      shape,
      first,
      last,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async trimCurve(shape: ShapeHandle, first: number, last: number): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("trimCurve", {
      scopeId: this.#scopeId, shape, first, last,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async convertCurveToBSpline(
    shape: ShapeHandle,
    parameterization: "tgtThetaOver2" | "quasiAngular" | "rationalC1" | "polynomial" = "tgtThetaOver2",
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("convertCurveToBSpline", {
      scopeId: this.#scopeId, shape, parameterization,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async makeSolidFromShell(shell: ShapeHandle): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("makeSolidFromShell", { scopeId: this.#scopeId, shell });
    return shapeFromKernel(this.#client, result.shape);
  }

  async generalTransform(shape: ShapeHandle, matrix: Matrix12, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async generalTransform(shape: ShapeHandle, matrix: Matrix12, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async generalTransform(shape: ShapeHandle, matrix: Matrix12, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("generalTransform", { scopeId: this.#scopeId, shape, matrix, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("generalTransform did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async sew(shapes: readonly ShapeHandle[], tolerance?: number, options?: { includeHistory?: false }): Promise<{ shape: ShapeHandle; freeEdges: number; multipleEdges: number }>;
  async sew(shapes: readonly ShapeHandle[], tolerance: number | undefined, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; freeEdges: number; multipleEdges: number; history: History }>;
  async sew(shapes: readonly ShapeHandle[], tolerance?: number, options: { includeHistory?: boolean } = {}): Promise<{ shape: ShapeHandle; freeEdges: number; multipleEdges: number; history?: History }> {
    const result = await this.#client.request<{ shape: number; freeEdges: number; multipleEdges: number; history?: History }>("sew", {
      scopeId: this.#scopeId, shapes, ...(tolerance === undefined ? {} : { tolerance }), ...options,
    });
    const response = { shape: shapeFromKernel(this.#client, result.shape), freeEdges: result.freeEdges, multipleEdges: result.multipleEdges };
    if (!options.includeHistory) return response;
    if (result.history === undefined) throw new Error("sew did not return topology history");
    return { ...response, history: result.history };
  }

  async fixShape(shape: ShapeHandle, precision?: number, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async fixShape(shape: ShapeHandle, precision: number | undefined, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async fixShape(shape: ShapeHandle, precision?: number, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("fixShape", {
      scopeId: this.#scopeId, shape, ...(precision === undefined ? {} : { precision }), ...options,
    });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("fixShape did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async setTolerance(
    shape: ShapeHandle,
    tolerance: number,
    type: "all" | "face" | "edge" | "vertex" = "all",
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("setTolerance", {
      scopeId: this.#scopeId,
      shape,
      tolerance,
      type,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async unifySameDomain(shape: ShapeHandle, options?: { unifyEdges?: boolean; unifyFaces?: boolean; concatBSplines?: boolean; includeHistory?: false }): Promise<ShapeHandle>;
  async unifySameDomain(shape: ShapeHandle, options: { unifyEdges?: boolean; unifyFaces?: boolean; concatBSplines?: boolean; includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async unifySameDomain(shape: ShapeHandle, options: { unifyEdges?: boolean; unifyFaces?: boolean; concatBSplines?: boolean; includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("unifySameDomain", { scopeId: this.#scopeId, shape, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("unifySameDomain did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async shapeUpgrade(
    shape: ShapeHandle,
    options: ShapeUpgradeOptions = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("shapeUpgrade", {
      scopeId: this.#scopeId,
      shape,
      mode: options.mode ?? "continuity",
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async booleanFuse(base: ShapeHandle, tools: readonly ShapeHandle[], options?: { includeHistory?: false }): Promise<{ shape: ShapeHandle }>;
  async booleanFuse(base: ShapeHandle, tools: readonly ShapeHandle[], options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async booleanFuse(base: ShapeHandle, tools: readonly ShapeHandle[], options: { includeHistory?: boolean } = {}): Promise<{ shape: ShapeHandle; history?: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("booleanFuse", { scopeId: this.#scopeId, base, tools, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return { shape: resultShape };
    if (result.history === undefined) throw new Error("booleanFuse did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async booleanCommon(base: ShapeHandle, tools: readonly ShapeHandle[], options?: { includeHistory?: false }): Promise<{ shape: ShapeHandle }>;
  async booleanCommon(base: ShapeHandle, tools: readonly ShapeHandle[], options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async booleanCommon(base: ShapeHandle, tools: readonly ShapeHandle[], options: { includeHistory?: boolean } = {}): Promise<{ shape: ShapeHandle; history?: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("booleanCommon", { scopeId: this.#scopeId, base, tools, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return { shape: resultShape };
    if (result.history === undefined) throw new Error("booleanCommon did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async section(first: ShapeHandle, second: ShapeHandle): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("section", { scopeId: this.#scopeId, first, second });
    return shapeFromKernel(this.#client, result.shape);
  }

  async projectHLR(shape: ShapeHandle, options: HLRProjectionOptions): Promise<HLRProjection> {
    const result = await this.#client.request<{ visible: number; hidden: number }>("projectHLR", {
      scopeId: this.#scopeId,
      shape,
      ...options,
    });
    return {
      visible: shapeFromKernel(this.#client, result.visible),
      hidden: shapeFromKernel(this.#client, result.hidden),
    };
  }

  async sectionAnalysis(
    first: ShapeHandle,
    second: ShapeHandle,
    options: {
      approximation?: boolean;
      computePCurveOnFirst?: boolean;
      computePCurveOnSecond?: boolean;
    } = {},
  ): Promise<SectionAnalysis> {
    const result = await this.#client.request<{
      shape: number;
      edges: SectionAnalysis["edges"];
      standaloneVertices: number[];
    }>("sectionAnalysis", { scopeId: this.#scopeId, first, second, ...options });
    return { ...result, shape: shapeFromKernel(this.#client, result.shape) };
  }

  async split(
    objects: readonly ShapeHandle[],
    tools: readonly ShapeHandle[] = [],
    options: { fuzzyValue?: number; useOBB?: boolean } = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("split", {
      scopeId: this.#scopeId,
      objects,
      tools,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async generalFuse(
    shapes: readonly ShapeHandle[],
    options: { fuzzyValue?: number; glue?: "off" | "shift" | "full"; useOBB?: boolean } = {},
  ): Promise<{ shapes: ShapeHandle[]; sourceIndices: number[][] }> {
    const result = await this.#client.request<{ shapes: number[]; sourceIndices: number[][] }>("generalFuse", {
      scopeId: this.#scopeId, shapes, ...options,
    });
    return {
      shapes: result.shapes.map((shape) => shapeFromKernel(this.#client, shape)),
      sourceIndices: result.sourceIndices,
    };
  }

  async selectGeneralFuseCells(
    shapes: readonly ShapeHandle[],
    rules: readonly GeneralFuseCellRule[],
    options: GeneralFuseCellSelectionOptions = {},
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("selectGeneralFuseCells", {
      scopeId: this.#scopeId,
      shapes,
      rules,
      ...options,
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async glue(
    newShape: ShapeHandle,
    baseShape: ShapeHandle,
    faceBindings: readonly GlueBinding[],
    edgeBindings: readonly GlueBinding[] = [],
  ): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("glue", {
      scopeId: this.#scopeId,
      newShape,
      baseShape,
      faceBindings,
      ...(edgeBindings.length === 0 ? {} : { edgeBindings }),
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async intersectSurfaceSurface(first: ShapeHandle, second: ShapeHandle, tolerance = 1e-7): Promise<ShapeHandle[]> {
    const result = await this.#client.request<{ shapes: number[] }>("intersectSurfaceSurface", {
      scopeId: this.#scopeId,
      first,
      second,
      tolerance,
    });
    return result.shapes.map((value) => shapeFromKernel(this.#client, value));
  }

  async intersectCurveCurve(first: ShapeHandle, second: ShapeHandle, tolerance = 1e-7): Promise<ShapeHandle[]> {
    const result = await this.#client.request<{ shapes: number[] }>("intersectCurveCurve", {
      scopeId: this.#scopeId,
      first,
      second,
      tolerance,
    });
    return result.shapes.map((value) => shapeFromKernel(this.#client, value));
  }

  async transform(shape: ShapeHandle, options?: { matrix?: Matrix12; translation?: Vec3; rotation?: { origin?: Vec3; direction?: Vec3; angle: number }; scale?: number; scaleOrigin?: Vec3; mirror?: Vec3; copy?: boolean; includeHistory?: false }): Promise<ShapeHandle>;
  async transform(shape: ShapeHandle, options: { matrix?: Matrix12; translation?: Vec3; rotation?: { origin?: Vec3; direction?: Vec3; angle: number }; scale?: number; scaleOrigin?: Vec3; mirror?: Vec3; copy?: boolean; includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async transform(shape: ShapeHandle, options: { matrix?: Matrix12; translation?: Vec3; rotation?: { origin?: Vec3; direction?: Vec3; angle: number }; scale?: number; scaleOrigin?: Vec3; mirror?: Vec3; copy?: boolean; includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("transform", { scopeId: this.#scopeId, shape, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("transform did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async batchTransformCopy(
    shape: ShapeHandle,
    options: { mode: "linear"; count: number; translation: Vec3 } | {
      mode: "circular"; count: number; angle: number; direction: Vec3; origin?: Vec3;
    },
  ): Promise<ShapeHandle[]> {
    const result = await this.#client.request<{ shapes: number[] }>("batchTransformCopy", {
      scopeId: this.#scopeId,
      shape,
      ...options,
    });
    return result.shapes.map((value) => shapeFromKernel(this.#client, value));
  }

  async translate(shape: ShapeHandle, translation: Vec3, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async translate(shape: ShapeHandle, translation: Vec3, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async translate(shape: ShapeHandle, translation: Vec3, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("translate", { scopeId: this.#scopeId, shape, translation, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("translate did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async rotate(shape: ShapeHandle, rotation: { origin?: Vec3; direction?: Vec3; angle: number }, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async rotate(shape: ShapeHandle, rotation: { origin?: Vec3; direction?: Vec3; angle: number }, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async rotate(shape: ShapeHandle, rotation: { origin?: Vec3; direction?: Vec3; angle: number }, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("rotate", { scopeId: this.#scopeId, shape, rotation, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("rotate did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async scale(shape: ShapeHandle, factor: number, origin?: Vec3, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async scale(shape: ShapeHandle, factor: number, origin: Vec3 | undefined, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async scale(shape: ShapeHandle, factor: number, origin?: Vec3, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("scale", { scopeId: this.#scopeId, shape, factor, ...(origin === undefined ? {} : { origin }), ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("scale did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async mirror(shape: ShapeHandle, normal: Vec3, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async mirror(shape: ShapeHandle, normal: Vec3, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async mirror(shape: ShapeHandle, normal: Vec3, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("mirror", { scopeId: this.#scopeId, shape, normal, ...options });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("mirror did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async booleanCut(
    base: ShapeHandle,
    tools: readonly ShapeHandle[],
    options?: { includeHistory?: false },
  ): Promise<{ shape: ShapeHandle }>;
  async booleanCut(
    base: ShapeHandle,
    tools: readonly ShapeHandle[],
    options: { includeHistory: true },
  ): Promise<{ shape: ShapeHandle; history: History }>;
  async booleanCut(
    base: ShapeHandle,
    tools: readonly ShapeHandle[],
    options: { includeHistory?: boolean } = {},
  ): Promise<{ shape: ShapeHandle; history?: History }> {
    const result = await this.#client.request<{ shape: number; history?: History }>("booleanCut", {
      scopeId: this.#scopeId,
      base,
      tools,
      ...options,
    });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeHistory) return { shape: resultShape };
    if (result.history === undefined) throw new Error("booleanCut did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async importBREP(data: BinaryBuffer): Promise<ShapeHandle> {
    const result = await this.#client.request<{ shape: number }>("importBREP", {
      scopeId: this.#scopeId,
      data: { $inputBuffer: data },
    });
    return shapeFromKernel(this.#client, result.shape);
  }

  async importIGES(
    data: BinaryBuffer,
    options: IGESImportOptions = {},
  ): Promise<MultiRootShapeImport> {
    const result = await this.#client.request<{ shape: number; shapes: number[]; rootCount: number }>("importIGES", {
      scopeId: this.#scopeId,
      data: { $inputBuffer: data },
      ...options,
    });
    return {
      shape: shapeFromKernel(this.#client, result.shape),
      shapes: result.shapes.map((shape) => shapeFromKernel(this.#client, shape)),
      rootCount: result.rootCount,
    };
  }

  async importIGESDocument(
    data: BinaryBuffer,
    options: IGESImportOptions = {},
  ): Promise<STEPDocument> {
    const result = await this.#client.request<{
      shape: number;
      rootCount: number;
      roots: number[];
      nodes: Array<Omit<STEPDocument["nodes"][number], "shape"> & { shape: number }>;
      gdt: Array<RawAnnotation<STEPDocument["gdt"][number]>>;
      datums: Array<RawAnnotation<STEPDocument["datums"][number]>>;
      geometricTolerances: Array<RawAnnotation<STEPDocument["geometricTolerances"][number]>>;
      views: STEPDocument["views"];
      shuo: STEPDocument["shuo"];
    }>("importIGESDocument", {
      scopeId: this.#scopeId,
      data: { $inputBuffer: data },
      ...options,
    });
    return {
      shape: shapeFromKernel(this.#client, result.shape),
      rootCount: result.rootCount,
      roots: result.roots,
      gdt: materializeAnnotationPresentations(this.#client, result.gdt),
      datums: materializeAnnotationPresentations(this.#client, result.datums),
      geometricTolerances: materializeAnnotationPresentations(this.#client, result.geometricTolerances),
      views: result.views,
      shuo: result.shuo,
      nodes: result.nodes.map((node) => ({
        ...node,
        shape: shapeFromKernel(this.#client, node.shape),
      })),
    };
  }

  async importGLTF(data: BinaryBuffer): Promise<ShapeHandle>;
  async importGLTF(
    data: BinaryBuffer,
    options: GLTFImportOptions & { includeDocument?: false },
  ): Promise<ShapeHandle>;
  async importGLTF(
    data: BinaryBuffer,
    options: GLTFImportOptions & { includeDocument: true },
  ): Promise<GLTFDocumentImport>;
  async importGLTF(
    data: BinaryBuffer,
    options: GLTFImportOptions = {},
  ): Promise<ShapeHandle | GLTFDocumentImport> {
    const decoded = await decodeGLTFCompression(data, options.resources);
    const importData = decoded?.data ?? data;
    const resources = options.resources === undefined
      ? undefined
      : Object.entries(options.resources).map(([uri, value]) => ({ uri, data: { $inputBuffer: value } }));
    const result = await this.#client.request<{
      shape?: number;
      positions?: MaterializedBuffer;
      indices?: MaterializedBuffer;
      normals?: MaterializedBuffer;
      uvs?: MaterializedBuffer;
      colors?: MaterializedBuffer;
      document?: GLTFDocumentImport["document"];
      activeScene?: number;
      sceneRoots?: number[];
      primitives?: GLTFDocumentImport["primitives"];
      buffers?: Array<{ uri?: string; data: MaterializedBuffer }>;
      resources?: Array<{ uri: string; data: MaterializedBuffer }>;
    }>("importGLTF", {
      scopeId: this.#scopeId,
      data: { $inputBuffer: importData },
      ...(options.includeDocument === undefined ? {} : { includeDocument: options.includeDocument }),
      ...(resources === undefined ? {} : { resources }),
      ...(options.morphWeights === undefined ? {} : { morphWeights: options.morphWeights }),
      ...(options.animationIndex === undefined ? {} : { animationIndex: options.animationIndex }),
      ...(options.animationTime === undefined ? {} : { animationTime: options.animationTime }),
    });
    if (!options.includeDocument) {
      if (result.shape === undefined) throw new Error("glTF shape import did not return a shape");
      return shapeFromKernel(this.#client, result.shape);
    }
    if (result.positions === undefined || result.indices === undefined || result.document === undefined
        || result.sceneRoots === undefined || result.primitives === undefined
        || result.buffers === undefined || result.resources === undefined) {
      throw new Error("glTF document import did not return required data");
    }
    return {
      ...(result.shape === undefined ? {} : { shape: shapeFromKernel(this.#client, result.shape) }),
      positions: new Float32Array(result.positions.data),
      indices: new Uint32Array(result.indices.data),
      ...(result.normals === undefined ? {} : { normals: new Float32Array(result.normals.data) }),
      ...(result.uvs === undefined ? {} : { uvs: new Float32Array(result.uvs.data) }),
      document: decoded?.document ?? result.document,
      ...(result.activeScene === undefined ? {} : { activeScene: result.activeScene }),
      sceneRoots: result.sceneRoots,
      primitives: result.primitives,
      buffers: decoded?.buffers ?? result.buffers.map((buffer) => ({
        ...(buffer.uri === undefined ? {} : { uri: buffer.uri }),
        data: buffer.data.data,
      })),
      resources: result.resources.map((resource) => ({ uri: resource.uri, data: resource.data.data })),
    };
  }

  async importOBJ(data: BinaryBuffer): Promise<ShapeHandle>;
  async importOBJ(
    data: BinaryBuffer,
    options: OBJImportOptions & { includeDocument: true },
  ): Promise<OBJMeshImport>;
  async importOBJ(
    data: BinaryBuffer,
    options: OBJImportOptions = {},
  ): Promise<ShapeHandle | OBJMeshImport> {
    const resources = options.resources === undefined
      ? undefined
      : Object.entries(options.resources).map(([uri, value]) => ({ uri, data: { $inputBuffer: value } }));
    const result = await this.#client.request<{
      shape: number;
      positions?: MaterializedBuffer;
      indices?: MaterializedBuffer;
      normals?: MaterializedBuffer;
      uvs?: MaterializedBuffer;
      document?: OBJMeshImport["document"];
    }>("importOBJ", {
      scopeId: this.#scopeId,
      data: { $inputBuffer: data },
      ...(options.includeDocument === undefined ? {} : { includeDocument: options.includeDocument }),
      ...(resources === undefined ? {} : { resources }),
    });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeDocument) return resultShape;
    if (result.positions === undefined || result.indices === undefined || result.document === undefined) {
      throw new Error("OBJ document import did not return required mesh data");
    }
    return {
      shape: resultShape,
      positions: new Float32Array(result.positions.data),
      indices: new Uint32Array(result.indices.data),
      ...(result.normals === undefined ? {} : { normals: new Float32Array(result.normals.data) }),
      ...(result.uvs === undefined ? {} : { uvs: new Float32Array(result.uvs.data) }),
      document: result.document,
    };
  }

  async importPLY(data: BinaryBuffer): Promise<ShapeHandle>;
  async importPLY(data: BinaryBuffer, options: PLYImportOptions & { includeMesh: true }): Promise<PLYMeshImport>;
  async importPLY(data: BinaryBuffer, options: PLYImportOptions = {}): Promise<ShapeHandle | PLYMeshImport> {
    const result = await this.#client.request<{
      shape: number;
      positions?: MaterializedBuffer;
      indices?: MaterializedBuffer;
      normals?: MaterializedBuffer;
      uvs?: MaterializedBuffer;
      colors?: MaterializedBuffer;
      document?: { comments: string[]; objectInfo: string[] };
    }>("importPLY", {
      scopeId: this.#scopeId,
      data: { $inputBuffer: data },
      ...options,
    });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeMesh) return resultShape;
    if (result.positions === undefined || result.indices === undefined || result.document === undefined)
      throw new Error("PLY mesh import did not return required mesh buffers");
    return {
      shape: resultShape,
      positions: new Float32Array(result.positions.data),
      indices: new Uint32Array(result.indices.data),
      ...(result.normals === undefined ? {} : { normals: new Float32Array(result.normals.data) }),
      ...(result.uvs === undefined ? {} : { uvs: new Float32Array(result.uvs.data) }),
      ...(result.colors === undefined ? {} : { colors: new Uint8Array(result.colors.data) }),
      document: result.document,
    };
  }

  async importSTEP(
    data: BinaryBuffer,
    options: STEPImportOptions = {},
  ): Promise<MultiRootShapeImport> {
    const result = await this.#client.request<{ shape: number; shapes: number[]; rootCount: number }>("importSTEP", {
      scopeId: this.#scopeId,
      data: { $inputBuffer: data },
      ...options,
    });
    return {
      shape: shapeFromKernel(this.#client, result.shape),
      shapes: result.shapes.map((shape) => shapeFromKernel(this.#client, shape)),
      rootCount: result.rootCount,
    };
  }

  async importSTEPDocument(
    data: BinaryBuffer,
    options: STEPImportOptions = {},
  ): Promise<STEPDocument> {
    const result = await this.#client.request<{
      shape: number;
      rootCount: number;
      roots: number[];
      nodes: Array<Omit<STEPDocument["nodes"][number], "shape"> & { shape: number }>;
      gdt: Array<RawAnnotation<STEPDocument["gdt"][number]>>;
      datums: Array<RawAnnotation<STEPDocument["datums"][number]>>;
      geometricTolerances: Array<RawAnnotation<STEPDocument["geometricTolerances"][number]>>;
      views: STEPDocument["views"];
      shuo: STEPDocument["shuo"];
    }>("importSTEPDocument", {
      scopeId: this.#scopeId,
      data: { $inputBuffer: data },
      ...options,
    });
    return {
      shape: shapeFromKernel(this.#client, result.shape),
      rootCount: result.rootCount,
      roots: result.roots,
      gdt: materializeAnnotationPresentations(this.#client, result.gdt),
      datums: materializeAnnotationPresentations(this.#client, result.datums),
      geometricTolerances: materializeAnnotationPresentations(this.#client, result.geometricTolerances),
      views: result.views,
      shuo: result.shuo,
      nodes: result.nodes.map((node) => ({
        ...node,
        shape: shapeFromKernel(this.#client, node.shape),
      })),
    };
  }

  async importXCAF(
    data: BinaryBuffer,
    format: XCAFDocumentFormat = "bin",
  ): Promise<STEPDocument> {
    const result = await this.#client.request<{
      shape: number;
      rootCount: number;
      roots: number[];
      nodes: Array<Omit<STEPDocument["nodes"][number], "shape"> & { shape: number }>;
      gdt: Array<RawAnnotation<STEPDocument["gdt"][number]>>;
      datums: Array<RawAnnotation<STEPDocument["datums"][number]>>;
      geometricTolerances: Array<RawAnnotation<STEPDocument["geometricTolerances"][number]>>;
      views: STEPDocument["views"];
      shuo: STEPDocument["shuo"];
    }>("importXCAF", {
      scopeId: this.#scopeId,
      data: { $inputBuffer: data },
      format,
    });
    return {
      shape: shapeFromKernel(this.#client, result.shape),
      rootCount: result.rootCount,
      roots: result.roots,
      gdt: materializeAnnotationPresentations(this.#client, result.gdt),
      datums: materializeAnnotationPresentations(this.#client, result.datums),
      geometricTolerances: materializeAnnotationPresentations(this.#client, result.geometricTolerances),
      views: result.views,
      shuo: result.shuo,
      nodes: result.nodes.map((node) => ({
        ...node,
        shape: shapeFromKernel(this.#client, node.shape),
      })),
    };
  }

  async importSTL(data: BinaryBuffer): Promise<ShapeHandle>;
  async importSTL(data: BinaryBuffer, options: STLImportOptions & { includeMesh: true }): Promise<STLMeshImport>;
  async importSTL(
    data: BinaryBuffer,
    options: STLImportOptions = {},
  ): Promise<ShapeHandle | STLMeshImport> {
    const result = await this.#client.request<{
      shape: number;
      positions?: MaterializedBuffer;
      indices?: MaterializedBuffer;
      normals?: MaterializedBuffer;
      solidName?: string;
      binaryHeader?: MaterializedBuffer;
    }>("importSTL", {
      scopeId: this.#scopeId,
      data: { $inputBuffer: data },
      ...options,
    });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeMesh) return resultShape;
    if (result.positions === undefined || result.indices === undefined || result.normals === undefined)
      throw new Error("STL mesh import did not return required mesh buffers");
    return {
      shape: resultShape,
      positions: new Float32Array(result.positions.data),
      indices: new Uint32Array(result.indices.data),
      normals: new Float32Array(result.normals.data),
      ...(result.solidName === undefined ? {} : { solidName: result.solidName }),
      ...(result.binaryHeader === undefined
        ? {}
        : { binaryHeader: new Uint8Array(result.binaryHeader.data) }),
    };
  }

  async importVRML(data: BinaryBuffer): Promise<ShapeHandle>;
  async importVRML(data: BinaryBuffer, options: VRMLImportOptions & { includeMesh: true }): Promise<VRMLMeshImport>;
  async importVRML(
    data: BinaryBuffer,
    options: VRMLImportOptions = {},
  ): Promise<ShapeHandle | VRMLMeshImport> {
    const result = await this.#client.request<{
      shape: number;
      positions?: MaterializedBuffer;
      indices?: MaterializedBuffer;
      normals?: MaterializedBuffer;
      uvs?: MaterializedBuffer;
      colors?: MaterializedBuffer;
    }>("importVRML", {
      scopeId: this.#scopeId,
      data: { $inputBuffer: data },
      ...options,
    });
    const resultShape = shapeFromKernel(this.#client, result.shape);
    if (!options.includeMesh) return resultShape;
    if (result.positions === undefined || result.indices === undefined || result.normals === undefined)
      throw new Error("VRML mesh import did not return required mesh buffers");
    const mesh: VRMLMeshImport = {
      shape: resultShape,
      positions: new Float32Array(result.positions.data),
      indices: new Uint32Array(result.indices.data),
      normals: new Float32Array(result.normals.data),
    };
    if (result.uvs !== undefined) mesh.uvs = new Float32Array(result.uvs.data);
    if (result.colors !== undefined) mesh.colors = new Uint8Array(result.colors.data);
    return mesh;
  }


  async end(): Promise<void> {
    if (this.#endPromise === undefined) {
      this.#endPromise = this.#client.request("endScope", { scopeId: this.#scopeId }).then(() => undefined);
      void this.#endPromise.catch(() => {
        this.#endPromise = undefined;
      });
    }
    await this.#endPromise;
  }

  async [Symbol.asyncDispose](): Promise<void> {
    await this.end();
  }
}
