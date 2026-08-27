import { matrixValue, expressionValue, vectorValue } from "./parametric-evaluator-values.js";
import type { BuiltFeature, FeatureBuildContext } from "./parametric-evaluator-context.js";
import type { OperationParametricFeature } from "./parametric-types.js";
import type { ShapeHandle } from "./client.js";
import type { History } from "./types.js";

export async function buildOperationFeature(context: FeatureBuildContext, feature: OperationParametricFeature): Promise<BuiltFeature> {
  const { scope, parameters, shape, subshapeIndices } = context;
  switch (feature.type) {
    case "extrude": {
      const result = await scope.extrude(
        shape(feature.input), vectorValue(feature.vector, parameters), { includeHistory: true },
      );
      return { ...result, inputs: [feature.input] };
    }
    case "revolve": {
      const result = await scope.revolve(
        shape(feature.input),
        vectorValue(feature.origin, parameters),
        vectorValue(feature.direction, parameters),
        feature.angle === undefined ? undefined : expressionValue(feature.angle, parameters),
        { includeHistory: true },
      );
      return { ...result, inputs: [feature.input] };
    }
    case "loft": {
      const result = await scope.loft(feature.sections.map(shape), {
        ...(feature.solid === undefined ? {} : { solid: feature.solid }),
        ...(feature.ruled === undefined ? {} : { ruled: feature.ruled }),
        ...(feature.precision === undefined ? {} : { precision: expressionValue(feature.precision, parameters) }),
        includeHistory: true,
      });
      return { ...result, inputs: feature.sections };
    }
    case "sweepPipe": {
      const result = await scope.sweepPipe(
        shape(feature.spine), shape(feature.profile), { includeHistory: true },
      );
      return { ...result, inputs: [feature.spine, feature.profile] };
    }
    case "sweepPipeShell": {
      let result: { shape: ShapeHandle; history: History };
      const historyInputs = [feature.spine, ...feature.profiles];
      if (feature.mode === "fixedAxis") {
        if (feature.axis === undefined) throw new TypeError(`Feature "${feature.id}" requires axis`);
        result = await scope.sweepPipeShell(shape(feature.spine), feature.profiles.map(shape), {
          mode: "fixedAxis",
          axis: {
            ...(feature.axis.origin === undefined ? {} : { origin: vectorValue(feature.axis.origin, parameters) }),
            direction: vectorValue(feature.axis.direction, parameters),
            ...(feature.axis.xDirection === undefined ? {} : { xDirection: vectorValue(feature.axis.xDirection, parameters) }),
          },
          includeHistory: true,
        });
      } else if (feature.mode === "auxiliarySpine") {
        if (feature.auxiliarySpine === undefined) throw new TypeError(`Feature "${feature.id}" requires auxiliarySpine`);
        historyInputs.push(feature.auxiliarySpine);
        result = await scope.sweepPipeShell(shape(feature.spine), feature.profiles.map(shape), {
          mode: "auxiliarySpine",
          auxiliarySpine: shape(feature.auxiliarySpine),
          ...(feature.curvilinearEquivalence === undefined
            ? {} : { curvilinearEquivalence: feature.curvilinearEquivalence }),
          includeHistory: true,
        });
      } else {
        result = await scope.sweepPipeShell(shape(feature.spine), feature.profiles.map(shape), {
          mode: feature.mode ?? "frenet",
          includeHistory: true,
        });
      }
      return {
        ...result,
        inputs: historyInputs,
      };
    }
    case "fillet": {
      const result = await scope.fillet(
        shape(feature.input),
        subshapeIndices(feature.edgeIndices, "edge", feature.input),
        expressionValue(feature.radius, parameters),
        {
          ...(feature.radii === undefined
            ? {} : { radii: feature.radii.map((radius) => expressionValue(radius, parameters)) }),
          ...(feature.radius2 === undefined
            ? {} : { radius2: expressionValue(feature.radius2, parameters) }),
          ...(feature.radiusLaw === undefined ? {} : {
            radiusLaw: feature.radiusLaw.map((point) => ({
              parameter: expressionValue(point.parameter, parameters),
              radius: expressionValue(point.radius, parameters),
            })),
          }),
          ...(feature.radiusLaws === undefined ? {} : {
            radiusLaws: feature.radiusLaws.map((law) => law.map((point) => ({
              parameter: expressionValue(point.parameter, parameters),
              radius: expressionValue(point.radius, parameters),
            }))),
          }),
          includeHistory: true,
        },
      );
      if (result.history === undefined) throw new Error(`Feature "${feature.id}" did not return topology history`);
      return { ...result, history: result.history, inputs: [feature.input] };
    }
    case "chamfer": {
      const result = await scope.chamfer(
        shape(feature.input),
        subshapeIndices(feature.edgeIndices, "edge", feature.input),
        expressionValue(feature.distance, parameters),
        {
          ...(feature.distances === undefined
            ? {} : { distances: feature.distances.map((distance) => expressionValue(distance, parameters)) }),
          ...(feature.distance2 === undefined
            ? {} : { distance2: expressionValue(feature.distance2, parameters) }),
          ...(feature.distances2 === undefined
            ? {} : { distances2: feature.distances2.map((distance) => expressionValue(distance, parameters)) }),
          ...(feature.referenceFaceIndices === undefined
            ? {} : { referenceFaceIndices: subshapeIndices(feature.referenceFaceIndices, "face", feature.input) }),
          includeHistory: true,
        },
      );
      if (result.history === undefined) throw new Error(`Feature "${feature.id}" did not return topology history`);
      return { ...result, history: result.history, inputs: [feature.input] };
    }
    case "hollow":
      return scope.hollow(
        shape(feature.input),
        expressionValue(feature.thickness, parameters),
        subshapeIndices(feature.closingFaceIndices ?? [], "face", feature.input),
      );
    case "offsetShape":
      return scope.offsetShape(
        shape(feature.input),
        expressionValue(feature.offset, parameters),
        feature.tolerance === undefined ? undefined : expressionValue(feature.tolerance, parameters),
      );
    case "offsetWire2D":
      return scope.offsetWire2D(shape(feature.input), expressionValue(feature.offset, parameters));
    case "draftAngle":
      return scope.draftAngle(
        shape(feature.input),
        subshapeIndices(feature.faceIndices, "face", feature.input),
        vectorValue(feature.direction, parameters),
        expressionValue(feature.angle, parameters),
        {
          neutralPlane: {
            origin: vectorValue(feature.neutralPlane.origin, parameters),
            normal: vectorValue(feature.neutralPlane.normal, parameters),
          },
          ...(feature.flag === undefined ? {} : { flag: feature.flag }),
        },
      );
    case "localPrism": {
      const common = [shape(feature.base), subshapeIndices(feature.faceIndices, "face", feature.base),
        vectorValue(feature.direction, parameters)] as const;
      if (feature.mode === undefined || feature.mode === "length") {
        return scope.localPrism(...common, expressionValue(feature.length, parameters), feature.operation ?? "add");
      }
      if (feature.mode === "until") {
        return scope.localPrism(...common, {
          mode: feature.mode, until: shape(feature.until), operation: feature.operation ?? "add",
        });
      }
      if (feature.mode === "fromUntil") {
        return scope.localPrism(...common, {
          mode: feature.mode, from: shape(feature.from), until: shape(feature.until),
          operation: feature.operation ?? "add",
        });
      }
      if (feature.mode === "fromEnd") {
        return scope.localPrism(...common, {
          mode: feature.mode, from: shape(feature.from), operation: feature.operation ?? "add",
        });
      }
      return scope.localPrism(...common, { mode: feature.mode, operation: feature.operation ?? "add" });
    }
    case "localRevolution": {
      const common = [shape(feature.base), subshapeIndices(feature.faceIndices, "face", feature.base),
        vectorValue(feature.origin, parameters), vectorValue(feature.direction, parameters)] as const;
      if (feature.mode === undefined || feature.mode === "angle") {
        return scope.localRevolution(...common, expressionValue(feature.angle, parameters), feature.operation ?? "add");
      }
      if (feature.mode === "until") {
        return scope.localRevolution(...common, {
          mode: feature.mode, until: shape(feature.until), operation: feature.operation ?? "add",
        });
      }
      if (feature.mode === "fromUntil") {
        return scope.localRevolution(...common, {
          mode: feature.mode, from: shape(feature.from), until: shape(feature.until),
          operation: feature.operation ?? "add",
        });
      }
      return scope.localRevolution(...common, { mode: feature.mode, operation: feature.operation ?? "add" });
    }
    case "linearForm":
      return scope.linearForm(shape(feature.base), shape(feature.profile), {
        planeOrigin: vectorValue(feature.planeOrigin, parameters),
        planeNormal: vectorValue(feature.planeNormal, parameters),
        direction: vectorValue(feature.direction, parameters),
        ...(feature.direction1 === undefined ? {} : { direction1: vectorValue(feature.direction1, parameters) }),
        ...(feature.operation === undefined ? {} : { operation: feature.operation }),
        ...(feature.modify === undefined ? {} : { modify: feature.modify }),
      });
    case "revolutionForm":
      return scope.revolutionForm(shape(feature.base), shape(feature.profile), {
        planeOrigin: vectorValue(feature.planeOrigin, parameters),
        planeNormal: vectorValue(feature.planeNormal, parameters),
        origin: vectorValue(feature.origin, parameters),
        direction: vectorValue(feature.direction, parameters),
        height1: expressionValue(feature.height1, parameters),
        height2: expressionValue(feature.height2, parameters),
        ...(feature.operation === undefined ? {} : { operation: feature.operation }),
        ...(feature.modify === undefined ? {} : { modify: feature.modify }),
      });
    case "glue":
      return scope.glue(
        shape(feature.newShape),
        shape(feature.baseShape),
        feature.faceBindings,
        feature.edgeBindings ?? [],
      );
    case "cylindricalHole": {
      const mode = feature.mode ?? "throughAll";
      const common = feature.withControl === undefined ? {} : { withControl: feature.withControl };
      if (mode === "blind") {
        if (feature.length === undefined) throw new TypeError(`Feature "${feature.id}" requires length`);
        const result = await scope.cylindricalHole(
          shape(feature.input),
          vectorValue(feature.origin, parameters),
          vectorValue(feature.direction, parameters),
          expressionValue(feature.radius, parameters),
          { mode, length: expressionValue(feature.length, parameters), ...common, includeHistory: true },
        );
        if (result.history === undefined) throw new Error(`Feature "${feature.id}" did not return topology history`);
        return { ...result, history: result.history, inputs: [feature.input] };
      }
      if (mode === "between") {
        if (feature.from === undefined || feature.to === undefined) {
          throw new TypeError(`Feature "${feature.id}" requires from and to`);
        }
        const result = await scope.cylindricalHole(
          shape(feature.input),
          vectorValue(feature.origin, parameters),
          vectorValue(feature.direction, parameters),
          expressionValue(feature.radius, parameters),
          {
            mode,
            from: expressionValue(feature.from, parameters),
            to: expressionValue(feature.to, parameters),
            ...common,
            includeHistory: true,
          },
        );
        if (result.history === undefined) throw new Error(`Feature "${feature.id}" did not return topology history`);
        return { ...result, history: result.history, inputs: [feature.input] };
      }
      const result = await scope.cylindricalHole(
        shape(feature.input),
        vectorValue(feature.origin, parameters),
        vectorValue(feature.direction, parameters),
        expressionValue(feature.radius, parameters),
        { mode, ...common, includeHistory: true },
      );
      if (result.history === undefined) throw new Error(`Feature "${feature.id}" did not return topology history`);
      return { ...result, history: result.history, inputs: [feature.input] };
    }
    case "defeature": {
      const result = await scope.defeature(
        shape(feature.input), subshapeIndices(feature.faceIndices, "face", feature.input),
        { includeHistory: true },
      );
      if (result.history === undefined) throw new Error(`Feature "${feature.id}" did not return topology history`);
      return { ...result, history: result.history, inputs: [feature.input] };
    }
    case "sew": {
      const result = await scope.sew(
        feature.shapes.map(shape),
        feature.tolerance === undefined ? undefined : expressionValue(feature.tolerance, parameters),
        { includeHistory: true },
      );
      return { ...result, inputs: feature.shapes };
    }
    case "fixShape": {
      const result = await scope.fixShape(
        shape(feature.input),
        feature.precision === undefined ? undefined : expressionValue(feature.precision, parameters),
        { includeHistory: true },
      );
      return { ...result, inputs: [feature.input] };
    }
    case "unifySameDomain": {
      const result = await scope.unifySameDomain(shape(feature.input), {
        ...(feature.unifyEdges === undefined ? {} : { unifyEdges: feature.unifyEdges }),
        ...(feature.unifyFaces === undefined ? {} : { unifyFaces: feature.unifyFaces }),
        ...(feature.concatBSplines === undefined ? {} : { concatBSplines: feature.concatBSplines }),
        includeHistory: true,
      });
      return { ...result, inputs: [feature.input] };
    }
    case "shapeUpgrade": {
      const input = shape(feature.input);
      if (feature.mode === "removeInternalWires") {
        return scope.shapeUpgrade(input, {
          mode: feature.mode,
          minArea: expressionValue(feature.minArea, parameters),
          ...(feature.removeFaces === undefined ? {} : { removeFaces: feature.removeFaces }),
        });
      }
      if (feature.mode === "removeLocations") {
        return scope.shapeUpgrade(input, {
          mode: feature.mode,
          ...(feature.removeLevel === undefined ? {} : { removeLevel: feature.removeLevel }),
        });
      }
      const divide = {
        ...(feature.precision === undefined ? {} : { precision: expressionValue(feature.precision, parameters) }),
        ...(feature.minTolerance === undefined ? {} : { minTolerance: expressionValue(feature.minTolerance, parameters) }),
        ...(feature.maxTolerance === undefined ? {} : { maxTolerance: expressionValue(feature.maxTolerance, parameters) }),
        ...(feature.surfaceSegmentMode === undefined ? {} : { surfaceSegmentMode: feature.surfaceSegmentMode }),
        ...("edgeMode" in feature && feature.edgeMode !== undefined ? { edgeMode: feature.edgeMode } : {}),
      };
      if (feature.mode === "angle") {
        return scope.shapeUpgrade(input, {
          mode: feature.mode,
          maxAngle: expressionValue(feature.maxAngle, parameters),
          ...divide,
        });
      }
      if (feature.mode === "area") {
        if (feature.areaMode === "maxArea") {
          return scope.shapeUpgrade(input, { mode: feature.mode, areaMode: feature.areaMode,
            maxArea: expressionValue(feature.maxArea, parameters), ...divide });
        }
        if (feature.areaMode === "parts") {
          return scope.shapeUpgrade(input, { mode: feature.mode, areaMode: feature.areaMode,
            nbParts: expressionValue(feature.nbParts, parameters), ...divide });
        }
        return scope.shapeUpgrade(input, { mode: feature.mode, areaMode: feature.areaMode,
          uSplits: expressionValue(feature.uSplits, parameters),
          vSplits: expressionValue(feature.vSplits, parameters), ...divide });
      }
      if (feature.mode === "closedFaces") {
        return scope.shapeUpgrade(input, { mode: feature.mode,
          ...(feature.splitPoints === undefined ? {} : { splitPoints: expressionValue(feature.splitPoints, parameters) }),
          ...divide });
      }
      if (feature.mode === "closedEdges") {
        return scope.shapeUpgrade(input, { mode: feature.mode, ...divide });
      }
      if (feature.mode === "convertToBezier") {
        return scope.shapeUpgrade(input, {
          mode: feature.mode,
          ...(feature.convert2d === undefined ? {} : { convert2d: feature.convert2d }),
          ...(feature.convert3d === undefined ? {} : { convert3d: feature.convert3d }),
          ...(feature.convertSurfaces === undefined ? {} : { convertSurfaces: feature.convertSurfaces }),
          ...(feature.convertLines === undefined ? {} : { convertLines: feature.convertLines }),
          ...(feature.convertCircles === undefined ? {} : { convertCircles: feature.convertCircles }),
          ...(feature.convertConics === undefined ? {} : { convertConics: feature.convertConics }),
          ...(feature.convertPlanes === undefined ? {} : { convertPlanes: feature.convertPlanes }),
          ...(feature.convertRevolutions === undefined ? {} : { convertRevolutions: feature.convertRevolutions }),
          ...(feature.convertExtrusions === undefined ? {} : { convertExtrusions: feature.convertExtrusions }),
          ...(feature.convertBSplines === undefined ? {} : { convertBSplines: feature.convertBSplines }),
          ...divide,
        });
      }
      return scope.shapeUpgrade(input, {
        mode: "continuity",
        ...(feature.boundaryCriterion === undefined ? {} : { boundaryCriterion: feature.boundaryCriterion }),
        ...(feature.pcurveCriterion === undefined ? {} : { pcurveCriterion: feature.pcurveCriterion }),
        ...(feature.surfaceCriterion === undefined ? {} : { surfaceCriterion: feature.surfaceCriterion }),
        ...(feature.tolerance === undefined ? {} : { tolerance: expressionValue(feature.tolerance, parameters) }),
        ...(feature.tolerance2d === undefined ? {} : { tolerance2d: expressionValue(feature.tolerance2d, parameters) }),
        ...divide,
      });
    }
    case "section":
      return scope.section(shape(feature.first), shape(feature.second));
    case "split":
      return scope.split(
        feature.objects.map(shape),
        (feature.tools ?? []).map(shape),
        {
          ...(feature.fuzzyValue === undefined ? {} : { fuzzyValue: expressionValue(feature.fuzzyValue, parameters) }),
          ...(feature.useOBB === undefined ? {} : { useOBB: feature.useOBB }),
        },
      );
    case "translate": {
      const result = await scope.translate(
        shape(feature.input), vectorValue(feature.translation, parameters), { includeHistory: true },
      );
      return { ...result, inputs: [feature.input] };
    }
    case "rotate": {
      const result = await scope.rotate(shape(feature.input), {
        angle: expressionValue(feature.angle, parameters),
        ...(feature.origin === undefined ? {} : { origin: vectorValue(feature.origin, parameters) }),
        ...(feature.direction === undefined ? {} : { direction: vectorValue(feature.direction, parameters) }),
      }, { includeHistory: true });
      return { ...result, inputs: [feature.input] };
    }
    case "scale": {
      const result = await scope.scale(
        shape(feature.input),
        expressionValue(feature.factor, parameters),
        feature.origin === undefined ? undefined : vectorValue(feature.origin, parameters),
        { includeHistory: true },
      );
      return { ...result, inputs: [feature.input] };
    }
    case "mirror": {
      const result = await scope.mirror(
        shape(feature.input), vectorValue(feature.normal, parameters), { includeHistory: true },
      );
      return { ...result, inputs: [feature.input] };
    }
    case "generalTransform": {
      const result = await scope.generalTransform(
        shape(feature.input), matrixValue(feature.matrix, parameters), { includeHistory: true },
      );
      return { ...result, inputs: [feature.input] };
    }
    case "booleanCut": {
      const result = await scope.booleanCut(shape(feature.base), feature.tools.map(shape), { includeHistory: true });
      if (result.history === undefined) throw new Error(`Feature "${feature.id}" did not return topology history`);
      return { ...result, history: result.history, inputs: [feature.base, ...feature.tools] };
    }
    case "booleanFuse": {
      const result = await scope.booleanFuse(shape(feature.base), feature.tools.map(shape), { includeHistory: true });
      if (result.history === undefined) throw new Error(`Feature "${feature.id}" did not return topology history`);
      return { ...result, history: result.history, inputs: [feature.base, ...feature.tools] };
    }
    case "booleanCommon": {
      const result = await scope.booleanCommon(shape(feature.base), feature.tools.map(shape), { includeHistory: true });
      if (result.history === undefined) throw new Error(`Feature "${feature.id}" did not return topology history`);
      return { ...result, history: result.history, inputs: [feature.base, ...feature.tools] };
    }
    default: {
      const exhaustive: never = feature;
      throw new TypeError(`Unsupported operation feature ${String(exhaustive)}`);
    }
  }
}
