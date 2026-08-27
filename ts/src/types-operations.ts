import type { OperationName } from "./generated.js";
import type { Vec3 } from "./types-base.js";
import type { KernelErrorData } from "./types-base.js";

export interface FormatProbe {
  format: "brep" | "step" | "iges" | "stl" | "obj" | "ply" | "gltf" | "vrml" | "xcaf" | "unknown";
  encoding?: "text" | "binary" | "json" | "glb";
  confidence: "exact" | "likely" | "none";
  documentMetadata?: boolean;
}

export interface Adjacency {
  from: string;
  to: string;
  items: Array<{ index: number; adjacent: number[] }>;
}

export interface FilletRadiusPoint {
  parameter: number;
  radius: number;
}

export interface TopologyCounts {
  compound: number;
  compsolid: number;
  solid: number;
  shell: number;
  face: number;
  wire: number;
  edge: number;
  vertex: number;
}

export interface HistorySource {
  input: number;
  type: "face" | "edge";
  index: number;
}

export interface HistoryTarget {
  type: "face" | "edge";
  index: number;
}

export interface History {
  retained: Array<{ from: HistorySource; to: HistoryTarget }>;
  generated: Array<{ from: HistorySource; to: HistoryTarget[] }>;
  modified: Array<{ from: HistorySource; to: HistoryTarget[] }>;
  deleted: HistorySource[];
}

export interface BatchOperation {
  op: OperationName;
  args?: Record<string, unknown>;
}

export interface BatchItemResult extends Record<string, unknown> {
  shape?: import("./client.js").ShapeHandle;
  shapes?: import("./client.js").ShapeHandle[];
}

export interface BatchResult {
  results: BatchItemResult[];
  error?: KernelErrorData;
}


export interface GeneralFuseCellRule {
  take: readonly number[];
  avoid?: readonly number[];
  material?: number;
}

export interface GeneralFuseCellSelectionOptions {
  fuzzyValue?: number;
  glue?: "off" | "shift" | "full";
  useOBB?: boolean;
  removeInternalBoundaries?: boolean;
  makeContainers?: boolean;
}

export interface GlueBinding {
  newIndex: number;
  baseIndex: number;
}

export interface DraftAngleOptions {
  neutralPlane: { origin: Vec3; normal: Vec3 };
  flag?: boolean;
}

interface CylindricalHoleCommonOptions {
  withControl?: boolean;
  includeHistory?: boolean;
}

export type CylindricalHoleOptions =
  | (CylindricalHoleCommonOptions & { mode?: "throughAll" | "throughNext" | "untilEnd" })
  | (CylindricalHoleCommonOptions & { mode: "blind"; length: number })
  | (CylindricalHoleCommonOptions & { mode: "between"; from: number; to: number });

