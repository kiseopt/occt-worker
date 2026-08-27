import type { ShapeHandle, ShapeScope } from "./client.js";
import type { ParametricFeature, ParametricSubshapeReference, ParametricSubshapeSelector } from "./parametric-types.js";
import { buildPrimitiveFeature } from "./parametric-evaluator-primitives.js";
import { buildSketchFeature } from "./parametric-evaluator-sketch.js";
import { buildOperationFeature } from "./parametric-evaluator-operations.js";
import type { BuiltFeature } from "./parametric-evaluator-context.js";

export { expressionValue, vectorValue, point2Value, matrixValue } from "./parametric-evaluator-values.js";

export async function buildFeature(
  scope: ShapeScope,
  feature: ParametricFeature,
  parameters: Readonly<Record<string, number>>,
  shapes: Map<string, ShapeHandle>,
  subshapeReferenceDefinitions: ReadonlyMap<string, ParametricSubshapeReference>,
  subshapeReferenceIndices: ReadonlyMap<string, number>,
): Promise<BuiltFeature> {
  const shape = (id: string): ShapeHandle => {
    const result = shapes.get(id);
    if (result === undefined) throw new ReferenceError(`Feature "${id}" has not been built`);
    return result;
  };
  const subshapeIndices = (
    selectors: readonly ParametricSubshapeSelector[],
    type: "face" | "edge",
    featureId: string,
  ): number[] => selectors.map((selector) => {
    if (typeof selector === "number") {
      if (!Number.isInteger(selector) || selector < 0) {
        throw new TypeError(`${type} index must be a non-negative integer`);
      }
      return selector;
    }
    if (selector === null || typeof selector !== "object" || typeof selector.reference !== "string") {
      throw new TypeError(`${type} selector must be an index or a subshape reference`);
    }
    const definition = subshapeReferenceDefinitions.get(selector.reference);
    if (definition === undefined) {
      throw new ReferenceError(`Unknown subshape reference "${selector.reference}"`);
    }
    if (definition.feature !== featureId || definition.type !== type) {
      throw new TypeError(
        `Subshape reference "${selector.reference}" must select a ${type} on feature "${featureId}"`,
      );
    }
    const index = subshapeReferenceIndices.get(selector.reference);
    if (index === undefined) {
      throw new ReferenceError(`Subshape reference "${selector.reference}" is not uniquely resolved`);
    }
    return index;
  });
  const context = {
    scope, parameters, shapes, shape, subshapeIndices,
    subshapeReferenceDefinitions, subshapeReferenceIndices,
  };
  switch (feature.type) {
    case "box": case "cylinder": case "sphere": case "cone": case "torus": case "wedge":
    case "polygon": case "bezierCurve": case "bsplineCurve": case "bezierSurface": case "bsplineSurface":
      return buildPrimitiveFeature(context, feature);
    case "sketch": case "face":
      return buildSketchFeature(context, feature);
    case "extrude": case "revolve": case "loft": case "sweepPipe": case "sweepPipeShell":
    case "fillet": case "chamfer": case "hollow": case "offsetShape": case "offsetWire2D":
    case "draftAngle": case "localPrism": case "localRevolution": case "linearForm": case "revolutionForm":
    case "glue": case "cylindricalHole": case "defeature": case "sew": case "fixShape":
    case "unifySameDomain": case "shapeUpgrade": case "section": case "split": case "translate":
    case "rotate": case "scale": case "mirror": case "generalTransform":
    case "booleanCut": case "booleanFuse": case "booleanCommon":
      return buildOperationFeature(context, feature);
    default: {
      const exhaustive: never = feature;
      throw new TypeError(`Unsupported feature ${String(exhaustive)}`);
    }
  }
}
