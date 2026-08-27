import { solveSketch, type SketchConstraint, type SketchEntity } from "./sketch.js";
import type { BuiltFeature, FeatureBuildContext } from "./parametric-evaluator-context.js";
import { expressionValue, point2Value, vectorValue } from "./parametric-evaluator-values.js";
import type { SketchParametricFeature } from "./parametric-types.js";
import type { ShapeHandle } from "./client.js";
import type { Vec3 } from "./types.js";

export async function buildSketchFeature(context: FeatureBuildContext, feature: SketchParametricFeature): Promise<BuiltFeature> {
  const { scope, parameters, shapes, shape } = context;
  switch (feature.type) {
    case "sketch": {
      const entities: SketchEntity[] = feature.entities.map((entity) => {
        if (entity.type === "line") {
          return {
            id: entity.id,
            type: "line",
            start: point2Value(entity.start, parameters),
            end: point2Value(entity.end, parameters),
            ...(entity.construction === undefined ? {} : { construction: entity.construction }),
          };
        }
        if (entity.type === "circle") {
          return {
            id: entity.id,
            type: "circle",
            center: point2Value(entity.center, parameters),
            radius: expressionValue(entity.radius, parameters),
            ...(entity.construction === undefined ? {} : { construction: entity.construction }),
          };
        }
        if (entity.type === "arc") {
          return {
            id: entity.id,
            type: "arc",
            center: point2Value(entity.center, parameters),
            radius: expressionValue(entity.radius, parameters),
            startAngle: expressionValue(entity.startAngle, parameters),
            endAngle: expressionValue(entity.endAngle, parameters),
            ...(entity.construction === undefined ? {} : { construction: entity.construction }),
          };
        }
        return {
          id: entity.id,
          type: "spline",
          poles: entity.poles.map((pole) => point2Value(pole, parameters)),
          ...(entity.degree === undefined ? {} : { degree: entity.degree }),
          ...(entity.construction === undefined ? {} : { construction: entity.construction }),
        };
      });
      const constraints: SketchConstraint[] = (feature.constraints ?? []).map((constraint) => {
        if (constraint.type === "fixed") {
          return { ...constraint, value: point2Value(constraint.value, parameters) };
        }
        if (constraint.type === "distance" || constraint.type === "length" || constraint.type === "radius") {
          return { ...constraint, value: expressionValue(constraint.value, parameters) };
        }
        return constraint;
      });
      const solved = solveSketch(
        { entities, constraints },
        {
          ...(feature.tolerance === undefined ? {} : { tolerance: expressionValue(feature.tolerance, parameters) }),
          ...(feature.maxIterations === undefined ? {} : { maxIterations: feature.maxIterations }),
        },
      );
      if (!solved.diagnostics.converged) {
        throw new Error(`Sketch feature "${feature.id}" did not converge; max residual ${solved.diagnostics.maxResidual}`);
      }
      const origin = feature.origin === undefined ? [0, 0, 0] as Vec3 : vectorValue(feature.origin, parameters);
      const normalValue = feature.normal === undefined ? [0, 0, 1] as Vec3 : vectorValue(feature.normal, parameters);
      const normalLength = Math.hypot(...normalValue);
      if (normalLength <= Number.EPSILON) throw new RangeError(`Sketch feature "${feature.id}" normal must be non-zero`);
      const normal: Vec3 = [
        normalValue[0] / normalLength,
        normalValue[1] / normalLength,
        normalValue[2] / normalLength,
      ];
      const xValue = feature.xDirection === undefined ? [1, 0, 0] as Vec3 : vectorValue(feature.xDirection, parameters);
      const normalProjection = xValue[0] * normal[0] + xValue[1] * normal[1] + xValue[2] * normal[2];
      const projectedX: Vec3 = [
        xValue[0] - normal[0] * normalProjection,
        xValue[1] - normal[1] * normalProjection,
        xValue[2] - normal[2] * normalProjection,
      ];
      const xLength = Math.hypot(...projectedX);
      if (xLength <= Number.EPSILON) throw new RangeError(`Sketch feature "${feature.id}" xDirection must not be parallel to normal`);
      const xDirection: Vec3 = [
        projectedX[0] / xLength,
        projectedX[1] / xLength,
        projectedX[2] / xLength,
      ];
      const yDirection: Vec3 = [
        normal[1] * xDirection[2] - normal[2] * xDirection[1],
        normal[2] * xDirection[0] - normal[0] * xDirection[2],
        normal[0] * xDirection[1] - normal[1] * xDirection[0],
      ];
      const mapPoint = (point: readonly [number, number]): Vec3 => [
        origin[0] + xDirection[0] * point[0] + yDirection[0] * point[1],
        origin[1] + xDirection[1] * point[0] + yDirection[1] * point[1],
        origin[2] + xDirection[2] * point[0] + yDirection[2] * point[1],
      ];
      const edgesById = new Map<string, ShapeHandle>();
      for (const entity of solved.entities) {
        if (entity.construction) continue;
        let edge: ShapeHandle;
        if (entity.type === "line") {
          edge = await scope.makeEdgeLine(mapPoint(entity.start), mapPoint(entity.end));
        } else if (entity.type === "circle") {
          edge = await scope.makeEdgeCircle(entity.radius, { center: mapPoint(entity.center), normal });
        } else if (entity.type === "arc") {
          edge = await scope.makeEdgeArc({
            center: mapPoint(entity.center),
            normal,
            radius: entity.radius,
            startAngle: entity.startAngle,
            endAngle: entity.endAngle,
            xDirection: xDirection,
          });
        } else {
          edge = await scope.makeEdgeBSpline(
            entity.poles.map((pole) => mapPoint(pole)),
            { mode: "controlPoints", ...(entity.degree === undefined ? {} : { degree: entity.degree }) },
          );
        }
        edgesById.set(entity.id, edge);
      }
      if (edgesById.size === 0) throw new Error(`Sketch feature "${feature.id}" has no construction geometry output`);
      if (feature.profiles === undefined) return scope.makeWire([...edgesById.values()]);
      const profileShapes: ShapeHandle[] = [];
      for (const profile of feature.profiles) {
        const profileEdges = profile.entities.map((entityId) => {
          const edge = edgesById.get(entityId);
          if (edge === undefined) throw new Error(`Sketch profile "${feature.id}.${profile.id}" references no output edge`);
          return edge;
        });
        const wire = await scope.makeWire(profileEdges);
        shapes.set(`${feature.id}.${profile.id}`, wire);
        profileShapes.push(wire);
      }
      return profileShapes.length === 1 ? profileShapes[0]! : scope.makeCompound(profileShapes);
    }
    case "face":
      return scope.makeFace(shape(feature.outer), (feature.holes ?? []).map(shape));

    default: {
      const exhaustive: never = feature;
      throw new TypeError(`Unsupported sketch feature ${String(exhaustive)}`);
    }
  }
}
