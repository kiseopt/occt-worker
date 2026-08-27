import { shapeFromKernel, type KernelShapeToken, type ShapeHandle } from "./client-contract.js";
import { ShapeScopeCore } from "./shape-scope-core.js";
import type {
  BSplineApproximationOptions,
  BSplineCurveEdit,
  CurveDegreeReductionOptions,
  CylindricalHoleOptions,
  DraftAngleOptions,
  ExtensionContinuity,
  FilletRadiusPoint,
  History,
  LinearFormOptions,
  LocalPrismOptions,
  LocalRevolutionOptions,
  RevolutionFormOptions,
  Vec3,
} from "./types.js";

export class ModelingShapeScope extends ShapeScopeCore {
  async extrude(shape: ShapeHandle, vector: Vec3, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async extrude(shape: ShapeHandle, vector: Vec3, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async extrude(shape: ShapeHandle, vector: Vec3, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("extrude", { scopeId: this.scope, shape, vector, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("extrude did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async revolve(shape: ShapeHandle, origin: Vec3, direction: Vec3, angle?: number, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async revolve(shape: ShapeHandle, origin: Vec3, direction: Vec3, angle: number | undefined, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async revolve(shape: ShapeHandle, origin: Vec3, direction: Vec3, angle?: number, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("revolve", { scopeId: this.scope, shape, origin, direction, ...(angle === undefined ? {} : { angle }), ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
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
    const result = await this.client.request("localPrism", {
      scopeId: this.scope,
      base,
      faceIndices,
      direction,
      ...limitArgs,
    });
    return shapeFromKernel(this.client, result.shape);
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
    const result = await this.client.request("localRevolution", {
      scopeId: this.scope,
      base,
      faceIndices,
      origin,
      direction,
      ...limitArgs,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async linearForm(
    base: ShapeHandle,
    profile: ShapeHandle,
    options: LinearFormOptions,
  ): Promise<ShapeHandle> {
    const result = await this.client.request("linearForm", {
      scopeId: this.scope,
      base,
      profile,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async revolutionForm(
    base: ShapeHandle,
    profile: ShapeHandle,
    options: RevolutionFormOptions,
  ): Promise<ShapeHandle> {
    const result = await this.client.request("revolutionForm", {
      scopeId: this.scope,
      base,
      profile,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
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
    const result = await this.client.request("cylindricalHole", {
      scopeId: this.scope,
      shape,
      origin,
      direction,
      radius,
      ...options,
    });
    const resultShape = shapeFromKernel(this.client, result.shape);
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
    const result = await this.client.request("defeature", {
      scopeId: this.scope,
      shape,
      faceIndices,
      ...options,
    });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return { shape: resultShape };
    if (result.history === undefined) throw new Error("defeature did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async loft(sections: readonly ShapeHandle[], options?: { solid?: boolean; ruled?: boolean; precision?: number; includeHistory?: false }): Promise<ShapeHandle>;
  async loft(sections: readonly ShapeHandle[], options: { solid?: boolean; ruled?: boolean; precision?: number; includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async loft(sections: readonly ShapeHandle[], options: { solid?: boolean; ruled?: boolean; precision?: number; includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("loft", { scopeId: this.scope, sections, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("loft did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async sweepPipe(spine: ShapeHandle, profile: ShapeHandle, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async sweepPipe(spine: ShapeHandle, profile: ShapeHandle, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async sweepPipe(spine: ShapeHandle, profile: ShapeHandle, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("sweepPipe", { scopeId: this.scope, spine, profile, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("sweepPipe did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async middlePath(
    shape: ShapeHandle,
    start: ShapeHandle,
    end: ShapeHandle,
  ): Promise<ShapeHandle> {
    const result = await this.client.request("middlePath", {
      scopeId: this.scope,
      shape,
      start,
      end,
    });
    return shapeFromKernel(this.client, result.shape);
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
    const result = await this.client.request("sweepPipeShell", { scopeId: this.scope, spine, profiles, ...requestOptions });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!requestOptions.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("sweepPipeShell did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async fillet(shape: ShapeHandle, edgeIndices: readonly number[], radius: number, options?: { radii?: readonly number[]; radius2?: number; radiusLaw?: readonly FilletRadiusPoint[]; radiusLaws?: readonly (readonly FilletRadiusPoint[])[]; includeHistory?: false }): Promise<ShapeHandle>;
  async fillet(shape: ShapeHandle, edgeIndices: readonly number[], radius: number, options: { radii?: readonly number[]; radius2?: number; radiusLaw?: readonly FilletRadiusPoint[]; radiusLaws?: readonly (readonly FilletRadiusPoint[])[]; includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async fillet(shape: ShapeHandle, edgeIndices: readonly number[], radius: number, options: { radii?: readonly number[]; radius2?: number; radiusLaw?: readonly FilletRadiusPoint[]; radiusLaws?: readonly (readonly FilletRadiusPoint[])[]; includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("fillet", { scopeId: this.scope, shape, edgeIndices, radius, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("fillet did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async chamfer(shape: ShapeHandle, edgeIndices: readonly number[], distance: number, options?: { distances?: readonly number[]; distance2?: number; distances2?: readonly number[]; referenceFaceIndices?: readonly number[]; includeHistory?: false }): Promise<ShapeHandle>;
  async chamfer(shape: ShapeHandle, edgeIndices: readonly number[], distance: number, options: { distances?: readonly number[]; distance2?: number; distances2?: readonly number[]; referenceFaceIndices?: readonly number[]; includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async chamfer(shape: ShapeHandle, edgeIndices: readonly number[], distance: number, options: { distances?: readonly number[]; distance2?: number; distances2?: readonly number[]; referenceFaceIndices?: readonly number[]; includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("chamfer", { scopeId: this.scope, shape, edgeIndices, distance, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
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
    const result = await this.client.request("draftAngle", {
      scopeId: this.scope,
      shape,
      faceIndices,
      direction,
      angle,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async hollow(
    shape: ShapeHandle,
    thickness: number,
    closingFaceIndices: readonly number[] = [],
    tolerance?: number,
  ): Promise<ShapeHandle> {
    const result = await this.client.request("hollow", {
      scopeId: this.scope,
      shape,
      thickness,
      closingFaceIndices,
      ...(tolerance === undefined ? {} : { tolerance }),
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async offsetShape(shape: ShapeHandle, offset: number, tolerance?: number): Promise<ShapeHandle> {
    const result = await this.client.request("offsetShape", { scopeId: this.scope, shape, offset, ...(tolerance === undefined ? {} : { tolerance }) });
    return shapeFromKernel(this.client, result.shape);
  }

  async offsetWire2D(wire: ShapeHandle, offset: number): Promise<ShapeHandle> {
    const result = await this.client.request("offsetWire2D", { scopeId: this.scope, wire, offset });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeEdgeLine(start: Vec3, end: Vec3): Promise<ShapeHandle> {
    const result = await this.client.request("makeEdgeLine", { scopeId: this.scope, start, end });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeEdgeArc(start: Vec3, point: Vec3, end: Vec3): Promise<ShapeHandle>;
  async makeEdgeArc(options: { center: Vec3; normal: Vec3; radius: number; startAngle: number; endAngle: number; xDirection?: Vec3 }): Promise<ShapeHandle>;
  async makeEdgeArc(
    startOrOptions: Vec3 | { center: Vec3; normal: Vec3; radius: number; startAngle: number; endAngle: number; xDirection?: Vec3 },
    point?: Vec3,
    end?: Vec3,
  ): Promise<ShapeHandle> {
    const args = Array.isArray(startOrOptions) ? { start: startOrOptions, point, end } : startOrOptions;
    const result = await this.client.requestUnsafe<{ shape: KernelShapeToken }>("makeEdgeArc", { scopeId: this.scope, ...args });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeEdgeCircle(radius: number, options: { center?: Vec3; normal?: Vec3 } = {}): Promise<ShapeHandle> {
    const result = await this.client.request("makeEdgeCircle", { scopeId: this.scope, radius, ...options });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeEdgeEllipse(majorRadius: number, minorRadius: number, options: { center?: Vec3; normal?: Vec3 } = {}): Promise<ShapeHandle> {
    const result = await this.client.request("makeEdgeEllipse", { scopeId: this.scope, majorRadius, minorRadius, ...options });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeEdgeHyperbola(
    majorRadius: number,
    minorRadius: number,
    firstParameter: number,
    lastParameter: number,
    options: { center?: Vec3; normal?: Vec3; xDirection?: Vec3 } = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("makeEdgeHyperbola", {
      scopeId: this.scope,
      majorRadius,
      minorRadius,
      firstParameter,
      lastParameter,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeEdgeParabola(
    focal: number,
    firstParameter: number,
    lastParameter: number,
    options: { center?: Vec3; normal?: Vec3; xDirection?: Vec3 } = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("makeEdgeParabola", {
      scopeId: this.scope,
      focal,
      firstParameter,
      lastParameter,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeEdgeOffset(shape: ShapeHandle, offset: number, direction: Vec3): Promise<ShapeHandle> {
    const result = await this.client.request("makeEdgeOffset", {
      scopeId: this.scope,
      shape,
      offset,
      direction,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async approximateCurveBSpline(
    points: readonly Vec3[] | Float64Array,
    options: BSplineApproximationOptions = {},
  ): Promise<ShapeHandle> {
    const encodedPoints = points instanceof Float64Array
      ? { $inputBuffer: new Float64Array(points).buffer }
      : points;
    const result = await this.client.requestUnsafe<{ shape: KernelShapeToken }>("approximateCurveBSpline", {
      scopeId: this.scope,
      points: encodedPoints,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async approximateSurfaceBSpline(
    points: readonly (readonly Vec3[])[],
    options: BSplineApproximationOptions = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("approximateSurfaceBSpline", {
      scopeId: this.scope,
      points,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async extendCurve(
    shape: ShapeHandle,
    point: Vec3,
    options: { continuity?: ExtensionContinuity; after?: boolean } = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("extendCurve", {
      scopeId: this.scope,
      shape,
      point,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async extendSurface(
    shape: ShapeHandle,
    length: number,
    direction: "u" | "v",
    side: "before" | "after",
    continuity: ExtensionContinuity = "c1",
  ): Promise<ShapeHandle> {
    const result = await this.client.request("extendSurface", {
      scopeId: this.scope,
      shape,
      length,
      direction,
      side,
      continuity,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeEdgeBezier(
    poles: readonly Vec3[],
    options: { weights?: readonly number[] } = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("makeEdgeBezier", {
      scopeId: this.scope,
      poles,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
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
    const result = await this.client.requestUnsafe<{ shape: KernelShapeToken }>("makeEdgeBSpline", {
      scopeId: this.scope,
      poles: encodedPoles,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeEdgeHelix(radius: number, pitch: number, turns: number, options: { origin?: Vec3; direction?: Vec3; handedness?: "right" | "left" } = {}): Promise<ShapeHandle> {
    const result = await this.client.request("makeEdgeHelix", { scopeId: this.scope, radius, pitch, turns, ...options });
    return shapeFromKernel(this.client, result.shape);
  }

  async updateCurvePole(
    shape: ShapeHandle,
    index: number,
    point: Vec3,
    options: { weight?: number } = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("updateCurvePole", {
      scopeId: this.scope,
      shape,
      index,
      point,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async editCurveBSpline(shape: ShapeHandle, edit: BSplineCurveEdit): Promise<ShapeHandle> {
    const result = await this.client.request("editCurveBSpline", {
      scopeId: this.scope,
      shape,
      ...edit,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async reduceCurveDegree(
    shape: ShapeHandle,
    degree: number,
    options: CurveDegreeReductionOptions = {},
  ): Promise<{ shape: ShapeHandle; maxError: number }> {
    const result = await this.client.request("reduceCurveDegree", {
      scopeId: this.scope,
      shape,
      degree,
      ...options,
    });
    return { shape: shapeFromKernel(this.client, result.shape), maxError: result.maxError };
  }

  async reparameterizeCurve(shape: ShapeHandle, first: number, last: number): Promise<ShapeHandle> {
    const result = await this.client.request("reparameterizeCurve", {
      scopeId: this.scope,
      shape,
      first,
      last,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async trimCurve(shape: ShapeHandle, first: number, last: number): Promise<ShapeHandle> {
    const result = await this.client.request("trimCurve", {
      scopeId: this.scope, shape, first, last,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async convertCurveToBSpline(
    shape: ShapeHandle,
    parameterization: "tgtThetaOver2" | "quasiAngular" | "rationalC1" | "polynomial" = "tgtThetaOver2",
  ): Promise<ShapeHandle> {
    const result = await this.client.request("convertCurveToBSpline", {
      scopeId: this.scope, shape, parameterization,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async makeSolidFromShell(shell: ShapeHandle): Promise<ShapeHandle> {
    const result = await this.client.request("makeSolidFromShell", { scopeId: this.scope, shell });
    return shapeFromKernel(this.client, result.shape);
  }


}
