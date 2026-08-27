import { PROTOCOL_VERSION, type OperationName } from "./generated.js";
import { ShapeScope } from "./shape-scope.js";
import { mapProtocolValue } from "./protocol-codec.js";
import {
  allocateClientEpoch,
  ScopeHandle,
  ShapeHandle,
  scopeFromKernel,
  type InputBufferDescriptor,
  type ProtocolRequestArgs,
  type ProtocolResult,
} from "./client-contract.js";
export {
  ScopeHandle,
  ShapeHandle,
  type InputBufferDescriptor,
  type AnnotationWithPresentation,
  type KernelShapeToken,
  type RawAnnotation,
} from "./client-contract.js";
import {
  KernelError,
  type Adjacency,
  type BBox,
  type BinaryBuffer,
  type Capabilities,
  type Continuity,
  type CurveControlData,
  type CurveCurveExtrema,
  type CurveDomain,
  type CurveEvaluation,
  type CurveGeometry,
  type CurveProjection,
  type CurveSurfaceExtrema,
  type CurveSurfaceIntersection,
  type DistanceResult,
  type EdgeTessellation,
  type FormatProbe,
  type GLTFDocumentData,
  type GLTFDocumentExport,
  type GLTFExportOptions,
  type IGESExportOptions,
  type KernelErrorData,
  type KernelStats,
  type MassProperties,
  type MaterializedBuffer,
  type OBJExportOptions,
  type OBJMeshData,
  type OBJMeshExport,
  type OBJMeshExportOptions,
  type OrientedBBox,
  type PLYExportOptions,
  type PLYMeshData,
  type RequestOptions,
  type ShapeDiagnostics,
  type STEPDocumentDefinition,
  type STEPExportOptions,
  type STLExportOptions,
  type STLMeshData,
  type STLMeshExportOptions,
  type SurfaceControlData,
  type SurfaceDomain,
  type SurfaceEvaluation,
  type SurfaceGeometry,
  type SurfaceProjection,
  type SurfaceSurfaceExtrema,
  type Tessellation,
  type TopologyCounts,
  type TriangulationValidation,
  type Vec3,
  type VRMLExportOptions,
  type VRMLMeshData,
  type XCAFDocumentFormat,
} from "./types.js";

export function inputBuffer(value: BinaryBuffer | ArrayBufferView): InputBufferDescriptor {
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
  epoch = allocateClientEpoch();

  protected abstract send<T>(
    op: OperationName,
    args: Record<string, unknown>,
    options?: RequestOptions,
  ): Promise<T>;

  protected invalidateEpoch(): void {
    this.epoch = allocateClientEpoch();
  }

  protected validateCapabilities(capabilities: Capabilities): Capabilities {
    if (capabilities.protocolVersion !== PROTOCOL_VERSION) {
      throw new Error(
        `Unsupported kernel protocol ${capabilities.protocolVersion}; client requires ${PROTOCOL_VERSION}`,
      );
    }
    return capabilities;
  }

  async initialize(): Promise<Capabilities> {
    const capabilities = await this.requestUnsafe<Capabilities>("capabilities", {});
    return this.validateCapabilities(capabilities);
  }

  async request<K extends OperationName>(
    op: K,
    args: ProtocolRequestArgs<K>,
    timeoutOrOptions?: number | RequestOptions,
  ): Promise<ProtocolResult<K>> {
    return this.requestUnsafe<ProtocolResult<K>>(op, args, timeoutOrOptions);
  }

  async requestUnsafe<T>(
    op: OperationName,
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
      return value;
    };
    const options = typeof timeoutOrOptions === "number"
      ? { timeoutMs: timeoutOrOptions }
      : timeoutOrOptions;
    if (options?.timeoutMs !== undefined && (!Number.isFinite(options.timeoutMs) || options.timeoutMs < 0)) {
      throw new TypeError("timeoutMs must be a non-negative finite number");
    }
    if (options?.outputBuffers === "shared" && typeof SharedArrayBuffer === "undefined") {
      throw new TypeError("SharedArrayBuffer is not available in this host");
    }
    if (options?.signal?.aborted) {
      throw options.signal.reason ?? new DOMException("Kernel request aborted", "AbortError");
    }
    return this.send<T>(op, mapProtocolValue(args, encode) as Record<string, unknown>, options);
  }

  async beginScope(): Promise<ShapeScope> {
    const result = await this.request("beginScope", {});
    return new ShapeScope(this, scopeFromKernel(this, result.scopeId));
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
    return this.requestUnsafe<OrientedBBox>("obb", { shape, ...options });
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
    return this.requestUnsafe<MassProperties>("massProps", { shape, kind });
  }

  async topologyCounts(shape: ShapeHandle): Promise<TopologyCounts> {
    return this.request("topologyCounts", { shape });
  }

  async isValid(shape: ShapeHandle): Promise<boolean> {
    return (await this.request("isValid", { shape })).valid;
  }

  async distance(first: ShapeHandle, second: ShapeHandle): Promise<DistanceResult> {
    return this.request("distance", { first, second });
  }

  async classifyPoint(shape: ShapeHandle, point: Vec3, tolerance?: number): Promise<"inside" | "outside" | "on" | "unknown"> {
    const result = await this.request("classifyPoint", {
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
    return this.requestUnsafe<Adjacency>("getAdjacency", { shape, from, to });
  }

  async shapeType(shape: ShapeHandle): Promise<string> {
    return (await this.request("shapeType", { shape })).type;
  }

  async getSubShapes(shape: ShapeHandle, type: "compound" | "compsolid" | "solid" | "shell" | "face" | "wire" | "edge" | "vertex" = "face"): Promise<Array<{ type: string; index: number }>> {
    return (await this.request("getSubShapes", { shape, type })).shapes;
  }

  async curveDomain(shape: ShapeHandle): Promise<CurveDomain> {
    return this.request("curveDomain", { shape });
  }

  async curveGeometry(shape: ShapeHandle): Promise<CurveGeometry> {
    return this.requestUnsafe<CurveGeometry>("curveGeometry", { shape });
  }

  async curveControlData(shape: ShapeHandle): Promise<CurveControlData> {
    return this.request("curveControlData", { shape });
  }

  async curveContinuity(shape: ShapeHandle): Promise<Continuity> {
    return (await this.request("curveContinuity", { shape })).continuity;
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
    const result = await this.request("surfaceContinuity", { shape });
    return { u: result.uContinuity, v: result.vContinuity };
  }

  async surfaceGeometry(shape: ShapeHandle): Promise<SurfaceGeometry> {
    return this.requestUnsafe<SurfaceGeometry>("surfaceGeometry", { shape });
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
    return (await this.request("isSameShape", { first, second })).same;
  }

  async tessellate(
    shape: ShapeHandle,
    options: { linearDeflection?: number; angularDeflection?: number; relative?: boolean; includeUV?: boolean } = {},
  ): Promise<Tessellation> {
    const result = await this.request("tessellate", { shape, ...options });
    return {
      positions: new Float32Array(result.positions!.data),
      normals: new Float32Array(result.normals!.data),
      indices: new Uint32Array(result.indices!.data),
      faceGroups: new Uint32Array(result.faceGroups!.data),
      ...(result.uvs === undefined ? {} : { uvs: new Float32Array(result.uvs.data) }),
    };
  }

  async triangulationData(shape: ShapeHandle, options: { includeUV?: boolean } = {}): Promise<Tessellation> {
    const result = await this.request("triangulationData", { shape, ...options });
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
    const result = await this.request("tessellateEdges", { shape, ...options });
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
    const { binaryHeader, ...restOptions } = options;
    const encodedOptions = {
      ...restOptions,
      ...(binaryHeader === undefined
        ? {}
        : { binaryHeader: inputBuffer(binaryHeader) }),
    };
    let args: ProtocolRequestArgs<"exportSTL">;
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
    const result = await this.request("exportSTL", args);
    return result.data.data;
  }

  async exportBREP(shape: ShapeHandle): Promise<ArrayBuffer> {
    const result = await this.request("exportBREP", { shape });
    return result.data.data;
  }

  async exportIGES(
    shape: ShapeHandle | readonly ShapeHandle[],
    options: IGESExportOptions = {},
  ): Promise<ArrayBuffer> {
    const result = await this.request("exportIGES", {
      ...(shape instanceof ShapeHandle ? { shape } : { shapes: shape }),
      ...options,
    });
    return result.data.data;
  }

  async exportIGESDocument(
    document: STEPDocumentDefinition,
    options: Pick<IGESExportOptions, "unit"> = {},
  ): Promise<ArrayBuffer> {
    const result = await this.request("exportIGESDocument", {
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
      const result = await this.request("exportGLTF", { shape: source, ...options });
      return result.data.data;
    }
    const result = await this.request("exportGLTF", {
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
    if (!("resources" in result)) throw new Error("glTF document export did not return resources");
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
      const result = await this.request("exportOBJ", { shape, ...options });
      return result.data.data;
    }
    const meshOptions = options as OBJMeshExportOptions;
    const result = await this.request("exportOBJ", {
      positions: inputBuffer(shape.positions),
      indices: inputBuffer(shape.indices),
      ...(shape.normals === undefined ? {} : { normals: inputBuffer(shape.normals) }),
      ...(shape.uvs === undefined ? {} : { uvs: inputBuffer(shape.uvs) }),
      ...(shape.document === undefined ? {} : { document: shape.document }),
      ...(meshOptions.materialLibrary === undefined
        ? {}
        : { materialLibrary: meshOptions.materialLibrary }),
    });
    if (!("resources" in result)) throw new Error("OBJ mesh export did not return resources");
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
    let args: ProtocolRequestArgs<"exportPLY">;
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
    const result = await this.request("exportPLY", args);
    return result.data.data;
  }

  async exportSTEP(
    shape: ShapeHandle | readonly ShapeHandle[],
    options: STEPExportOptions = {},
  ): Promise<ArrayBuffer> {
    const result = await this.request("exportSTEP", {
      ...(shape instanceof ShapeHandle ? { shape } : { shapes: shape }),
      ...options,
    });
    return result.data.data;
  }

  async exportSTEPDocument(
    document: STEPDocumentDefinition,
    options: STEPExportOptions = {},
  ): Promise<ArrayBuffer> {
    const result = await this.request("exportSTEPDocument", {
      ...document,
      ...options,
    });
    return result.data.data;
  }

  async exportXCAF(
    document: STEPDocumentDefinition,
    format: XCAFDocumentFormat = "bin",
  ): Promise<ArrayBuffer> {
    const result = await this.request("exportXCAF", {
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
    let args: ProtocolRequestArgs<"exportVRML">;
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
    const result = await this.request("exportVRML", args);
    return result.data.data;
  }

  async probeFormat(data: BinaryBuffer): Promise<FormatProbe> {
    return this.request("probeFormat", { data: { $inputBuffer: data } });
  }


  kernelError(error: KernelErrorData): never {
    throw new KernelError(error);
  }
}

export { ShapeScope };
