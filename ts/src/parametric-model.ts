import { type BaseClient, type ShapeHandle, type ShapeScope } from "./client.js";
import type { RGBA, STEPDocumentDefinition } from "./types.js";

import type {
  FeatureDiagnostic,
  ParametricDefinition,
  ParametricExpression,
  ParametricFeature,
  SubshapeReferenceDiagnostic,
} from "./parametric-types.js";
import { parseParametricDefinition } from "./parametric-validation.js";

import { assertParameterName, cloneDefinition, migrateDefinition, resolveParameters } from "./parametric-expressions.js";
import { dependencyOwner, featureDependencies, orderFeatures } from "./parametric-graph.js";
import { buildFeature, matrixValue } from "./parametric-evaluator.js";
import {
  resolveSubshapeReferences, validateSubshapeReferences,
  type ParametricFeatureHistory,
} from "./parametric-references.js";

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
    return new ParametricModel(client, parseParametricDefinition(value));
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
          nextDiagnostics.push({ id: feature.id, type: feature.type, status: "ok" });
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
