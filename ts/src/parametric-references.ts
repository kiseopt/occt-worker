import { type BaseClient, type ShapeHandle, type ShapeScope } from "./client.js";
import type { History, HistorySource, HistoryTarget } from "./types.js";

import type {
  ParametricFeature,
  ParametricSubshapeReference,
  ParametricSubshapeSignature,
  PersistentSubshapeResolution,
  SubshapeReferenceDiagnostic,
} from "./parametric-types.js";

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

export function validateSubshapeReferences(
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

  const visiting = new Set<string>();
  const visited = new Set<string>();
  const visit = (id: string): void => {
    if (visited.has(id)) return;
    if (visiting.has(id)) {
      throw new TypeError(`Subshape reference source cycle includes "${id}"`);
    }
    visiting.add(id);
    const source = referencesById.get(id)?.source;
    if (source !== undefined) visit(source);
    visiting.delete(id);
    visited.add(id);
  };
  for (const reference of references) visit(reference.id);
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

export interface ParametricFeatureHistory {
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

export async function resolveSubshapeReferences(
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
