import {
  PARAMETRIC_SCHEMA_VERSION,
  type ParametricDefinition,
  type ParametricFeature,
} from "./parametric-types.js";

type Validator = (value: unknown, path: string) => void;

function fail(path: string, expected: string): never {
  throw new TypeError(`${path} must be ${expected}`);
}

function record(value: unknown, path: string): asserts value is Record<string, unknown> {
  if (value === null || typeof value !== "object" || Array.isArray(value)) fail(path, "an object");
}

const stringValue: Validator = (value, path) => { if (typeof value !== "string") fail(path, "a string"); };
const numberValue: Validator = (value, path) => { if (typeof value !== "number") fail(path, "a number"); };
const booleanValue: Validator = (value, path) => { if (typeof value !== "boolean") fail(path, "a boolean"); };
const expression: Validator = (value, path) => {
  if (typeof value !== "number" && typeof value !== "string") fail(path, "a number or expression string");
};
const oneOf = (...values: readonly unknown[]): Validator => (value, path) => {
  if (!values.includes(value)) fail(path, `one of ${values.map(String).join(", ")}`);
};
const arrayOf = (item: Validator): Validator => (value, path) => {
  if (!Array.isArray(value)) fail(path, "an array");
  value.forEach((entry, index) => item(entry, `${path}[${index}]`));
};
const tuple = (length: number, item: Validator): Validator => (value, path) => {
  if (!Array.isArray(value) || value.length !== length) fail(path, `an array of ${length} values`);
  value.forEach((entry, index) => item(entry, `${path}[${index}]`));
};
const vec3 = tuple(3, expression);
const point2 = tuple(2, expression);
const matrix12 = tuple(12, expression);
const numberVec3 = tuple(3, numberValue);
const rgba = tuple(4, numberValue);
const strings = arrayOf(stringValue);
const expressions = arrayOf(expression);

function fields(
  value: Record<string, unknown>,
  path: string,
  required: Record<string, Validator>,
  optional: Record<string, Validator> = {},
): void {
  for (const [name, validate] of Object.entries(required)) {
    if (!(name in value)) fail(`${path}.${name}`, "present");
    validate(value[name], `${path}.${name}`);
  }
  for (const [name, validate] of Object.entries(optional)) {
    if (value[name] !== undefined) validate(value[name], `${path}.${name}`);
  }
}

const selector: Validator = (value, path) => {
  if (typeof value === "number") return;
  record(value, path);
  fields(value, path, { reference: stringValue });
};
const selectors = arrayOf(selector);
const binding: Validator = (value, path) => {
  record(value, path);
  fields(value, path, { newIndex: numberValue, baseIndex: numberValue });
};
const bindings = arrayOf(binding);

function validateSketchEntity(value: unknown, path: string): void {
  record(value, path);
  fields(value, path, { id: stringValue, type: stringValue }, { construction: booleanValue });
  switch (value.type) {
    case "line": fields(value, path, { start: point2, end: point2 }); return;
    case "circle": fields(value, path, { center: point2, radius: expression }); return;
    case "arc": fields(value, path, { center: point2, radius: expression, startAngle: expression, endAngle: expression }); return;
    case "spline": fields(value, path, { poles: arrayOf(point2) }, { degree: numberValue }); return;
    default: fail(`${path}.type`, "line, circle, arc, or spline");
  }
}

const pointReference: Validator = (value, path) => {
  record(value, path);
  fields(value, path, { entity: stringValue, point: oneOf("start", "end", "center") });
};

function validateSketchConstraint(value: unknown, path: string): void {
  record(value, path);
  fields(value, path, { type: stringValue });
  switch (value.type) {
    case "coincident": fields(value, path, { first: pointReference, second: pointReference }); return;
    case "fixed": fields(value, path, { point: pointReference, value: point2 }); return;
    case "horizontal": case "vertical": fields(value, path, { entity: stringValue }); return;
    case "distance": fields(value, path, { first: pointReference, second: pointReference, value: expression }); return;
    case "length": case "radius": fields(value, path, { entity: stringValue, value: expression }); return;
    case "parallel": case "perpendicular": case "equalLength": case "equalRadius":
    case "tangent": case "concentric": fields(value, path, { first: stringValue, second: stringValue }); return;
    case "symmetry": fields(value, path, { first: pointReference, second: pointReference, axis: stringValue }); return;
    default: fail(`${path}.type`, "a supported sketch constraint type");
  }
}

type FeatureValidator = (value: Record<string, unknown>, path: string) => void;
type FeatureValidatorRegistry = { [Type in ParametricFeature["type"]]: FeatureValidator };

const oriented = { origin: vec3, direction: vec3 };
const sketchProfile: Validator = (value, path) => {
  record(value, path);
  fields(value, path, { id: stringValue, entities: strings });
};
const axis: Validator = (value, path) => {
  record(value, path);
  fields(value, path, { direction: vec3 }, { origin: vec3, xDirection: vec3 });
};
const lawPoint: Validator = (value, path) => {
  record(value, path);
  fields(value, path, { parameter: expression, radius: expression });
};
const plane: Validator = (value, path) => {
  record(value, path);
  fields(value, path, { origin: vec3, normal: vec3 });
};

const booleanFeature: FeatureValidator = (value, path) =>
  fields(value, path, { base: stringValue, tools: strings });

const featureValidators = {
  box: (value, path) => fields(value, path, { size: vec3 }, { origin: vec3 }),
  cylinder: (value, path) => fields(value, path, { radius: expression, height: expression }, oriented),
  sphere: (value, path) => fields(value, path, { radius: expression }, oriented),
  cone: (value, path) => fields(value, path, { radius1: expression, radius2: expression, height: expression }, oriented),
  torus: (value, path) => fields(value, path, { majorRadius: expression, minorRadius: expression }, oriented),
  wedge: (value, path) => fields(value, path, { size: vec3, ltx: expression }, oriented),
  polygon: (value, path) => fields(value, path, { points: arrayOf(vec3) }, { close: booleanValue }),
  bezierCurve: (value, path) => fields(value, path, { poles: arrayOf(vec3) }, { weights: expressions }),
  bsplineCurve: (value, path) => fields(value, path, { poles: arrayOf(vec3) }, {
    degree: expression, periodic: booleanValue, tolerance: expression, knots: expressions,
    multiplicities: expressions, weights: expressions,
  }),
  bezierSurface: (value, path) => fields(value, path, { poles: arrayOf(arrayOf(vec3)) }, {
    weights: arrayOf(expressions), tolerance: expression,
  }),
  bsplineSurface: (value, path) => fields(value, path, { poles: arrayOf(arrayOf(vec3)) }, {
    weights: arrayOf(expressions), uDegree: expression, vDegree: expression,
    uKnots: expressions, vKnots: expressions, uMultiplicities: expressions, vMultiplicities: expressions,
    uPeriodic: booleanValue, vPeriodic: booleanValue, tolerance: expression,
  }),
  sketch: (value, path) => fields(value, path, { entities: arrayOf(validateSketchEntity) }, {
    constraints: arrayOf(validateSketchConstraint), profiles: arrayOf(sketchProfile), origin: vec3,
    normal: vec3, xDirection: vec3, tolerance: expression, maxIterations: numberValue,
  }),
  face: (value, path) => fields(value, path, { outer: stringValue }, { holes: strings }),
  extrude: (value, path) => fields(value, path, { input: stringValue, vector: vec3 }),
  revolve: (value, path) => fields(value, path, { input: stringValue, origin: vec3, direction: vec3 }, { angle: expression }),
  loft: (value, path) => fields(value, path, { sections: strings }, { solid: booleanValue, ruled: booleanValue, precision: expression }),
  sweepPipe: (value, path) => fields(value, path, { spine: stringValue, profile: stringValue }),
  sweepPipeShell: (value, path) => fields(value, path, { spine: stringValue, profiles: strings }, {
    mode: oneOf("frenet", "correctedFrenet", "fixedAxis", "auxiliarySpine"), axis,
    auxiliarySpine: stringValue, curvilinearEquivalence: booleanValue,
  }),
  fillet: (value, path) => fields(value, path, { input: stringValue, edgeIndices: selectors, radius: expression }, {
    radii: expressions, radius2: expression, radiusLaw: arrayOf(lawPoint), radiusLaws: arrayOf(arrayOf(lawPoint)),
  }),
  chamfer: (value, path) => fields(value, path, { input: stringValue, edgeIndices: selectors, distance: expression }, {
    distances: expressions, distance2: expression, distances2: expressions, referenceFaceIndices: selectors,
  }),
  hollow: (value, path) => fields(value, path, { input: stringValue, thickness: expression }, { closingFaceIndices: selectors }),
  offsetShape: (value, path) => fields(value, path, { input: stringValue, offset: expression }, { tolerance: expression }),
  offsetWire2D: (value, path) => fields(value, path, { input: stringValue, offset: expression }),
  draftAngle: (value, path) => fields(value, path, {
    input: stringValue, faceIndices: selectors, direction: vec3, angle: expression, neutralPlane: plane,
  }, { flag: booleanValue }),
  localPrism: (value, path) => {
    fields(value, path, { base: stringValue, faceIndices: selectors, direction: vec3 }, {
      mode: oneOf("length", "until", "fromUntil", "untilEnd", "thruAll", "fromEnd"), operation: oneOf("add", "cut"),
    });
    const mode = value.mode ?? "length";
    if (mode === "length") fields(value, path, { length: expression });
    else if (mode === "until") fields(value, path, { until: stringValue });
    else if (mode === "fromUntil") fields(value, path, { from: stringValue, until: stringValue });
    else if (mode === "fromEnd") fields(value, path, { from: stringValue });
  },
  localRevolution: (value, path) => {
    fields(value, path, { base: stringValue, faceIndices: selectors, origin: vec3, direction: vec3 }, {
      mode: oneOf("angle", "until", "fromUntil", "thruAll"), operation: oneOf("add", "cut"),
    });
    const mode = value.mode ?? "angle";
    if (mode === "angle") fields(value, path, { angle: expression });
    else if (mode === "until") fields(value, path, { until: stringValue });
    else if (mode === "fromUntil") fields(value, path, { from: stringValue, until: stringValue });
  },
  linearForm: (value, path) => fields(value, path, {
    base: stringValue, profile: stringValue, planeOrigin: vec3, planeNormal: vec3, direction: vec3,
  }, { direction1: vec3, operation: oneOf("add", "cut"), modify: booleanValue }),
  revolutionForm: (value, path) => fields(value, path, {
    base: stringValue, profile: stringValue, planeOrigin: vec3, planeNormal: vec3,
    origin: vec3, direction: vec3, height1: expression, height2: expression,
  }, { operation: oneOf("add", "cut"), modify: booleanValue }),
  glue: (value, path) => fields(value, path, {
    newShape: stringValue, baseShape: stringValue, faceBindings: bindings,
  }, { edgeBindings: bindings }),
  cylindricalHole: (value, path) => fields(value, path, {
    input: stringValue, origin: vec3, direction: vec3, radius: expression,
  }, {
    mode: oneOf("throughAll", "throughNext", "untilEnd", "blind", "between"),
    length: expression, from: expression, to: expression, withControl: booleanValue,
  }),
  defeature: (value, path) => fields(value, path, { input: stringValue, faceIndices: selectors }),
  sew: (value, path) => fields(value, path, { shapes: strings }, { tolerance: expression }),
  fixShape: (value, path) => fields(value, path, { input: stringValue }, { precision: expression }),
  unifySameDomain: (value, path) => fields(value, path, { input: stringValue }, {
    unifyEdges: booleanValue, unifyFaces: booleanValue, concatBSplines: booleanValue,
  }),
  shapeUpgrade: (value, path) => {
    fields(value, path, { input: stringValue }, { mode: oneOf(
      "continuity", "angle", "area", "closedFaces", "closedEdges", "convertToBezier",
      "removeInternalWires", "removeLocations",
    ) });
    const common = {
      precision: expression, minTolerance: expression, maxTolerance: expression,
      surfaceSegmentMode: booleanValue, edgeMode: oneOf(0, 1, 2),
    };
    const mode = value.mode ?? "continuity";
    if (mode === "continuity") fields(value, path, {}, {
      ...common, boundaryCriterion: oneOf("c0", "c1", "c2", "c3", "cn"),
      pcurveCriterion: oneOf("c0", "c1", "c2", "c3", "cn"),
      surfaceCriterion: oneOf("c0", "c1", "c2", "c3", "cn"), tolerance: expression, tolerance2d: expression,
    });
    else if (mode === "angle") fields(value, path, { maxAngle: expression }, common);
    else if (mode === "area") {
      fields(value, path, { areaMode: oneOf("maxArea", "parts", "uv") }, common);
      if (value.areaMode === "maxArea") fields(value, path, { maxArea: expression });
      else if (value.areaMode === "parts") fields(value, path, { nbParts: expression });
      else fields(value, path, { uSplits: expression, vSplits: expression });
    } else if (mode === "closedFaces") fields(value, path, {}, { ...common, splitPoints: expression });
    else if (mode === "closedEdges") fields(value, path, {}, common);
    else if (mode === "convertToBezier") fields(value, path, {}, {
      ...common, convert2d: booleanValue, convert3d: booleanValue, convertSurfaces: booleanValue,
      convertLines: booleanValue, convertCircles: booleanValue, convertConics: booleanValue,
      convertPlanes: booleanValue, convertRevolutions: booleanValue, convertExtrusions: booleanValue,
      convertBSplines: booleanValue,
    });
    else if (mode === "removeInternalWires") fields(value, path, { minArea: expression }, { removeFaces: booleanValue });
    else fields(value, path, {}, { removeLevel: oneOf("shape", "compound", "solid", "shell", "face") });
  },
  section: (value, path) => fields(value, path, { first: stringValue, second: stringValue }),
  split: (value, path) => fields(value, path, { objects: strings }, { tools: strings, fuzzyValue: expression, useOBB: booleanValue }),
  translate: (value, path) => fields(value, path, { input: stringValue, translation: vec3 }),
  rotate: (value, path) => fields(value, path, { input: stringValue, angle: expression }, { origin: vec3, direction: vec3 }),
  scale: (value, path) => fields(value, path, { input: stringValue, factor: expression }, { origin: vec3 }),
  mirror: (value, path) => fields(value, path, { input: stringValue, normal: vec3 }),
  generalTransform: (value, path) => fields(value, path, { input: stringValue, matrix: matrix12 }),
  booleanCut: booleanFeature,
  booleanFuse: booleanFeature,
  booleanCommon: booleanFeature,
} satisfies FeatureValidatorRegistry;

function validateFeature(value: unknown, path: string): asserts value is ParametricFeature {
  record(value, path);
  fields(value, path, { id: stringValue, type: stringValue }, { suppressed: booleanValue });
  const type = value.type;
  if (typeof type !== "string" || !Object.prototype.hasOwnProperty.call(featureValidators, type)) {
    fail(`${path}.type`, "a supported parametric feature type");
  }
  featureValidators[type as ParametricFeature["type"]](value, path);
}

function validateDocument(value: unknown, path: string): void {
  record(value, path);
  const material: Validator = (entry, entryPath) => {
    record(entry, entryPath);
    fields(entry, entryPath, { name: stringValue, density: numberValue }, {
      description: stringValue, densityName: stringValue, densityValueType: stringValue,
    });
  };
  const visualMaterial: Validator = (entry, entryPath) => {
    record(entry, entryPath);
    fields(entry, entryPath, { name: stringValue }, {
      baseColor: rgba, metallic: numberValue, roughness: numberValue, emissive: numberVec3, refractionIndex: numberValue,
    });
  };
  const validation: Validator = (entry, entryPath) => {
    record(entry, entryPath);
    fields(entry, entryPath, {}, { area: numberValue, volume: numberValue, centroid: numberVec3 });
  };
  const node: Validator = (entry, entryPath) => {
    record(entry, entryPath);
    fields(entry, entryPath, { kind: oneOf("part", "assembly") }, {
      name: stringValue, transform: matrix12, color: rgba, layers: strings, visible: booleanValue,
      material, visualMaterial, validationProperties: validation,
    });
    if (entry.kind === "part") fields(entry, entryPath, { feature: stringValue });
    else fields(entry, entryPath, {}, { children: arrayOf(numberValue) });
  };
  fields(value, path, { nodes: arrayOf(node), roots: arrayOf(numberValue) });
}

const signature: Validator = (value, path) => {
  record(value, path);
  fields(value, path, { geometry: stringValue, normalizedBounds: tuple(6, numberValue) });
};

function validateSubshapeReference(value: unknown, path: string): void {
  record(value, path);
  fields(value, path, { id: stringValue, feature: stringValue, type: oneOf("face", "edge") });
  if ("source" in value) fields(value, path, { source: stringValue });
  else fields(value, path, { initialIndex: numberValue }, {
    disambiguation: oneOf("initialIndex"), allowGeometryReplacement: booleanValue,
    signature, tolerance: numberValue,
  });
}

export function parseParametricDefinition(value: unknown): ParametricDefinition {
  record(value, "Parametric JSON");
  if (value.schemaVersion !== undefined && value.schemaVersion !== PARAMETRIC_SCHEMA_VERSION) {
    throw new RangeError(`Unsupported parametric schema version: ${String(value.schemaVersion)}`);
  }
  fields(value, "Parametric JSON", {
    parameters: (parameters, path) => {
      record(parameters, path);
      for (const [name, parameter] of Object.entries(parameters)) expression(parameter, `${path}.${name}`);
    },
    features: arrayOf(validateFeature),
  }, {
    schemaVersion: oneOf(PARAMETRIC_SCHEMA_VERSION),
    document: validateDocument,
    subshapeReferences: arrayOf(validateSubshapeReference),
  });
  return structuredClone(value) as unknown as ParametricDefinition;
}
