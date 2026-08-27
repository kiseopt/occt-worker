import type { ShapeHandle, ShapeScope } from "./client.js";
import type { History } from "./types.js";
import type { ParametricSubshapeReference, ParametricSubshapeSelector } from "./parametric-types.js";

export type BuiltFeature = ShapeHandle | { shape: ShapeHandle; history: History; inputs: readonly string[] };

export interface FeatureBuildContext {
  scope: ShapeScope;
  parameters: Readonly<Record<string, number>>;
  shapes: Map<string, ShapeHandle>;
  shape(id: string): ShapeHandle;
  subshapeIndices(selectors: readonly ParametricSubshapeSelector[], type: "face" | "edge", featureId: string): number[];
  subshapeReferenceDefinitions: ReadonlyMap<string, ParametricSubshapeReference>;
  subshapeReferenceIndices: ReadonlyMap<string, number>;
}
