import type { HistorySupportLevel, OperationName, KernelErrorCode } from "./generated.js";

export interface KernelErrorData {
  code: KernelErrorCode;
  message: string;
  details?: unknown;
}

export class KernelError extends Error {
  readonly code: KernelErrorCode;
  readonly details: unknown;

  constructor(error: KernelErrorData) {
    super(error.message);
    this.name = "KernelError";
    this.code = error.code;
    this.details = error.details;
  }
}

export class TimeoutError extends Error {
  constructor(message = "Kernel worker request timed out") {
    super(message);
    this.name = "TimeoutError";
  }
}

export interface RequestProgress {
  operation: string;
  fraction: number;
}

export interface RequestOptions {
  timeoutMs?: number;
  signal?: AbortSignal;
  onProgress?: (progress: RequestProgress) => void;
  /** Materialize kernel output buffers into shared memory instead of ordinary ArrayBuffers. */
  outputBuffers?: "array" | "shared";
}

export interface Capabilities {
  protocolVersion: string;
  kernelVersion: string;
  occtVersion: string;
  ops: OperationName[];
  historySupport: Partial<Record<OperationName, HistorySupportLevel>>;
  buildFlags: Record<string, boolean | string | number>;
}

export interface KernelStats {
  liveShapeHandles: number;
  liveBufferBytes: number;
  wasmMemorySize: number;
}

export interface BBox {
  min: [number, number, number];
  max: [number, number, number];
}

export interface OrientedBBox {
  center: Vec3;
  axes: readonly [Vec3, Vec3, Vec3];
  halfSizes: Vec3;
  axisAligned: boolean;
}

export interface ShapeDiagnosticIssue {
  type: "compound" | "compsolid" | "solid" | "shell" | "face" | "wire" | "edge" | "vertex";
  index: number;
  status: string;
}

export interface ShapeDiagnostics {
  valid: boolean;
  issues: ShapeDiagnosticIssue[];
}

export type BinaryBuffer = ArrayBuffer | SharedArrayBuffer;

export interface MaterializedBuffer<TBuffer extends BinaryBuffer = ArrayBuffer> {
  layout: string;
  data: TBuffer;
}

export type SharedBufferResult<T> =
  T extends ArrayBuffer ? SharedArrayBuffer
    : T extends (...args: never[]) => unknown ? T
      : T extends readonly unknown[] ? { [K in keyof T]: SharedBufferResult<T[K]> }
        : T extends object ? { [K in keyof T]: SharedBufferResult<T[K]> }
          : T;

export type Vec3 = readonly [number, number, number];
export type Matrix12 = readonly [
  number, number, number, number,
  number, number, number, number,
  number, number, number, number,
];
export type RGBA = readonly [number, number, number, number];
export type LengthUnit = "mm" | "cm" | "m" | "inch" | "foot";
export type XCAFDocumentFormat = "bin" | "xml";
