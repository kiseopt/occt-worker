import type { ParametricFeature } from "./parametric-types.js";

export function featureDependencies(feature: ParametricFeature): readonly string[] {
  switch (feature.type) {
    case "box": case "cylinder": case "sphere": case "cone": case "torus": case "wedge":
    case "polygon": case "bezierCurve": case "bsplineCurve": case "bezierSurface": case "bsplineSurface":
    case "sketch":
      return [];
    case "extrude": case "revolve": case "translate": case "rotate": case "scale": case "mirror":
    case "generalTransform": case "fillet": case "chamfer": case "hollow": case "offsetShape":
    case "offsetWire2D": case "draftAngle": case "fixShape": case "unifySameDomain": case "shapeUpgrade":
    case "cylindricalHole": case "defeature":
      return [feature.input];
    case "face":
      return [feature.outer, ...(feature.holes ?? [])];
    case "loft":
      return feature.sections;
    case "sweepPipe":
      return [feature.spine, feature.profile];
    case "sweepPipeShell":
      return [feature.spine, ...feature.profiles,
        ...(feature.auxiliarySpine === undefined ? [] : [feature.auxiliarySpine])];
    case "localPrism": case "localRevolution":
      return [feature.base,
        ...("from" in feature ? [feature.from] : []),
        ...("until" in feature ? [feature.until] : [])];
    case "linearForm": case "revolutionForm":
      return [feature.base, feature.profile];
    case "glue":
      return [feature.newShape, feature.baseShape];
    case "sew":
      return feature.shapes;
    case "section":
      return [feature.first, feature.second];
    case "split":
      return [...feature.objects, ...(feature.tools ?? [])];
    case "booleanCut": case "booleanFuse": case "booleanCommon":
      return [feature.base, ...feature.tools];
    default: {
      const exhaustive: never = feature;
      throw new TypeError(`Unsupported feature type '${String((exhaustive as { type?: unknown }).type)}'`);
    }
  }
}

export function dependencyOwner(id: string): string {
  const separator = id.indexOf(".");
  return separator === -1 ? id : id.slice(0, separator);
}

export function orderFeatures(features: readonly ParametricFeature[]): ParametricFeature[] {
  const byId = new Map<string, ParametricFeature>();
  for (const feature of features) {
    if (typeof feature.id !== "string" || !/^[A-Za-z_][A-Za-z0-9_-]*$/.test(feature.id)) {
      throw new TypeError(`Invalid feature id "${String(feature.id)}"`);
    }
    featureDependencies(feature);
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
