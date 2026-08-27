import { orientedOptions, expressionValue, vectorValue } from "./parametric-evaluator-values.js";
import type { BuiltFeature, FeatureBuildContext } from "./parametric-evaluator-context.js";
import type { PrimitiveParametricFeature } from "./parametric-types.js";

export async function buildPrimitiveFeature(context: FeatureBuildContext, feature: PrimitiveParametricFeature): Promise<BuiltFeature> {
  const { scope, parameters } = context;
  switch (feature.type) {
    case "box":
      return scope.makeBox(
        vectorValue(feature.size, parameters),
        feature.origin === undefined ? [0, 0, 0] : vectorValue(feature.origin, parameters),
      );
    case "cylinder":
      return scope.makeCylinder(expressionValue(feature.radius, parameters), expressionValue(feature.height, parameters), orientedOptions(feature, parameters));
    case "sphere":
      return scope.makeSphere(expressionValue(feature.radius, parameters), orientedOptions(feature, parameters));
    case "cone":
      return scope.makeCone(
        expressionValue(feature.radius1, parameters),
        expressionValue(feature.radius2, parameters),
        expressionValue(feature.height, parameters),
        orientedOptions(feature, parameters),
      );
    case "torus":
      return scope.makeTorus(expressionValue(feature.majorRadius, parameters), expressionValue(feature.minorRadius, parameters), orientedOptions(feature, parameters));
    case "wedge":
      return scope.makeWedge(vectorValue(feature.size, parameters), expressionValue(feature.ltx, parameters), orientedOptions(feature, parameters));
    case "polygon":
      return scope.makePolygon(feature.points.map((point) => vectorValue(point, parameters)), feature.close ?? true);
    case "bezierCurve":
      return scope.makeEdgeBezier(
        feature.poles.map((pole) => vectorValue(pole, parameters)),
        feature.weights === undefined
          ? {} : { weights: feature.weights.map((weight) => expressionValue(weight, parameters)) },
      );
    case "bsplineCurve":
      return scope.makeEdgeBSpline(
        feature.poles.map((pole) => vectorValue(pole, parameters)),
        {
          mode: "controlPoints",
          ...(feature.degree === undefined ? {} : { degree: expressionValue(feature.degree, parameters) }),
          ...(feature.periodic === undefined ? {} : { periodic: feature.periodic }),
          ...(feature.tolerance === undefined ? {} : { tolerance: expressionValue(feature.tolerance, parameters) }),
          ...(feature.knots === undefined
            ? {} : { knots: feature.knots.map((knot) => expressionValue(knot, parameters)) }),
          ...(feature.multiplicities === undefined ? {} : {
            multiplicities: feature.multiplicities.map((multiplicity) => expressionValue(multiplicity, parameters)),
          }),
          ...(feature.weights === undefined
            ? {} : { weights: feature.weights.map((weight) => expressionValue(weight, parameters)) }),
        },
      );
    case "bezierSurface":
      return scope.makeSurfaceBezier(
        feature.poles.map((row) => row.map((pole) => vectorValue(pole, parameters))),
        {
          ...(feature.weights === undefined ? {} : {
            weights: feature.weights.map((row) => row.map((weight) => expressionValue(weight, parameters))),
          }),
          ...(feature.tolerance === undefined ? {} : { tolerance: expressionValue(feature.tolerance, parameters) }),
        },
      );
    case "bsplineSurface":
      return scope.makeSurfaceBSpline(
        feature.poles.map((row) => row.map((pole) => vectorValue(pole, parameters))),
        {
          ...(feature.weights === undefined ? {} : {
            weights: feature.weights.map((row) => row.map((weight) => expressionValue(weight, parameters))),
          }),
          ...(feature.uDegree === undefined ? {} : { uDegree: expressionValue(feature.uDegree, parameters) }),
          ...(feature.vDegree === undefined ? {} : { vDegree: expressionValue(feature.vDegree, parameters) }),
          ...(feature.uKnots === undefined
            ? {} : { uKnots: feature.uKnots.map((knot) => expressionValue(knot, parameters)) }),
          ...(feature.vKnots === undefined
            ? {} : { vKnots: feature.vKnots.map((knot) => expressionValue(knot, parameters)) }),
          ...(feature.uMultiplicities === undefined ? {} : {
            uMultiplicities: feature.uMultiplicities.map((value) => expressionValue(value, parameters)),
          }),
          ...(feature.vMultiplicities === undefined ? {} : {
            vMultiplicities: feature.vMultiplicities.map((value) => expressionValue(value, parameters)),
          }),
          ...(feature.uPeriodic === undefined ? {} : { uPeriodic: feature.uPeriodic }),
          ...(feature.vPeriodic === undefined ? {} : { vPeriodic: feature.vPeriodic }),
          ...(feature.tolerance === undefined ? {} : { tolerance: expressionValue(feature.tolerance, parameters) }),
        },
      );

    default: {
      const exhaustive: never = feature;
      throw new TypeError(`Unsupported primitive feature ${String(exhaustive)}`);
    }
  }
}
