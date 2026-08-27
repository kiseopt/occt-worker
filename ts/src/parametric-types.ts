import type { SketchConstraint } from "./sketch.js";
import type {
  HistoryTarget, RGBA, XCAFMaterial, XCAFValidationProperties,
  XCAFVisualMaterial,
} from "./types.js";

export type ParametricExpression = number | string;
export const PARAMETRIC_SCHEMA_VERSION = 1 as const;
export type ParametricVec3 = readonly [ParametricExpression, ParametricExpression, ParametricExpression];
export type ParametricPoint2 = readonly [ParametricExpression, ParametricExpression];
export type ParametricMatrix12 = readonly [
  ParametricExpression, ParametricExpression, ParametricExpression, ParametricExpression,
  ParametricExpression, ParametricExpression, ParametricExpression, ParametricExpression,
  ParametricExpression, ParametricExpression, ParametricExpression, ParametricExpression,
];

interface ParametricDocumentNodeBase {
  name?: string;
  transform?: ParametricMatrix12;
  color?: RGBA;
  layers?: readonly string[];
  visible?: boolean;
  material?: XCAFMaterial;
  visualMaterial?: XCAFVisualMaterial;
  validationProperties?: XCAFValidationProperties;
}

export type ParametricDocumentNode =
  | (ParametricDocumentNodeBase & { kind: "part"; feature: string })
  | (ParametricDocumentNodeBase & { kind: "assembly"; children?: readonly number[] });

export interface ParametricDocumentDefinition {
  nodes: readonly ParametricDocumentNode[];
  roots: readonly number[];
}

export type ParametricSketchEntity =
  | { id: string; type: "line"; start: ParametricPoint2; end: ParametricPoint2; construction?: boolean }
  | { id: string; type: "circle"; center: ParametricPoint2; radius: ParametricExpression; construction?: boolean }
  | {
      id: string;
      type: "arc";
      center: ParametricPoint2;
      radius: ParametricExpression;
      startAngle: ParametricExpression;
      endAngle: ParametricExpression;
      construction?: boolean;
    }
  | {
      id: string;
      type: "spline";
      poles: readonly ParametricPoint2[];
      degree?: number;
      construction?: boolean;
    };

export type ParametricSketchConstraint =
  | Exclude<SketchConstraint, { type: "fixed" | "distance" | "length" | "radius" }>
  | { type: "fixed"; point: { entity: string; point: "start" | "end" | "center" }; value: ParametricPoint2 }
  | { type: "distance"; first: { entity: string; point: "start" | "end" | "center" }; second: { entity: string; point: "start" | "end" | "center" }; value: ParametricExpression }
  | { type: "length"; entity: string; value: ParametricExpression }
  | { type: "radius"; entity: string; value: ParametricExpression };

export interface ParametricSketchProfile {
  id: string;
  entities: readonly string[];
}

interface FeatureBase {
  id: string;
  /** Suppressed features remain serializable but are omitted from recompute. */
  suppressed?: boolean;
}

export type FeatureDiagnosticStatus = "ok" | "suppressed" | "failed";

export interface FeatureDiagnostic {
  id: string;
  type: ParametricFeature["type"];
  status: FeatureDiagnosticStatus;
  message?: string;
}

export interface ParametricSubshapeSignature {
  geometry: string;
  normalizedBounds: readonly [number, number, number, number, number, number];
}

interface ParametricSubshapeReferenceBase {
  id: string;
  feature: string;
  type: "face" | "edge";
}

export type ParametricSubshapeReference = ParametricSubshapeReferenceBase & (
  | {
      /** Used only to capture the first persistent signature. */
      initialIndex: number;
      source?: never;
      /** Explicit deterministic tie-breaker for symmetric equal-signature topology. */
      disambiguation?: "initialIndex";
      /** Allow a unique normalized-bounds match when the curve or surface family changes. */
      allowGeometryReplacement?: boolean;
      signature?: ParametricSubshapeSignature;
      tolerance?: number;
    }
  | {
      /** Propagate another reference through OCCT histories to this feature output. */
      source: string;
      initialIndex?: never;
      disambiguation?: never;
      allowGeometryReplacement?: never;
      signature?: never;
      tolerance?: never;
    }
);

export interface SubshapeReferenceDiagnostic {
  id: string;
  feature: string;
  type: "face" | "edge";
  status: "resolved" | "missing" | "ambiguous";
  index?: number;
  candidateIndices?: number[];
  message?: string;
}

export type PersistentSubshapeResolutionStatus = "resolved" | "missing" | "ambiguous";

export interface PersistentSubshapeResolution {
  status: PersistentSubshapeResolutionStatus;
  type?: "face" | "edge";
  index?: number;
  kind?: "retained" | "modified" | "generated";
  /** Operation in the supplied chain where resolution stopped, zero based. */
  stage?: number;
  reason?: "deleted" | "unmapped" | "multipleTargets";
  candidates?: readonly HistoryTarget[];
}

export type ParametricSubshapeSelector = number | { reference: string };

export interface OrientedFeature {
  origin?: ParametricVec3;
  direction?: ParametricVec3;
}

export type ParametricFeature =
  | (FeatureBase & { type: "box"; size: ParametricVec3; origin?: ParametricVec3 })
  | (FeatureBase & OrientedFeature & { type: "cylinder"; radius: ParametricExpression; height: ParametricExpression })
  | (FeatureBase & OrientedFeature & { type: "sphere"; radius: ParametricExpression })
  | (FeatureBase & OrientedFeature & { type: "cone"; radius1: ParametricExpression; radius2: ParametricExpression; height: ParametricExpression })
  | (FeatureBase & OrientedFeature & { type: "torus"; majorRadius: ParametricExpression; minorRadius: ParametricExpression })
  | (FeatureBase & OrientedFeature & { type: "wedge"; size: ParametricVec3; ltx: ParametricExpression })
  | (FeatureBase & { type: "polygon"; points: readonly ParametricVec3[]; close?: boolean })
  | (FeatureBase & {
      type: "bezierCurve";
      poles: readonly ParametricVec3[];
      weights?: readonly ParametricExpression[];
    })
  | (FeatureBase & {
      type: "bsplineCurve";
      poles: readonly ParametricVec3[];
      degree?: ParametricExpression;
      periodic?: boolean;
      tolerance?: ParametricExpression;
      knots?: readonly ParametricExpression[];
      multiplicities?: readonly ParametricExpression[];
      weights?: readonly ParametricExpression[];
    })
  | (FeatureBase & {
      type: "bezierSurface";
      poles: readonly (readonly ParametricVec3[])[];
      weights?: readonly (readonly ParametricExpression[])[];
      tolerance?: ParametricExpression;
    })
  | (FeatureBase & {
      type: "bsplineSurface";
      poles: readonly (readonly ParametricVec3[])[];
      weights?: readonly (readonly ParametricExpression[])[];
      uDegree?: ParametricExpression;
      vDegree?: ParametricExpression;
      uKnots?: readonly ParametricExpression[];
      vKnots?: readonly ParametricExpression[];
      uMultiplicities?: readonly ParametricExpression[];
      vMultiplicities?: readonly ParametricExpression[];
      uPeriodic?: boolean;
      vPeriodic?: boolean;
      tolerance?: ParametricExpression;
    })
  | (FeatureBase & {
      type: "sketch";
      entities: readonly ParametricSketchEntity[];
      constraints?: readonly ParametricSketchConstraint[];
      profiles?: readonly ParametricSketchProfile[];
      origin?: ParametricVec3;
      normal?: ParametricVec3;
      xDirection?: ParametricVec3;
      tolerance?: ParametricExpression;
      maxIterations?: number;
    })
  | (FeatureBase & { type: "face"; outer: string; holes?: readonly string[] })
  | (FeatureBase & { type: "extrude"; input: string; vector: ParametricVec3 })
  | (FeatureBase & { type: "revolve"; input: string; origin: ParametricVec3; direction: ParametricVec3; angle?: ParametricExpression })
  | (FeatureBase & { type: "loft"; sections: readonly string[]; solid?: boolean; ruled?: boolean; precision?: ParametricExpression })
  | (FeatureBase & { type: "sweepPipe"; spine: string; profile: string })
  | (FeatureBase & {
      type: "sweepPipeShell";
      spine: string;
      profiles: readonly string[];
      mode?: "frenet" | "correctedFrenet" | "fixedAxis" | "auxiliarySpine";
      axis?: { origin?: ParametricVec3; direction: ParametricVec3; xDirection?: ParametricVec3 };
      auxiliarySpine?: string;
      curvilinearEquivalence?: boolean;
    })
  | (FeatureBase & {
      type: "fillet";
      input: string;
      edgeIndices: readonly ParametricSubshapeSelector[];
      radius: ParametricExpression;
      radii?: readonly ParametricExpression[];
      radius2?: ParametricExpression;
      radiusLaw?: readonly { parameter: ParametricExpression; radius: ParametricExpression }[];
      radiusLaws?: readonly (readonly { parameter: ParametricExpression; radius: ParametricExpression }[])[];
    })
  | (FeatureBase & {
      type: "chamfer";
      input: string;
      edgeIndices: readonly ParametricSubshapeSelector[];
      distance: ParametricExpression;
      distances?: readonly ParametricExpression[];
      distance2?: ParametricExpression;
      distances2?: readonly ParametricExpression[];
      referenceFaceIndices?: readonly ParametricSubshapeSelector[];
    })
  | (FeatureBase & { type: "hollow"; input: string; thickness: ParametricExpression; closingFaceIndices?: readonly ParametricSubshapeSelector[] })
  | (FeatureBase & { type: "offsetShape"; input: string; offset: ParametricExpression; tolerance?: ParametricExpression })
  | (FeatureBase & { type: "offsetWire2D"; input: string; offset: ParametricExpression })
  | (FeatureBase & {
      type: "draftAngle";
      input: string;
      faceIndices: readonly ParametricSubshapeSelector[];
      direction: ParametricVec3;
      angle: ParametricExpression;
      neutralPlane: { origin: ParametricVec3; normal: ParametricVec3 };
      flag?: boolean;
    })
  | (FeatureBase & {
      type: "localPrism";
      base: string;
      faceIndices: readonly ParametricSubshapeSelector[];
      direction: ParametricVec3;
      operation?: "add" | "cut";
    } & (
      | { mode?: "length"; length: ParametricExpression }
      | { mode: "until"; until: string }
      | { mode: "fromUntil"; from: string; until: string }
      | { mode: "untilEnd" | "thruAll" }
      | { mode: "fromEnd"; from: string }
    ))
  | (FeatureBase & {
      type: "localRevolution";
      base: string;
      faceIndices: readonly ParametricSubshapeSelector[];
      origin: ParametricVec3;
      direction: ParametricVec3;
      operation?: "add" | "cut";
    } & (
      | { mode?: "angle"; angle: ParametricExpression }
      | { mode: "until"; until: string }
      | { mode: "fromUntil"; from: string; until: string }
      | { mode: "thruAll" }
    ))
  | (FeatureBase & {
      type: "linearForm";
      base: string;
      profile: string;
      planeOrigin: ParametricVec3;
      planeNormal: ParametricVec3;
      direction: ParametricVec3;
      direction1?: ParametricVec3;
      operation?: "add" | "cut";
      modify?: boolean;
    })
  | (FeatureBase & {
      type: "revolutionForm";
      base: string;
      profile: string;
      planeOrigin: ParametricVec3;
      planeNormal: ParametricVec3;
      origin: ParametricVec3;
      direction: ParametricVec3;
      height1: ParametricExpression;
      height2: ParametricExpression;
      operation?: "add" | "cut";
      modify?: boolean;
    })
  | (FeatureBase & {
      type: "glue";
      newShape: string;
      baseShape: string;
      faceBindings: readonly { newIndex: number; baseIndex: number }[];
      edgeBindings?: readonly { newIndex: number; baseIndex: number }[];
    })
  | (FeatureBase & {
      type: "cylindricalHole";
      input: string;
      origin: ParametricVec3;
      direction: ParametricVec3;
      radius: ParametricExpression;
      mode?: "throughAll" | "throughNext" | "untilEnd" | "blind" | "between";
      length?: ParametricExpression;
      from?: ParametricExpression;
      to?: ParametricExpression;
      withControl?: boolean;
    })
  | (FeatureBase & { type: "defeature"; input: string; faceIndices: readonly ParametricSubshapeSelector[] })
  | (FeatureBase & { type: "sew"; shapes: readonly string[]; tolerance?: ParametricExpression })
  | (FeatureBase & { type: "fixShape"; input: string; precision?: ParametricExpression })
  | (FeatureBase & {
      type: "unifySameDomain";
      input: string;
      unifyEdges?: boolean;
      unifyFaces?: boolean;
      concatBSplines?: boolean;
    })
  | (FeatureBase & { type: "shapeUpgrade"; input: string } & (
      | {
          mode?: "continuity";
          boundaryCriterion?: "c0" | "c1" | "c2" | "c3" | "cn";
          pcurveCriterion?: "c0" | "c1" | "c2" | "c3" | "cn";
          surfaceCriterion?: "c0" | "c1" | "c2" | "c3" | "cn";
          tolerance?: ParametricExpression;
          tolerance2d?: ParametricExpression;
          precision?: ParametricExpression;
          minTolerance?: ParametricExpression;
          maxTolerance?: ParametricExpression;
          surfaceSegmentMode?: boolean;
          edgeMode?: 0 | 1 | 2;
        }
      | { mode: "angle"; maxAngle: ParametricExpression; precision?: ParametricExpression; minTolerance?: ParametricExpression; maxTolerance?: ParametricExpression; surfaceSegmentMode?: boolean }
      | ({ mode: "area"; precision?: ParametricExpression; minTolerance?: ParametricExpression; maxTolerance?: ParametricExpression; surfaceSegmentMode?: boolean } & (
          | { areaMode: "maxArea"; maxArea: ParametricExpression }
          | { areaMode: "parts"; nbParts: ParametricExpression }
          | { areaMode: "uv"; uSplits: ParametricExpression; vSplits: ParametricExpression }
        ))
      | { mode: "closedFaces"; splitPoints?: ParametricExpression; precision?: ParametricExpression; minTolerance?: ParametricExpression; maxTolerance?: ParametricExpression; surfaceSegmentMode?: boolean }
      | { mode: "closedEdges"; precision?: ParametricExpression; minTolerance?: ParametricExpression; maxTolerance?: ParametricExpression; surfaceSegmentMode?: boolean; edgeMode?: 0 | 1 | 2 }
      | {
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
          precision?: ParametricExpression;
          minTolerance?: ParametricExpression;
          maxTolerance?: ParametricExpression;
          surfaceSegmentMode?: boolean;
          edgeMode?: 0 | 1 | 2;
        }
      | { mode: "removeInternalWires"; minArea: ParametricExpression; removeFaces?: boolean }
      | { mode: "removeLocations"; removeLevel?: "shape" | "compound" | "solid" | "shell" | "face" }
    ))
  | (FeatureBase & { type: "section"; first: string; second: string })
  | (FeatureBase & {
      type: "split";
      objects: readonly string[];
      tools?: readonly string[];
      fuzzyValue?: ParametricExpression;
      useOBB?: boolean;
    })
  | (FeatureBase & { type: "translate"; input: string; translation: ParametricVec3 })
  | (FeatureBase & { type: "rotate"; input: string; angle: ParametricExpression; origin?: ParametricVec3; direction?: ParametricVec3 })
  | (FeatureBase & { type: "scale"; input: string; factor: ParametricExpression; origin?: ParametricVec3 })
  | (FeatureBase & { type: "mirror"; input: string; normal: ParametricVec3 })
  | (FeatureBase & { type: "generalTransform"; input: string; matrix: ParametricMatrix12 })
  | (FeatureBase & { type: "booleanCut" | "booleanFuse" | "booleanCommon"; base: string; tools: readonly string[] });

export type PrimitiveParametricFeature = Extract<ParametricFeature, { type:
  | "box" | "cylinder" | "sphere" | "cone" | "torus" | "wedge" | "polygon"
  | "bezierCurve" | "bsplineCurve" | "bezierSurface" | "bsplineSurface"
}>;
export type SketchParametricFeature = Extract<ParametricFeature, { type: "sketch" | "face" }>;
export type OperationParametricFeature = Exclude<ParametricFeature, PrimitiveParametricFeature | SketchParametricFeature>;

export interface ParametricDefinition {
  schemaVersion?: typeof PARAMETRIC_SCHEMA_VERSION;
  parameters: Record<string, ParametricExpression>;
  features: ParametricFeature[];
  document?: ParametricDocumentDefinition;
  subshapeReferences?: ParametricSubshapeReference[];
}
