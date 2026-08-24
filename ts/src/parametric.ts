import { type BaseClient, type ShapeHandle, type ShapeScope } from "./client.js";
import { solveSketch, type SketchConstraint, type SketchEntity } from "./sketch.js";
import type {
  RGBA, STEPDocumentDefinition, XCAFMaterial, XCAFValidationProperties,
  XCAFVisualMaterial, Vec3, History, HistorySource, HistoryTarget,
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

interface PersistentHistoryStage {
  history: History;
  input: number;
}

function resolvePersistentSubshapeStages(
  source: Pick<HistorySource, "type" | "index">,
  stages: readonly PersistentHistoryStage[],
): PersistentSubshapeResolution {
  let current = { ...source };
  let kind: PersistentSubshapeResolution["kind"] = "retained";
  for (let stage = 0; stage < stages.length; stage += 1) {
    const { history, input } = stages[stage]!;
    const targets = new Map<string, { target: HistoryTarget; kind: NonNullable<PersistentSubshapeResolution["kind"]> }>();
    const add = (
      entries: readonly { from: HistorySource; to: HistoryTarget | readonly HistoryTarget[] }[],
      entryKind: NonNullable<PersistentSubshapeResolution["kind"]>,
    ): void => {
      for (const entry of entries) {
        if (entry.from.input !== input || entry.from.type !== current.type || entry.from.index !== current.index) continue;
        const mapped = Array.isArray(entry.to) ? entry.to : [entry.to];
        for (const target of mapped) {
          const key = `${target.type}:${target.index}`;
          const previous = targets.get(key);
          if (previous === undefined || (previous.kind === "retained" && entryKind !== "retained")) {
            targets.set(key, { target, kind: entryKind });
          }
        }
      }
    };
    add(history.retained, "retained");
    add(history.modified, "modified");
    add(history.generated, "generated");
    if (targets.size === 0) {
      const deleted = history.deleted.some((entry) =>
        entry.input === input && entry.type === current.type && entry.index === current.index);
      return { status: "missing", stage, reason: deleted ? "deleted" : "unmapped" };
    }
    if (targets.size !== 1) {
      return {
        status: "ambiguous",
        stage,
        reason: "multipleTargets",
        candidates: [...targets.values()].map(({ target }) => target),
      };
    }
    const mapped = targets.values().next().value!;
    current = mapped.target;
    kind = mapped.kind;
  }
  return { status: "resolved", type: current.type, index: current.index, kind };
}

/**
 * Propagates one subshape reference through local OCCT histories.
 * Each history after the first consumes the previous operation's output as input 0.
 * A one-to-many generated/modified mapping is deliberately reported as ambiguous.
 */
export function resolvePersistentSubshape(
  source: HistorySource,
  historyChain: readonly History[],
): PersistentSubshapeResolution {
  return resolvePersistentSubshapeStages(source, historyChain.map((history, stage) => ({
    history,
    input: stage === 0 ? source.input : 0,
  })));
}

export type ParametricSubshapeSelector = number | { reference: string };

interface OrientedFeature {
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

export interface ParametricDefinition {
  schemaVersion?: typeof PARAMETRIC_SCHEMA_VERSION;
  parameters: Record<string, ParametricExpression>;
  features: ParametricFeature[];
  document?: ParametricDocumentDefinition;
  subshapeReferences?: ParametricSubshapeReference[];
}

type ArithmeticNode =
  | { type: "number"; value: number }
  | { type: "name"; name: string }
  | { type: "unary"; operator: "+" | "-"; value: ArithmeticNode }
  | { type: "binary"; operator: "+" | "-" | "*" | "/" | "^"; left: ArithmeticNode; right: ArithmeticNode };

class ArithmeticParser {
  readonly #source: string;
  #offset = 0;

  constructor(source: string) {
    this.#source = source;
  }

  parse(): ArithmeticNode {
    const result = this.#parseAddition();
    this.#skipWhitespace();
    if (this.#offset !== this.#source.length) this.#fail("Unexpected token");
    return result;
  }

  #parseAddition(): ArithmeticNode {
    let left = this.#parseMultiplication();
    while (true) {
      if (this.#consume("+")) left = { type: "binary", operator: "+", left, right: this.#parseMultiplication() };
      else if (this.#consume("-")) left = { type: "binary", operator: "-", left, right: this.#parseMultiplication() };
      else return left;
    }
  }

  #parseMultiplication(): ArithmeticNode {
    let left = this.#parseUnary();
    while (true) {
      if (this.#consume("*")) left = { type: "binary", operator: "*", left, right: this.#parseUnary() };
      else if (this.#consume("/")) left = { type: "binary", operator: "/", left, right: this.#parseUnary() };
      else return left;
    }
  }

  #parseUnary(): ArithmeticNode {
    if (this.#consume("+")) return { type: "unary", operator: "+", value: this.#parseUnary() };
    if (this.#consume("-")) return { type: "unary", operator: "-", value: this.#parseUnary() };
    return this.#parsePower();
  }

  #parsePower(): ArithmeticNode {
    const left = this.#parsePrimary();
    return this.#consume("^")
      ? { type: "binary", operator: "^", left, right: this.#parseUnary() }
      : left;
  }

  #parsePrimary(): ArithmeticNode {
    this.#skipWhitespace();
    if (this.#consume("(")) {
      const value = this.#parseAddition();
      if (!this.#consume(")")) this.#fail("Expected ')'");
      return value;
    }
    const remaining = this.#source.slice(this.#offset);
    const number = /^(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?/.exec(remaining)?.[0];
    if (number !== undefined) {
      this.#offset += number.length;
      return { type: "number", value: Number(number) };
    }
    const name = /^[A-Za-z_][A-Za-z0-9_]*/.exec(remaining)?.[0];
    if (name !== undefined) {
      this.#offset += name.length;
      return { type: "name", name };
    }
    this.#fail("Expected a number, parameter, or '('");
  }

  #consume(token: string): boolean {
    this.#skipWhitespace();
    if (!this.#source.startsWith(token, this.#offset)) return false;
    this.#offset += token.length;
    return true;
  }

  #skipWhitespace(): void {
    while (/\s/.test(this.#source[this.#offset] ?? "")) this.#offset++;
  }

  #fail(message: string): never {
    throw new SyntaxError(`${message} at offset ${this.#offset} in "${this.#source}"`);
  }
}

function evaluateNode(node: ArithmeticNode, resolve: (name: string) => number): number {
  let value: number;
  if (node.type === "number") value = node.value;
  else if (node.type === "name") {
    if (node.name === "pi") value = Math.PI;
    else if (node.name === "e") value = Math.E;
    else value = resolve(node.name);
  } else if (node.type === "unary") {
    value = node.operator === "-" ? -evaluateNode(node.value, resolve) : evaluateNode(node.value, resolve);
  } else {
    const left = evaluateNode(node.left, resolve);
    const right = evaluateNode(node.right, resolve);
    if (node.operator === "+") value = left + right;
    else if (node.operator === "-") value = left - right;
    else if (node.operator === "*") value = left * right;
    else if (node.operator === "/") {
      if (right === 0) throw new RangeError("Division by zero in parametric expression");
      value = left / right;
    } else value = left ** right;
  }
  if (!Number.isFinite(value)) throw new RangeError("Parametric expression produced a non-finite number");
  return value;
}

function parseArithmetic(expression: ParametricExpression): ArithmeticNode {
  if (typeof expression === "number") {
    if (!Number.isFinite(expression)) throw new RangeError("Parametric values must be finite");
    return { type: "number", value: expression };
  }
  if (expression.trim() === "") throw new SyntaxError("Parametric expression cannot be empty");
  return new ArithmeticParser(expression).parse();
}

export function evaluateExpression(
  expression: ParametricExpression,
  parameters: Readonly<Record<string, number>> = {},
): number {
  return evaluateNode(parseArithmetic(expression), (name) => {
    if (!Object.prototype.hasOwnProperty.call(parameters, name)) throw new ReferenceError(`Unknown parameter "${name}"`);
    const value = parameters[name]!;
    return value;
  });
}

function assertParameterName(name: string): void {
  if (!/^[A-Za-z_][A-Za-z0-9_]*$/.test(name) || name === "pi" || name === "e"
      || name === "__proto__" || name === "prototype" || name === "constructor") {
    throw new TypeError(`Invalid parameter name "${name}"`);
  }
}

export function resolveParameters(
  definitions: Readonly<Record<string, ParametricExpression>>,
): Readonly<Record<string, number>> {
  const nodes = new Map<string, ArithmeticNode>();
  for (const [name, expression] of Object.entries(definitions)) {
    assertParameterName(name);
    nodes.set(name, parseArithmetic(expression));
  }
  const values = new Map<string, number>();
  const visiting: string[] = [];
  const resolve = (name: string): number => {
    const known = values.get(name);
    if (known !== undefined) return known;
    const node = nodes.get(name);
    if (node === undefined) {
      const owner = visiting.at(-1);
      throw new ReferenceError(owner === undefined
        ? `Unknown parameter "${name}"`
        : `Unknown parameter "${name}" referenced by "${owner}"`);
    }
    const cycleStart = visiting.indexOf(name);
    if (cycleStart !== -1) {
      throw new Error(`Parameter dependency cycle: ${[...visiting.slice(cycleStart), name].join(" -> ")}`);
    }
    visiting.push(name);
    try {
      const value = evaluateNode(node, resolve);
      values.set(name, value);
      return value;
    } finally {
      visiting.pop();
    }
  };
  for (const name of nodes.keys()) resolve(name);
  return Object.freeze(Object.fromEntries(values));
}

function cloneDefinition(definition: ParametricDefinition): ParametricDefinition {
  return structuredClone(definition);
}

function migrateDefinition(definition: ParametricDefinition): ParametricDefinition {
  if (definition.schemaVersion !== undefined
      && definition.schemaVersion !== PARAMETRIC_SCHEMA_VERSION) {
    throw new RangeError(`Unsupported parametric schema version: ${String(definition.schemaVersion)}`);
  }
  return { ...cloneDefinition(definition), schemaVersion: PARAMETRIC_SCHEMA_VERSION };
}

function featureDependencies(feature: ParametricFeature): readonly string[] {
  if (feature.type === "extrude" || feature.type === "revolve" || feature.type === "translate"
      || feature.type === "rotate" || feature.type === "scale" || feature.type === "mirror"
      || feature.type === "generalTransform" || feature.type === "fillet"
      || feature.type === "chamfer" || feature.type === "hollow" || feature.type === "offsetShape"
      || feature.type === "offsetWire2D" || feature.type === "draftAngle") return [feature.input];
  if (feature.type === "face") return [feature.outer, ...(feature.holes ?? [])];
  if (feature.type === "loft") return feature.sections;
  if (feature.type === "sweepPipe") return [feature.spine, feature.profile];
  if (feature.type === "sweepPipeShell") {
    return [feature.spine, ...feature.profiles,
      ...(feature.auxiliarySpine === undefined ? [] : [feature.auxiliarySpine])];
  }
  if (feature.type === "localPrism") {
    return [feature.base,
      ...("from" in feature ? [feature.from] : []),
      ...("until" in feature ? [feature.until] : [])];
  }
  if (feature.type === "localRevolution") {
    return [feature.base,
      ...("from" in feature ? [feature.from] : []),
      ...("until" in feature ? [feature.until] : [])];
  }
  if (feature.type === "linearForm" || feature.type === "revolutionForm") {
    return [feature.base, feature.profile];
  }
  if (feature.type === "glue") return [feature.newShape, feature.baseShape];
  if (feature.type === "sew") return feature.shapes;
  if (feature.type === "fixShape" || feature.type === "unifySameDomain" || feature.type === "shapeUpgrade"
      || feature.type === "cylindricalHole" || feature.type === "defeature") return [feature.input];
  if (feature.type === "section") return [feature.first, feature.second];
  if (feature.type === "split") return [...feature.objects, ...(feature.tools ?? [])];
  if (feature.type === "booleanCut" || feature.type === "booleanFuse" || feature.type === "booleanCommon") {
    return [feature.base, ...feature.tools];
  }
  return [];
}

function dependencyOwner(id: string): string {
  const separator = id.indexOf(".");
  return separator === -1 ? id : id.slice(0, separator);
}

function orderFeatures(features: readonly ParametricFeature[]): ParametricFeature[] {
  const supportedTypes = new Set<ParametricFeature["type"]>([
    "box", "cylinder", "sphere", "cone", "torus", "wedge", "extrude", "revolve",
    "polygon", "bezierCurve", "bsplineCurve", "bezierSurface", "bsplineSurface",
    "sketch", "face", "loft", "sweepPipe", "sweepPipeShell", "fillet", "chamfer",
    "hollow", "offsetShape", "offsetWire2D", "draftAngle", "localPrism", "localRevolution", "linearForm",
    "revolutionForm",
    "glue",
    "cylindricalHole", "defeature",
    "sew", "fixShape", "unifySameDomain", "shapeUpgrade", "section", "split",
    "translate", "rotate", "scale", "mirror", "generalTransform",
    "booleanCut", "booleanFuse", "booleanCommon",
  ]);
  const byId = new Map<string, ParametricFeature>();
  for (const feature of features) {
    if (typeof feature.id !== "string" || !/^[A-Za-z_][A-Za-z0-9_-]*$/.test(feature.id)) {
      throw new TypeError(`Invalid feature id "${String(feature.id)}"`);
    }
    if (!supportedTypes.has(feature.type)) throw new TypeError(`Unsupported feature type "${String(feature.type)}"`);
    if ((feature.type === "bezierCurve" || feature.type === "bsplineCurve"
        || feature.type === "bezierSurface" || feature.type === "bsplineSurface")
        && (!Array.isArray(feature.poles) || feature.poles.length === 0)) {
      throw new TypeError(`Feature "${feature.id}" requires control points`);
    }
    if ((feature.type === "extrude" || feature.type === "revolve" || feature.type === "translate"
        || feature.type === "rotate" || feature.type === "scale" || feature.type === "mirror"
        || feature.type === "generalTransform" || feature.type === "fillet"
        || feature.type === "chamfer" || feature.type === "hollow" || feature.type === "offsetShape"
        || feature.type === "offsetWire2D" || feature.type === "draftAngle")
        && typeof feature.input !== "string") {
      throw new TypeError(`Feature "${feature.id}" requires an input feature id`);
    }
    if (feature.type === "face"
        && (typeof feature.outer !== "string" || (feature.holes !== undefined
          && (!Array.isArray(feature.holes) || feature.holes.some((hole) => typeof hole !== "string"))))) {
      throw new TypeError(`Feature "${feature.id}" requires outer and hole feature ids`);
    }
    if (feature.type === "sketch"
        && (!Array.isArray(feature.entities) || feature.entities.length === 0)) {
      throw new TypeError(`Feature "${feature.id}" requires sketch entities`);
    }
    if (feature.type === "sketch" && feature.profiles !== undefined) {
      if (!Array.isArray(feature.profiles) || feature.profiles.length === 0) {
        throw new TypeError(`Feature "${feature.id}" profiles must not be empty`);
      }
      const entityIds = new Set(feature.entities.map((entity) => entity.id));
      const profileIds = new Set<string>();
      const assigned = new Set<string>();
      for (const profile of feature.profiles) {
        if (typeof profile.id !== "string" || !/^[A-Za-z_][A-Za-z0-9_-]*$/.test(profile.id)
            || profileIds.has(profile.id)) {
          throw new TypeError(`Feature "${feature.id}" has an invalid or duplicate sketch profile id`);
        }
        if (!Array.isArray(profile.entities) || profile.entities.length === 0) {
          throw new TypeError(`Sketch profile "${feature.id}.${profile.id}" requires entity ids`);
        }
        profileIds.add(profile.id);
        for (const entityId of profile.entities) {
          if (typeof entityId !== "string" || !entityIds.has(entityId)
              || assigned.has(entityId)
              || feature.entities.find((entity) => entity.id === entityId)?.construction) {
            throw new TypeError(`Sketch profile "${feature.id}.${profile.id}" has an invalid or reused entity id`);
          }
          assigned.add(entityId);
        }
      }
      for (const entity of feature.entities) {
        if (!entity.construction && !assigned.has(entity.id)) {
          throw new TypeError(`Sketch entity "${feature.id}.${entity.id}" is not assigned to a profile`);
        }
      }
    }
    if (feature.type === "loft"
        && (!Array.isArray(feature.sections) || feature.sections.some((section) => typeof section !== "string"))) {
      throw new TypeError(`Feature "${feature.id}" requires section feature ids`);
    }
    if (feature.type === "sweepPipe"
        && (typeof feature.spine !== "string" || typeof feature.profile !== "string")) {
      throw new TypeError(`Feature "${feature.id}" requires spine and profile feature ids`);
    }
    if (feature.type === "sweepPipeShell"
        && (typeof feature.spine !== "string" || !Array.isArray(feature.profiles)
          || feature.profiles.some((profile) => typeof profile !== "string")
          || (feature.auxiliarySpine !== undefined && typeof feature.auxiliarySpine !== "string"))) {
      throw new TypeError(`Feature "${feature.id}" requires spine and profile feature ids`);
    }
    if (feature.type === "localPrism"
        && (typeof feature.base !== "string"
          || ("from" in feature && typeof feature.from !== "string")
          || ("until" in feature && typeof feature.until !== "string"))) {
      throw new TypeError(`Feature "${feature.id}" requires valid base and limit feature ids`);
    }
    if (feature.type === "localRevolution"
        && (typeof feature.base !== "string"
          || ("from" in feature && typeof feature.from !== "string")
          || ("until" in feature && typeof feature.until !== "string"))) {
      throw new TypeError(`Feature "${feature.id}" requires valid base and limit feature ids`);
    }
    if ((feature.type === "linearForm" || feature.type === "revolutionForm")
        && (typeof feature.base !== "string" || typeof feature.profile !== "string")) {
      throw new TypeError(`Feature "${feature.id}" requires base and profile feature ids`);
    }
    if (feature.type === "glue"
        && (typeof feature.newShape !== "string" || typeof feature.baseShape !== "string"
          || !Array.isArray(feature.faceBindings) || feature.faceBindings.length === 0)) {
      throw new TypeError(`Feature "${feature.id}" requires shape ids and face bindings`);
    }
    if ((feature.type === "cylindricalHole" || feature.type === "defeature")
        && typeof feature.input !== "string") {
      throw new TypeError(`Feature "${feature.id}" requires an input feature id`);
    }
    if (feature.type === "sew"
        && (!Array.isArray(feature.shapes) || feature.shapes.length === 0
          || feature.shapes.some((shape) => typeof shape !== "string"))) {
      throw new TypeError(`Feature "${feature.id}" requires shape feature ids`);
    }
    if ((feature.type === "fixShape" || feature.type === "unifySameDomain" || feature.type === "shapeUpgrade")
        && typeof feature.input !== "string") {
      throw new TypeError(`Feature "${feature.id}" requires an input feature id`);
    }
    if (feature.type === "section"
        && (typeof feature.first !== "string" || typeof feature.second !== "string")) {
      throw new TypeError(`Feature "${feature.id}" requires two input feature ids`);
    }
    if (feature.type === "split"
        && (!Array.isArray(feature.objects) || feature.objects.length === 0
          || feature.objects.some((object) => typeof object !== "string")
          || (feature.tools !== undefined
            && (!Array.isArray(feature.tools) || feature.tools.some((tool) => typeof tool !== "string"))))) {
      throw new TypeError(`Feature "${feature.id}" requires object and tool feature ids`);
    }
    if (feature.type === "booleanCut" || feature.type === "booleanFuse" || feature.type === "booleanCommon") {
      if (typeof feature.base !== "string" || !Array.isArray(feature.tools)
          || feature.tools.some((tool) => typeof tool !== "string")) {
        throw new TypeError(`Feature "${feature.id}" requires base and tools feature ids`);
      }
    }
    if (byId.has(feature.id)) throw new Error(`Duplicate feature id "${feature.id}"`);
    byId.set(feature.id, feature);
  }
  for (const feature of features) {
    for (const dependency of featureDependencies(feature)) {
      const separator = dependency.indexOf(".");
      if (separator === -1) continue;
      const ownerId = dependency.slice(0, separator);
      const profileId = dependency.slice(separator + 1);
      const owner = byId.get(ownerId);
      if (owner?.type !== "sketch"
          || owner.profiles?.some((profile) => profile.id === profileId) !== true) {
        throw new ReferenceError(`Unknown sketch profile "${dependency}" referenced by "${feature.id}"`);
      }
    }
  }
  const ordered: ParametricFeature[] = [];
  const state = new Map<string, "visiting" | "done">();
  const path: string[] = [];
  const visit = (id: string): void => {
    const current = state.get(id);
    if (current === "done") return;
    if (current === "visiting") {
      const start = path.indexOf(id);
      throw new Error(`Feature dependency cycle: ${[...path.slice(start), id].join(" -> ")}`);
    }
    const feature = byId.get(id);
    if (feature === undefined) {
      const owner = path.at(-1);
      throw new ReferenceError(owner === undefined
        ? `Unknown feature "${id}"`
        : `Unknown feature "${id}" referenced by "${owner}"`);
    }
    state.set(id, "visiting");
    path.push(id);
    try {
      for (const dependency of featureDependencies(feature)) visit(dependencyOwner(dependency));
      state.set(id, "done");
      ordered.push(feature);
    } finally {
      path.pop();
    }
  };
  for (const feature of features) visit(feature.id);
  return ordered;
}

function expressionValue(expression: ParametricExpression, parameters: Readonly<Record<string, number>>): number {
  return evaluateExpression(expression, parameters);
}

function vectorValue(value: ParametricVec3, parameters: Readonly<Record<string, number>>): Vec3 {
  if (!Array.isArray(value) || value.length !== 3) throw new TypeError("Parametric vector must contain three expressions");
  return [
    expressionValue(value[0], parameters),
    expressionValue(value[1], parameters),
    expressionValue(value[2], parameters),
  ];
}

function point2Value(value: ParametricPoint2, parameters: Readonly<Record<string, number>>): [number, number] {
  if (!Array.isArray(value) || value.length !== 2) throw new TypeError("Parametric point must contain two expressions");
  return [expressionValue(value[0], parameters), expressionValue(value[1], parameters)];
}

function matrixValue(value: ParametricMatrix12, parameters: Readonly<Record<string, number>>): [
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
] {
  if (!Array.isArray(value) || value.length !== 12) {
    throw new TypeError("Parametric matrix must contain twelve expressions");
  }
  return value.map((entry) => expressionValue(entry, parameters)) as [
    number, number, number, number,
    number, number, number, number,
    number, number, number, number,
  ];
}

async function buildFeature(
  scope: ShapeScope,
  feature: ParametricFeature,
  parameters: Readonly<Record<string, number>>,
  shapes: Map<string, ShapeHandle>,
  subshapeReferenceDefinitions: ReadonlyMap<string, ParametricSubshapeReference>,
  subshapeReferenceIndices: ReadonlyMap<string, number>,
): Promise<ShapeHandle | { shape: ShapeHandle; history: History; inputs: readonly string[] }> {
  const shape = (id: string): ShapeHandle => {
    const result = shapes.get(id);
    if (result === undefined) throw new ReferenceError(`Feature "${id}" has not been built`);
    return result;
  };
  const orientedOptions = (value: OrientedFeature): { origin?: Vec3; direction?: Vec3 } => ({
    ...(value.origin === undefined ? {} : { origin: vectorValue(value.origin, parameters) }),
    ...(value.direction === undefined ? {} : { direction: vectorValue(value.direction, parameters) }),
  });
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

  switch (feature.type) {
    case "box":
      return scope.makeBox(
        vectorValue(feature.size, parameters),
        feature.origin === undefined ? [0, 0, 0] : vectorValue(feature.origin, parameters),
      );
    case "cylinder":
      return scope.makeCylinder(expressionValue(feature.radius, parameters), expressionValue(feature.height, parameters), orientedOptions(feature));
    case "sphere":
      return scope.makeSphere(expressionValue(feature.radius, parameters), orientedOptions(feature));
    case "cone":
      return scope.makeCone(
        expressionValue(feature.radius1, parameters),
        expressionValue(feature.radius2, parameters),
        expressionValue(feature.height, parameters),
        orientedOptions(feature),
      );
    case "torus":
      return scope.makeTorus(expressionValue(feature.majorRadius, parameters), expressionValue(feature.minorRadius, parameters), orientedOptions(feature));
    case "wedge":
      return scope.makeWedge(vectorValue(feature.size, parameters), expressionValue(feature.ltx, parameters), orientedOptions(feature));
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
      const normal = normalValue.map((value) => value / normalLength) as unknown as Vec3;
      const xValue = feature.xDirection === undefined ? [1, 0, 0] as Vec3 : vectorValue(feature.xDirection, parameters);
      const normalProjection = xValue[0] * normal[0] + xValue[1] * normal[1] + xValue[2] * normal[2];
      const projectedX: Vec3 = [
        xValue[0] - normal[0] * normalProjection,
        xValue[1] - normal[1] * normalProjection,
        xValue[2] - normal[2] * normalProjection,
      ];
      const xLength = Math.hypot(...projectedX);
      if (xLength <= Number.EPSILON) throw new RangeError(`Sketch feature "${feature.id}" xDirection must not be parallel to normal`);
      const xDirection: Vec3 = projectedX.map((value) => value / xLength) as unknown as Vec3;
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
  }
}

function validateSubshapeReferences(
  references: readonly ParametricSubshapeReference[],
  features: readonly ParametricFeature[],
): void {
  const featureIds = new Set(features.map(({ id }) => id));
  const referencesById = new Map(references.map((reference) => [reference.id, reference]));
  const ids = new Set<string>();
  for (const reference of references) {
    if (reference.id.length === 0 || ids.has(reference.id)) {
      throw new TypeError(`Duplicate or empty subshape reference id "${reference.id}"`);
    }
    ids.add(reference.id);
    if (!featureIds.has(reference.feature)) {
      throw new ReferenceError(`Unknown feature "${reference.feature}" referenced by subshape "${reference.id}"`);
    }
    if (reference.type !== "face" && reference.type !== "edge") {
      throw new TypeError(`Subshape reference "${reference.id}" type must be face or edge`);
    }
    if (reference.source !== undefined) {
      const source = referencesById.get(reference.source);
      if (source === undefined) {
        throw new ReferenceError(`Unknown source subshape reference "${reference.source}"`);
      }
      if (source.id === reference.id) {
        throw new TypeError(`Subshape reference "${reference.id}" cannot propagate itself`);
      }
      if (source.type !== reference.type) {
        throw new TypeError(`Subshape reference "${reference.id}" source must have type ${reference.type}`);
      }
      continue;
    }
    if (!Number.isInteger(reference.initialIndex) || reference.initialIndex < 0) {
      throw new TypeError(`Subshape reference "${reference.id}" initialIndex must be a non-negative integer`);
    }
    if (reference.disambiguation !== undefined && reference.disambiguation !== "initialIndex") {
      throw new TypeError(`Subshape reference "${reference.id}" has an invalid disambiguation strategy`);
    }
    if (reference.allowGeometryReplacement !== undefined
        && typeof reference.allowGeometryReplacement !== "boolean") {
      throw new TypeError(`Subshape reference "${reference.id}" allowGeometryReplacement must be boolean`);
    }
    if (reference.tolerance !== undefined
        && (!(reference.tolerance > 0) || !Number.isFinite(reference.tolerance))) {
      throw new TypeError(`Subshape reference "${reference.id}" tolerance must be positive and finite`);
    }
    if (reference.signature !== undefined
        && (typeof reference.signature.geometry !== "string"
            || reference.signature.normalizedBounds.length !== 6
            || reference.signature.normalizedBounds.some((value) => !Number.isFinite(value)))) {
      throw new TypeError(`Subshape reference "${reference.id}" has an invalid signature`);
    }
  }
}

async function subshapeSignature(
  client: BaseClient,
  parent: ShapeHandle,
  subshape: ShapeHandle,
  type: "face" | "edge",
): Promise<ParametricSubshapeSignature> {
  const [parentBounds, bounds, geometry] = await Promise.all([
    client.bbox(parent),
    client.bbox(subshape),
    type === "face" ? client.surfaceGeometry(subshape) : client.curveGeometry(subshape),
  ]);
  const normalized = (value: number, axis: number): number => {
    const extent = parentBounds.max[axis]! - parentBounds.min[axis]!;
    const scale = extent > 1e-12 ? extent : 1;
    return (value - parentBounds.min[axis]!) / scale;
  };
  return {
    geometry: geometry.type,
    normalizedBounds: [
      normalized(bounds.min[0], 0), normalized(bounds.min[1], 1), normalized(bounds.min[2], 2),
      normalized(bounds.max[0], 0), normalized(bounds.max[1], 1), normalized(bounds.max[2], 2),
    ],
  };
}

function signatureDistance(
  first: ParametricSubshapeSignature,
  second: ParametricSubshapeSignature,
  allowGeometryReplacement: boolean,
): number {
  if (!allowGeometryReplacement && first.geometry !== second.geometry) return Number.POSITIVE_INFINITY;
  return Math.max(...first.normalizedBounds.map((value, index) =>
    Math.abs(value - second.normalizedBounds[index]!)));
}

interface ParametricFeatureHistory {
  history: History;
  inputs: readonly string[];
}

function featureHistoryPath(
  sourceFeature: string,
  targetFeature: string,
  histories: ReadonlyMap<string, ParametricFeatureHistory>,
): PersistentHistoryStage[] | undefined {
  if (sourceFeature === targetFeature) return [];
  const target = histories.get(targetFeature);
  if (target === undefined) return undefined;
  const paths: PersistentHistoryStage[][] = [];
  for (let input = 0; input < target.inputs.length; input += 1) {
    const inputFeature = target.inputs[input]!;
    const prefix = inputFeature === sourceFeature
      ? []
      : featureHistoryPath(sourceFeature, inputFeature, histories);
    if (prefix !== undefined) paths.push([...prefix, { history: target.history, input }]);
  }
  return paths.length === 1 ? paths[0] : undefined;
}

async function resolveSubshapeReferences(
  client: BaseClient,
  scope: ShapeScope,
  references: ParametricSubshapeReference[],
  shapes: ReadonlyMap<string, ShapeHandle>,
  referenceDefinitions: ReadonlyMap<string, ParametricSubshapeReference>,
  referenceIndices: ReadonlyMap<string, number>,
  histories: ReadonlyMap<string, ParametricFeatureHistory>,
): Promise<{
  shapes: Map<string, ShapeHandle>;
  diagnostics: SubshapeReferenceDiagnostic[];
}> {
  const resolved = new Map<string, ShapeHandle>();
  const diagnostics: SubshapeReferenceDiagnostic[] = [];
  for (const reference of references) {
    const parent = shapes.get(reference.feature);
    if (parent === undefined) {
      diagnostics.push({
        id: reference.id, feature: reference.feature, type: reference.type, status: "missing",
        message: `Feature "${reference.feature}" has not been built`,
      });
      continue;
    }

    if (reference.source !== undefined) {
      const source = referenceDefinitions.get(reference.source)!;
      const sourceIndex = referenceIndices.get(reference.source);
      if (sourceIndex === undefined) {
        diagnostics.push({
          id: reference.id, feature: reference.feature, type: reference.type, status: "missing",
          message: `Source subshape reference "${reference.source}" has not been resolved`,
        });
        continue;
      }
      const path = featureHistoryPath(source.feature, reference.feature, histories);
      if (path === undefined || path.length === 0) {
        diagnostics.push({
          id: reference.id, feature: reference.feature, type: reference.type, status: "missing",
          message: `No unique OCCT history path connects feature "${source.feature}" to "${reference.feature}"`,
        });
        continue;
      }
      const result = resolvePersistentSubshapeStages(
        { type: source.type, index: sourceIndex }, path,
      );
      if (result.status === "resolved" && result.type === reference.type) {
        const subshape = await scope.getSubShape(parent, result.type!, result.index!);
        resolved.set(reference.id, subshape);
        diagnostics.push({
          id: reference.id, feature: reference.feature, type: reference.type,
          status: "resolved", index: result.index!,
        });
      } else {
        diagnostics.push({
          id: reference.id, feature: reference.feature, type: reference.type,
          status: result.status === "resolved" ? "missing" : result.status,
          ...(result.candidates === undefined
            ? {}
            : { candidateIndices: result.candidates.map(({ index }) => index) }),
          message: result.status === "ambiguous"
            ? `OCCT history maps source reference "${reference.source}" to multiple subshapes`
            : result.status === "resolved"
              ? `OCCT history changes source reference "${reference.source}" from ${reference.type} to ${result.type}`
            : `OCCT history no longer maps source reference "${reference.source}" to this feature`,
        });
      }
      continue;
    }

    if (reference.signature === undefined) {
      const initialExists = (await client.getSubShapes(parent, reference.type))
        .some(({ index }) => index === reference.initialIndex);
      if (!initialExists) {
        diagnostics.push({
          id: reference.id, feature: reference.feature, type: reference.type, status: "missing",
          message: `Initial ${reference.type} index ${reference.initialIndex} does not exist on feature "${reference.feature}"`,
        });
        continue;
      }
      const subshape = await scope.getSubShape(parent, reference.type, reference.initialIndex);
      reference.signature = await subshapeSignature(client, parent, subshape, reference.type);
      resolved.set(reference.id, subshape);
      diagnostics.push({
        id: reference.id, feature: reference.feature, type: reference.type,
        status: "resolved", index: reference.initialIndex,
      });
      continue;
    }

    const candidates: Array<{ index: number; shape: ShapeHandle }> = [];
    const tolerance = reference.tolerance ?? 1e-5;
    for (const { index } of await client.getSubShapes(parent, reference.type)) {
      const subshape = await scope.getSubShape(parent, reference.type, index);
      const signature = await subshapeSignature(client, parent, subshape, reference.type);
      if (signatureDistance(
        reference.signature,
        signature,
        reference.allowGeometryReplacement ?? false,
      ) <= tolerance) candidates.push({ index, shape: subshape });
    }
    if (candidates.length === 1) {
      resolved.set(reference.id, candidates[0]!.shape);
      diagnostics.push({
        id: reference.id, feature: reference.feature, type: reference.type,
        status: "resolved", index: candidates[0]!.index,
      });
    } else if (candidates.length === 0) {
      diagnostics.push({
        id: reference.id, feature: reference.feature, type: reference.type, status: "missing",
        message: `No ${reference.type} on feature "${reference.feature}" matches the persisted signature`,
      });
    } else {
      if (reference.disambiguation === "initialIndex") {
        const selected = candidates.find(({ index }) => index === reference.initialIndex);
        if (selected !== undefined) {
          resolved.set(reference.id, selected.shape);
          diagnostics.push({
            id: reference.id, feature: reference.feature, type: reference.type,
            status: "resolved", index: selected.index,
          });
          continue;
        }
      }
      diagnostics.push({
        id: reference.id, feature: reference.feature, type: reference.type, status: "ambiguous",
        candidateIndices: candidates.map(({ index }) => index),
        message: `${candidates.length} ${reference.type} candidates on feature "${reference.feature}" match the persisted signature`,
      });
    }
  }
  return { shapes: resolved, diagnostics };
}

export class ParametricModel implements AsyncDisposable {
  readonly #client: BaseClient;
  #definition: ParametricDefinition;
  #scope: ShapeScope | undefined;
  #shapes = new Map<string, ShapeHandle>();
  #subshapeReferences = new Map<string, ShapeHandle>();
  #parameterValues: Readonly<Record<string, number>> = Object.freeze({});
  #diagnostics: readonly FeatureDiagnostic[] = Object.freeze([]);
  #subshapeDiagnostics: readonly SubshapeReferenceDiagnostic[] = Object.freeze([]);
  #tail: Promise<void> = Promise.resolve();
  #disposeRequested = false;
  #disposePromise: Promise<void> | undefined;

  constructor(client: BaseClient, definition: ParametricDefinition = { parameters: {}, features: [] }) {
    this.#client = client;
    this.#definition = migrateDefinition(definition);
  }

  static fromJSON(client: BaseClient, json: string | unknown): ParametricModel {
    const value: unknown = typeof json === "string" ? JSON.parse(json) : json;
    if (value === null || typeof value !== "object") throw new TypeError("Parametric JSON must be an object");
    const candidate = value as Partial<ParametricDefinition>;
    if (candidate.parameters === null || typeof candidate.parameters !== "object" || Array.isArray(candidate.parameters)) {
      throw new TypeError("Parametric JSON parameters must be an object");
    }
    if (!Array.isArray(candidate.features)) throw new TypeError("Parametric JSON features must be an array");
    for (const feature of candidate.features as readonly { id?: unknown; suppressed?: unknown }[]) {
      if (feature !== null && typeof feature === "object"
          && feature.suppressed !== undefined && typeof feature.suppressed !== "boolean") {
        throw new TypeError(`Feature "${String(feature.id)}" suppressed must be boolean`);
      }
    }
    if (candidate.schemaVersion !== undefined
        && candidate.schemaVersion !== PARAMETRIC_SCHEMA_VERSION) {
      throw new RangeError(`Unsupported parametric schema version: ${String(candidate.schemaVersion)}`);
    }
    return new ParametricModel(client, candidate as ParametricDefinition);
  }

  setParameter(name: string, expression: ParametricExpression): this {
    assertParameterName(name);
    this.#definition.parameters[name] = expression;
    return this;
  }

  deleteParameter(name: string): boolean {
    return delete this.#definition.parameters[name];
  }

  setFeature(feature: ParametricFeature): this {
    const index = this.#definition.features.findIndex((candidate) => candidate.id === feature.id);
    const copy = structuredClone(feature);
    if (index === -1) this.#definition.features.push(copy);
    else this.#definition.features[index] = copy;
    return this;
  }

  deleteFeature(id: string): boolean {
    const index = this.#definition.features.findIndex((feature) => feature.id === id);
    if (index === -1) return false;
    this.#definition.features.splice(index, 1);
    return true;
  }

  setFeatureSuppressed(id: string, suppressed = true): this {
    const feature = this.#definition.features.find((candidate) => candidate.id === id);
    if (feature === undefined) throw new ReferenceError(`Feature "${id}" has not been defined`);
    feature.suppressed = suppressed;
    return this;
  }

  getParameter(name: string): number {
    if (!Object.prototype.hasOwnProperty.call(this.#parameterValues, name)) {
      throw new ReferenceError(`Parameter "${name}" has not been computed`);
    }
    return this.#parameterValues[name]!;
  }

  getShape(id: string): ShapeHandle {
    const shape = this.#shapes.get(id);
    if (shape === undefined) throw new ReferenceError(`Feature "${id}" has not been computed`);
    return shape;
  }

  getFeatureDiagnostics(): readonly FeatureDiagnostic[] {
    return this.#diagnostics.map((diagnostic) => ({ ...diagnostic }));
  }

  getSubshapeReference(id: string): ShapeHandle {
    const shape = this.#subshapeReferences.get(id);
    if (shape === undefined) {
      const diagnostic = this.#subshapeDiagnostics.find((candidate) => candidate.id === id);
      throw new ReferenceError(diagnostic?.message ?? `Subshape reference "${id}" has not been resolved`);
    }
    return shape;
  }

  getSubshapeReferenceDiagnostics(): readonly SubshapeReferenceDiagnostic[] {
    return this.#subshapeDiagnostics.map((diagnostic) => ({
      ...diagnostic,
      ...(diagnostic.candidateIndices === undefined ? {} : { candidateIndices: [...diagnostic.candidateIndices] }),
    }));
  }

  toSTEPDocumentDefinition(): STEPDocumentDefinition {
    const document = this.#definition.document;
    if (document === undefined) throw new ReferenceError("Parametric document has not been defined");
    return {
      roots: [...document.roots],
      nodes: document.nodes.map((node) => ({
        kind: node.kind,
        ...(node.kind === "part"
          ? { shape: this.getShape(node.feature) }
          : { children: [...(node.children ?? [])] }),
        ...(node.name === undefined ? {} : { name: node.name }),
        ...(node.transform === undefined ? {} : { transform: matrixValue(node.transform, this.#parameterValues) }),
        ...(node.color === undefined ? {} : { color: [...node.color] as RGBA }),
        ...(node.layers === undefined ? {} : { layers: [...node.layers] }),
        ...(node.visible === undefined ? {} : { visible: node.visible }),
        ...(node.material === undefined ? {} : { material: { ...node.material } }),
        ...(node.visualMaterial === undefined
          ? {}
          : { visualMaterial: structuredClone(node.visualMaterial) }),
        ...(node.validationProperties === undefined
          ? {}
          : { validationProperties: structuredClone(node.validationProperties) }),
      })),
    };
  }

  toJSON(): ParametricDefinition {
    return cloneDefinition(this.#definition);
  }

  async recompute(): Promise<void> {
    if (this.#disposeRequested) throw new Error("ParametricModel has been disposed");
    const definition = cloneDefinition(this.#definition);
    await this.#enqueue(() => this.#recompute(definition));
  }

  async #recompute(definition: ParametricDefinition): Promise<void> {
    const parameters = resolveParameters(definition.parameters);
    const features = orderFeatures(definition.features);
    const references = definition.subshapeReferences ?? [];
    validateSubshapeReferences(references, features);
    const nextScope = await this.#client.beginScope();
    const nextShapes = new Map<string, ShapeHandle>();
    const nextDiagnostics: FeatureDiagnostic[] = [];
    let nextSubshapes = new Map<string, ShapeHandle>();
    let nextSubshapeDiagnostics: SubshapeReferenceDiagnostic[] = [];
    const referenceDefinitions = new Map(references.map((reference) => [reference.id, reference]));
    const referenceIndices = new Map<string, number>();
    const featureHistories = new Map<string, ParametricFeatureHistory>();
    try {
      let hasFeatureError = false;
      let firstFeatureError: unknown;
      for (const feature of features) {
        if (feature.suppressed) {
          nextDiagnostics.push({ id: feature.id, type: feature.type, status: "suppressed" });
          continue;
        }
        const missingDependency = featureDependencies(feature)
          .map(dependencyOwner)
          .find((dependency) => !nextShapes.has(dependency));
        if (missingDependency !== undefined) {
          const error = new ReferenceError(`Feature "${missingDependency}" has not been built`);
          nextDiagnostics.push({
            id: feature.id,
            type: feature.type,
            status: "failed",
            message: error.message,
          });
          if (!hasFeatureError) {
            hasFeatureError = true;
            firstFeatureError = error;
          }
          continue;
        }
        try {
          const built = await buildFeature(
            nextScope, feature, parameters, nextShapes, referenceDefinitions, referenceIndices,
          );
          if ("history" in built && "inputs" in built && "shape" in built) {
            nextShapes.set(feature.id, built.shape);
            featureHistories.set(feature.id, { history: built.history, inputs: built.inputs });
          } else {
            nextShapes.set(feature.id, built);
          }
          nextDiagnostics.push({ id: feature.id, type: feature.type, status: "ok" });
          const featureReferences = references.filter((reference) => reference.feature === feature.id);
          if (featureReferences.length > 0) {
            const resolved = await resolveSubshapeReferences(
              this.#client, nextScope, featureReferences, nextShapes,
              referenceDefinitions, referenceIndices, featureHistories,
            );
            for (const [id, subshape] of resolved.shapes) nextSubshapes.set(id, subshape);
            for (const diagnostic of resolved.diagnostics) {
              nextSubshapeDiagnostics.push(diagnostic);
              if (diagnostic.status === "resolved") referenceIndices.set(diagnostic.id, diagnostic.index!);
            }
          }
        } catch (error) {
          nextDiagnostics.push({
            id: feature.id,
            type: feature.type,
            status: "failed",
            message: error instanceof Error ? error.message : String(error),
          });
          if (!hasFeatureError) {
            hasFeatureError = true;
            firstFeatureError = error;
          }
        }
      }
      for (const reference of references) {
        if (nextSubshapeDiagnostics.some(({ id }) => id === reference.id)) continue;
        nextSubshapeDiagnostics.push({
          id: reference.id,
          feature: reference.feature,
          type: reference.type,
          status: "missing",
          message: `Feature "${reference.feature}" has not been built`,
        });
      }
      nextSubshapeDiagnostics = references.map((reference) =>
        nextSubshapeDiagnostics.find(({ id }) => id === reference.id)!);
      if (hasFeatureError) throw firstFeatureError;
    } catch (error) {
      this.#diagnostics = Object.freeze(nextDiagnostics.map((diagnostic) => ({ ...diagnostic })));
      try {
        await nextScope.end();
      } catch {
        // Preserve the feature error; the temporary scope is never published.
      }
      throw error;
    }

    const previousScope = this.#scope;
    if (previousScope !== undefined) {
      try {
        await previousScope.end();
      } catch (error) {
        try {
          await nextScope.end();
        } catch {
          // Preserve the cleanup error from the currently published scope.
        }
        throw error;
      }
    }
    this.#scope = nextScope;
    this.#shapes = nextShapes;
    this.#subshapeReferences = nextSubshapes;
    this.#parameterValues = parameters;
    this.#diagnostics = Object.freeze(nextDiagnostics.map((diagnostic) => ({ ...diagnostic })));
    this.#subshapeDiagnostics = Object.freeze(nextSubshapeDiagnostics.map((diagnostic) => ({ ...diagnostic })));
    for (const reference of references) {
      if (reference.signature === undefined) continue;
      const current = this.#definition.subshapeReferences?.find(({ id }) => id === reference.id);
      if (current !== undefined && current.feature === reference.feature && current.type === reference.type) {
        current.signature = structuredClone(reference.signature);
      }
    }
  }

  async dispose(): Promise<void> {
    if (this.#disposePromise !== undefined) return this.#disposePromise;
    this.#disposeRequested = true;
    this.#disposePromise = this.#enqueue(async () => {
      const scope = this.#scope;
      if (scope !== undefined) await scope.end();
      this.#scope = undefined;
      this.#shapes = new Map();
      this.#subshapeReferences = new Map();
      this.#parameterValues = Object.freeze({});
      this.#diagnostics = Object.freeze([]);
      this.#subshapeDiagnostics = Object.freeze([]);
    });
    try {
      await this.#disposePromise;
    } catch (error) {
      this.#disposePromise = undefined;
      throw error;
    }
  }

  #enqueue(operation: () => Promise<void>): Promise<void> {
    const result = this.#tail.then(operation);
    this.#tail = result.catch(() => undefined);
    return result;
  }

  async [Symbol.asyncDispose](): Promise<void> {
    await this.dispose();
  }
}
