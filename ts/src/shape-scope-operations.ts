import { shapeFromKernel, type KernelShapeToken, type ShapeHandle } from "./client-contract.js";
import { ModelingShapeScope } from "./shape-scope-modeling.js";
import type {
  GeneralFuseCellRule,
  GeneralFuseCellSelectionOptions,
  GlueBinding,
  History,
  HLRProjection,
  HLRProjectionOptions,
  Matrix12,
  SectionAnalysis,
  ShapeUpgradeOptions,
  Vec3,
} from "./types.js";

export class OperationsShapeScope extends ModelingShapeScope {
  async generalTransform(shape: ShapeHandle, matrix: Matrix12, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async generalTransform(shape: ShapeHandle, matrix: Matrix12, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async generalTransform(shape: ShapeHandle, matrix: Matrix12, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("generalTransform", { scopeId: this.scope, shape, matrix, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("generalTransform did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async sew(shapes: readonly ShapeHandle[], tolerance?: number, options?: { includeHistory?: false }): Promise<{ shape: ShapeHandle; freeEdges: number; multipleEdges: number }>;
  async sew(shapes: readonly ShapeHandle[], tolerance: number | undefined, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; freeEdges: number; multipleEdges: number; history: History }>;
  async sew(shapes: readonly ShapeHandle[], tolerance?: number, options: { includeHistory?: boolean } = {}): Promise<{ shape: ShapeHandle; freeEdges: number; multipleEdges: number; history?: History }> {
    const result = await this.client.request("sew", {
      scopeId: this.scope, shapes, ...(tolerance === undefined ? {} : { tolerance }), ...options,
    });
    const response = { shape: shapeFromKernel(this.client, result.shape), freeEdges: result.freeEdges, multipleEdges: result.multipleEdges };
    if (!options.includeHistory) return response;
    if (result.history === undefined) throw new Error("sew did not return topology history");
    return { ...response, history: result.history };
  }

  async fixShape(shape: ShapeHandle, precision?: number, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async fixShape(shape: ShapeHandle, precision: number | undefined, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async fixShape(shape: ShapeHandle, precision?: number, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("fixShape", {
      scopeId: this.scope, shape, ...(precision === undefined ? {} : { precision }), ...options,
    });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("fixShape did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async setTolerance(
    shape: ShapeHandle,
    tolerance: number,
    type: "all" | "face" | "edge" | "vertex" = "all",
  ): Promise<ShapeHandle> {
    const result = await this.client.request("setTolerance", {
      scopeId: this.scope,
      shape,
      tolerance,
      type,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async unifySameDomain(shape: ShapeHandle, options?: { unifyEdges?: boolean; unifyFaces?: boolean; concatBSplines?: boolean; includeHistory?: false }): Promise<ShapeHandle>;
  async unifySameDomain(shape: ShapeHandle, options: { unifyEdges?: boolean; unifyFaces?: boolean; concatBSplines?: boolean; includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async unifySameDomain(shape: ShapeHandle, options: { unifyEdges?: boolean; unifyFaces?: boolean; concatBSplines?: boolean; includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("unifySameDomain", { scopeId: this.scope, shape, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("unifySameDomain did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async shapeUpgrade(
    shape: ShapeHandle,
    options: ShapeUpgradeOptions = {},
  ): Promise<ShapeHandle> {
    const upgrade = options.mode === undefined
      ? { ...options, mode: "continuity" as const }
      : options;
    const result = await this.client.request("shapeUpgrade", {
      scopeId: this.scope,
      shape,
      ...upgrade,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async booleanFuse(base: ShapeHandle, tools: readonly ShapeHandle[], options?: { includeHistory?: false }): Promise<{ shape: ShapeHandle }>;
  async booleanFuse(base: ShapeHandle, tools: readonly ShapeHandle[], options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async booleanFuse(base: ShapeHandle, tools: readonly ShapeHandle[], options: { includeHistory?: boolean } = {}): Promise<{ shape: ShapeHandle; history?: History }> {
    const result = await this.client.request("booleanFuse", { scopeId: this.scope, base, tools, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return { shape: resultShape };
    if (result.history === undefined) throw new Error("booleanFuse did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async booleanCommon(base: ShapeHandle, tools: readonly ShapeHandle[], options?: { includeHistory?: false }): Promise<{ shape: ShapeHandle }>;
  async booleanCommon(base: ShapeHandle, tools: readonly ShapeHandle[], options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async booleanCommon(base: ShapeHandle, tools: readonly ShapeHandle[], options: { includeHistory?: boolean } = {}): Promise<{ shape: ShapeHandle; history?: History }> {
    const result = await this.client.request("booleanCommon", { scopeId: this.scope, base, tools, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return { shape: resultShape };
    if (result.history === undefined) throw new Error("booleanCommon did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async section(first: ShapeHandle, second: ShapeHandle): Promise<ShapeHandle> {
    const result = await this.client.request("section", { scopeId: this.scope, first, second });
    return shapeFromKernel(this.client, result.shape);
  }

  async projectHLR(shape: ShapeHandle, options: HLRProjectionOptions): Promise<HLRProjection> {
    const result = await this.client.request("projectHLR", {
      scopeId: this.scope,
      shape,
      ...options,
    });
    return {
      visible: shapeFromKernel(this.client, result.visible),
      hidden: shapeFromKernel(this.client, result.hidden),
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
    const result = await this.client.request("sectionAnalysis", { scopeId: this.scope, first, second, ...options });
    return { ...result, shape: shapeFromKernel(this.client, result.shape) };
  }

  async split(
    objects: readonly ShapeHandle[],
    tools: readonly ShapeHandle[] = [],
    options: { fuzzyValue?: number; useOBB?: boolean } = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("split", {
      scopeId: this.scope,
      objects,
      tools,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async generalFuse(
    shapes: readonly ShapeHandle[],
    options: { fuzzyValue?: number; glue?: "off" | "shift" | "full"; useOBB?: boolean } = {},
  ): Promise<{ shapes: ShapeHandle[]; sourceIndices: number[][] }> {
    const result = await this.client.request("generalFuse", {
      scopeId: this.scope, shapes, ...options,
    });
    return {
      shapes: result.shapes.map((shape) => shapeFromKernel(this.client, shape)),
      sourceIndices: result.sourceIndices,
    };
  }

  async selectGeneralFuseCells(
    shapes: readonly ShapeHandle[],
    rules: readonly GeneralFuseCellRule[],
    options: GeneralFuseCellSelectionOptions = {},
  ): Promise<ShapeHandle> {
    const result = await this.client.request("selectGeneralFuseCells", {
      scopeId: this.scope,
      shapes,
      rules,
      ...options,
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async glue(
    newShape: ShapeHandle,
    baseShape: ShapeHandle,
    faceBindings: readonly GlueBinding[],
    edgeBindings: readonly GlueBinding[] = [],
  ): Promise<ShapeHandle> {
    const result = await this.client.request("glue", {
      scopeId: this.scope,
      newShape,
      baseShape,
      faceBindings,
      ...(edgeBindings.length === 0 ? {} : { edgeBindings }),
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async intersectSurfaceSurface(first: ShapeHandle, second: ShapeHandle, tolerance = 1e-7): Promise<ShapeHandle[]> {
    const result = await this.client.request("intersectSurfaceSurface", {
      scopeId: this.scope,
      first,
      second,
      tolerance,
    });
    return result.shapes.map((value) => shapeFromKernel(this.client, value));
  }

  async intersectCurveCurve(first: ShapeHandle, second: ShapeHandle, tolerance = 1e-7): Promise<ShapeHandle[]> {
    const result = await this.client.request("intersectCurveCurve", {
      scopeId: this.scope,
      first,
      second,
      tolerance,
    });
    return result.shapes.map((value) => shapeFromKernel(this.client, value));
  }

  async transform(shape: ShapeHandle, options?: { matrix?: Matrix12; translation?: Vec3; rotation?: { origin?: Vec3; direction?: Vec3; angle: number }; scale?: number; scaleOrigin?: Vec3; mirror?: Vec3; copy?: boolean; includeHistory?: false }): Promise<ShapeHandle>;
  async transform(shape: ShapeHandle, options: { matrix?: Matrix12; translation?: Vec3; rotation?: { origin?: Vec3; direction?: Vec3; angle: number }; scale?: number; scaleOrigin?: Vec3; mirror?: Vec3; copy?: boolean; includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async transform(shape: ShapeHandle, options: { matrix?: Matrix12; translation?: Vec3; rotation?: { origin?: Vec3; direction?: Vec3; angle: number }; scale?: number; scaleOrigin?: Vec3; mirror?: Vec3; copy?: boolean; includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("transform", { scopeId: this.scope, shape, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
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
    const result = await this.client.request("batchTransformCopy", {
      scopeId: this.scope,
      shape,
      ...options,
    });
    return result.shapes.map((value) => shapeFromKernel(this.client, value));
  }

  async translate(shape: ShapeHandle, translation: Vec3, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async translate(shape: ShapeHandle, translation: Vec3, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async translate(shape: ShapeHandle, translation: Vec3, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("translate", { scopeId: this.scope, shape, translation, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("translate did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async rotate(shape: ShapeHandle, rotation: { origin?: Vec3; direction?: Vec3; angle: number }, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async rotate(shape: ShapeHandle, rotation: { origin?: Vec3; direction?: Vec3; angle: number }, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async rotate(shape: ShapeHandle, rotation: { origin?: Vec3; direction?: Vec3; angle: number }, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("rotate", { scopeId: this.scope, shape, rotation, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("rotate did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async scale(shape: ShapeHandle, factor: number, origin?: Vec3, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async scale(shape: ShapeHandle, factor: number, origin: Vec3 | undefined, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async scale(shape: ShapeHandle, factor: number, origin?: Vec3, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("scale", { scopeId: this.scope, shape, factor, ...(origin === undefined ? {} : { origin }), ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return resultShape;
    if (result.history === undefined) throw new Error("scale did not return topology history");
    return { shape: resultShape, history: result.history };
  }

  async mirror(shape: ShapeHandle, normal: Vec3, options?: { includeHistory?: false }): Promise<ShapeHandle>;
  async mirror(shape: ShapeHandle, normal: Vec3, options: { includeHistory: true }): Promise<{ shape: ShapeHandle; history: History }>;
  async mirror(shape: ShapeHandle, normal: Vec3, options: { includeHistory?: boolean } = {}): Promise<ShapeHandle | { shape: ShapeHandle; history: History }> {
    const result = await this.client.request("mirror", { scopeId: this.scope, shape, normal, ...options });
    const resultShape = shapeFromKernel(this.client, result.shape);
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
    const result = await this.client.request("booleanCut", {
      scopeId: this.scope,
      base,
      tools,
      ...options,
    });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeHistory) return { shape: resultShape };
    if (result.history === undefined) throw new Error("booleanCut did not return topology history");
    return { shape: resultShape, history: result.history };
  }


}
