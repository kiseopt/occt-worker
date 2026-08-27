import {
  shapeFromKernel,
  type ClientRequestOwner,
  type KernelShapeToken,
  type ScopeHandle,
  type ShapeHandle,
} from "./client-contract.js";
import { OPERATION_RESULT_HANDLE_PATHS } from "./generated.js";
import { mapShapeHandles } from "./protocol-codec.js";
import type {
  AnalyticSurfaceDefinition,
  BatchOperation,
  BatchResult,
  BSplineSurfaceEdit,
  BSplineSurfaceOptions,
  SurfaceBounds,
  SurfaceDegreeReductionOptions,
  SurfaceFillingOptions,
  TriangulationRepairResult,
  TriangulationReplacement,
  Vec3,
} from "./types.js";
export class ShapeScopeCore {
  protected readonly client: ClientRequestOwner;
  protected readonly scope: ScopeHandle;

  constructor(client: ClientRequestOwner, scopeId: ScopeHandle) {
    this.client = client;
    this.scope = scopeId;
  }

  get scopeId(): number {
    return this.scope.encode(this.client);
  }

  async batch(operations: readonly BatchOperation[]): Promise<BatchResult> {
    const result = await this.client.requestUnsafe<{
      results: Array<Record<string, unknown>>;
      error?: BatchResult["error"];
    }>("batch", { scopeId: this.scope, ops: operations });
    return {
      results: result.results.map((entry, index) => mapShapeHandles(
        entry,
        OPERATION_RESULT_HANDLE_PATHS[operations[index]!.op],
        (handle) => shapeFromKernel(this.client, handle),
      ) as BatchResult["results"][number]),
      ...(result.error === undefined ? {} : { error: result.error }),
    };
  }

  async makeBox(size: Vec3, origin: Vec3 = [0, 0, 0]): Promise<ShapeHandle> {
    const result = await this.client.request("makeBox", {
      scopeId: this.scope,
      size,
      origin,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeCylinder(
    radius: number,
    height: number,
    options: { origin?: Vec3; direction?: Vec3 } = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("makeCylinder", {
      scopeId: this.scope,
      radius,
      height,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeSphere(radius: number, options: { origin?: Vec3; direction?: Vec3 } = {}): Promise<ShapeHandle> {
    const result = await this.client.request("makeSphere", {
      scopeId: this.scope, radius, ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeCone(radius1: number, radius2: number, height: number, options: { origin?: Vec3; direction?: Vec3 } = {}): Promise<ShapeHandle> {
    const result = await this.client.request("makeCone", {
      scopeId: this.scope, radius1, radius2, height, ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeTorus(majorRadius: number, minorRadius: number, options: { origin?: Vec3; direction?: Vec3 } = {}): Promise<ShapeHandle> {
    const result = await this.client.request("makeTorus", {
      scopeId: this.scope, majorRadius, minorRadius, ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeWedge(
    size: Vec3,
    ltx: number,
    options: { origin?: Vec3; direction?: Vec3 } = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("makeWedge", {
      scopeId: this.scope,
      size,
      ltx,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeHalfSpace(face: ShapeHandle, referencePoint: Vec3): Promise<ShapeHandle> {
    const result = await this.client.request("makeHalfSpace", {
      scopeId: this.scope,
      face,
      referencePoint,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeVertex(point: Vec3): Promise<ShapeHandle> {
    const result = await this.client.request("makeVertex", { scopeId: this.scope, point });
    return shapeFromKernel(this.client, result.shape);
  }

  async getSubShape(
    shape: ShapeHandle,
    type: "compound" | "compsolid" | "solid" | "shell" | "face" | "wire" | "edge" | "vertex",
    index: number,
  ): Promise<ShapeHandle> {
    const result = await this.client.request("getSubShape", {
      scopeId: this.scope, shape, type, index,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makePolygon(points: readonly Vec3[], close = true): Promise<ShapeHandle> {
    const result = await this.client.request("makePolygon", { scopeId: this.scope, points, close });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeWire(edges: readonly ShapeHandle[]): Promise<ShapeHandle> {
    const result = await this.client.request("makeWire", { scopeId: this.scope, edges });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeFace(outer: ShapeHandle, holes: readonly ShapeHandle[] = []): Promise<ShapeHandle> {
    const result = await this.client.request("makeFace", { scopeId: this.scope, outer, holes });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeFaceOnSurface(
    surface: ShapeHandle,
    outer: ShapeHandle,
    holes: readonly ShapeHandle[] = [],
    tolerance?: number,
  ): Promise<ShapeHandle> {
    const result = await this.client.request("makeFaceOnSurface", {
      scopeId: this.scope,
      surface,
      outer,
      holes,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeCompound(shapes: readonly ShapeHandle[]): Promise<ShapeHandle> {
    const result = await this.client.request("makeCompound", {
      scopeId: this.scope,
      shapes,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeShell(faces: readonly ShapeHandle[]): Promise<ShapeHandle> {
    const result = await this.client.request("makeShell", {
      scopeId: this.scope,
      faces,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeCompSolid(solids: readonly ShapeHandle[]): Promise<ShapeHandle> {
    const result = await this.client.request("makeCompSolid", {
      scopeId: this.scope,
      solids,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeSurfaceFace(
    surface: AnalyticSurfaceDefinition,
    bounds: SurfaceBounds,
    tolerance?: number,
  ): Promise<ShapeHandle> {
    const result = await this.client.requestUnsafe<{ shape: KernelShapeToken }>("makeSurfaceFace", {
      scopeId: this.scope,
      surface,
      ...bounds,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeSurfaceBezier(
    poles: readonly (readonly Vec3[])[],
    options: { weights?: readonly (readonly number[])[]; tolerance?: number } = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("makeSurfaceBezier", {
      scopeId: this.scope,
      poles,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeSurfaceBSpline(
    poles: readonly (readonly Vec3[])[],
    options: BSplineSurfaceOptions = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("makeSurfaceBSpline", {
      scopeId: this.scope,
      poles,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeSurfaceExtrusion(
    profile: ShapeHandle,
    direction: Vec3,
    vFirst: number,
    vLast: number,
    tolerance?: number,
  ): Promise<ShapeHandle> {
    const result = await this.client.request("makeSurfaceExtrusion", {
      scopeId: this.scope,
      profile,
      direction,
      vFirst,
      vLast,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeSurfaceRevolution(
    profile: ShapeHandle,
    options: { origin?: Vec3; direction?: Vec3; angle?: number; tolerance?: number } = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("makeSurfaceRevolution", {
      scopeId: this.scope,
      profile,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeSurfaceRuled(first: ShapeHandle, second: ShapeHandle): Promise<ShapeHandle> {
    const result = await this.client.request("makeSurfaceRuled", {
      scopeId: this.scope,
      first,
      second,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeSurfaceOffset(shape: ShapeHandle, offset: number, tolerance?: number): Promise<ShapeHandle> {
    const result = await this.client.request("makeSurfaceOffset", {
      scopeId: this.scope,
      shape,
      offset,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeSurfaceFilling(
    boundary: ShapeHandle,
    options: SurfaceFillingOptions = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("makeSurfaceFilling", {
      scopeId: this.scope,
      boundary,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async updateSurfacePole(
    shape: ShapeHandle,
    uIndex: number,
    vIndex: number,
    point: Vec3,
    options: { weight?: number; tolerance?: number } = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("updateSurfacePole", {
      scopeId: this.scope,
      shape,
      uIndex,
      vIndex,
      point,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async editSurfaceBSpline(shape: ShapeHandle, edit: BSplineSurfaceEdit): Promise<ShapeHandle> {
    const result = await this.client.request("editSurfaceBSpline", {
      scopeId: this.scope,
      shape,
      ...edit,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async reduceSurfaceDegree(
    shape: ShapeHandle,
    uDegree: number,
    vDegree: number,
    options: SurfaceDegreeReductionOptions = {},
  ): Promise<{ shape: ShapeHandle; maxError: number }> {
    const result = await this.client.request("reduceSurfaceDegree", {
      scopeId: this.scope,
      shape,
      uDegree,
      vDegree,
      ...options,
    });
    return { shape: shapeFromKernel(this.client, result.shape), maxError: result.maxError };
  }

  async reparameterizeSurface(
    shape: ShapeHandle,
    bounds: SurfaceBounds,
    tolerance?: number,
  ): Promise<ShapeHandle> {
    const result = await this.client.request("reparameterizeSurface", {
      scopeId: this.scope,
      shape,
      ...bounds,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async replaceTriangulation(shape: ShapeHandle, mesh: TriangulationReplacement): Promise<ShapeHandle> {
    const result = await this.client.request("replaceTriangulation", {
      scopeId: this.scope,
      shape,
      ...mesh,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async repairTriangulation(
    shape: ShapeHandle,
    options: { linearDeflection?: number; angularDeflection?: number; relative?: boolean } = {},
  ): Promise<TriangulationRepairResult> {
    const result = await this.client.request(
      "repairTriangulation",
      { scopeId: this.scope, shape, ...options },
    );
    return {
      shape: shapeFromKernel(this.client, result.shape),
      faces: result.faces,
      triangulatedFaces: result.triangulatedFaces,
      triangles: result.triangles,
    };
  }

  async trimSurface(shape: ShapeHandle, bounds: SurfaceBounds, tolerance?: number): Promise<ShapeHandle> {
    const result = await this.client.request("trimSurface", {
      scopeId: this.scope,
      shape,
      ...bounds,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async convertSurfaceToBSpline(shape: ShapeHandle, tolerance?: number): Promise<ShapeHandle> {
    const result = await this.client.request("convertSurfaceToBSpline", {
      scopeId: this.scope,
      shape,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async surfaceIsoCurve(
    surface: ShapeHandle,
    direction: "u" | "v",
    parameter: number,
    tolerance?: number,
  ): Promise<ShapeHandle[]> {
    const result = await this.client.request("surfaceIsoCurve", {
      scopeId: this.scope,
      surface,
      direction,
      parameter,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return result.shapes.map((shape) => shapeFromKernel(this.client, shape));
  }


}
