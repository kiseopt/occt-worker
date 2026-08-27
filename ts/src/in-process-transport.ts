import type { OperationName } from "./generated.js";
import { mapProtocolValue, materializeProtocolBuffers, type ProtocolBufferDescriptor } from "./protocol-codec.js";
import {
  TimeoutError,
  type BinaryBuffer,
  type KernelErrorData,
  type MaterializedBuffer,
  type RequestOptions,
} from "./types.js";

export interface InProcessMemory {
  bytes(): Uint8Array;
  alloc(length: number): number;
  free(pointer: number): void;
  handle(pointer: number, length: number): number;
  responsePointer(): number;
  bufferPointer(bufferId: number): number;
  bufferLength(bufferId: number): number;
}

interface ResponseFrame {
  ok: boolean;
  result?: unknown;
  error?: KernelErrorData;
}

export class InProcessTransport {
  readonly #memory: InProcessMemory;
  readonly #raiseKernelError: (error: KernelErrorData) => never;
  #nextId = 1;
  #operation: OperationName | undefined;
  #options: RequestOptions | undefined;
  #deadline: number | undefined;

  constructor(memory: InProcessMemory, raiseKernelError: (error: KernelErrorData) => never) {
    this.#memory = memory;
    this.#raiseKernelError = raiseKernelError;
  }

  isCancelled(): boolean {
    return this.#options?.signal?.aborted === true
      || (this.#deadline !== undefined && Date.now() >= this.#deadline);
  }

  reportProgress(fraction: number): void {
    if (this.#operation !== undefined) {
      this.#options?.onProgress?.({ operation: this.#operation, fraction });
    }
  }

  async request<T>(
    operation: OperationName,
    args: Record<string, unknown>,
    options: RequestOptions = {},
  ): Promise<T> {
    this.#throwIfCancelled(options);
    const inputBuffers: number[] = [];
    const prepareInput = (value: unknown): unknown => {
      if (value !== null && typeof value === "object" && "$inputBuffer" in value) {
        const input = value as { $inputBuffer: BinaryBuffer; byteOffset?: number; byteLength?: number };
        const byteOffset = input.byteOffset ?? 0;
        const byteLength = input.byteLength ?? input.$inputBuffer.byteLength - byteOffset;
        const source = new Uint8Array(input.$inputBuffer, byteOffset, byteLength);
        const descriptor = this.call("createBuffer", { byteLength }) as { bufferId: number };
        inputBuffers.push(descriptor.bufferId);
        const pointer = this.#memory.bufferPointer(descriptor.bufferId);
        if (this.#memory.bufferLength(descriptor.bufferId) !== byteLength || (byteLength !== 0 && pointer === 0)) {
          throw new Error("kernel input buffer is unavailable");
        }
        this.#memory.bytes().set(source, pointer);
        return { bufferId: descriptor.bufferId };
      }
      return value;
    };

    this.#operation = operation;
    this.#options = options;
    this.#deadline = options.timeoutMs === undefined ? undefined : Date.now() + options.timeoutMs;
    try {
      this.reportProgress(0);
      const encoded = mapProtocolValue(args, prepareInput) as Record<string, unknown>;
      const result = this.#materialize(this.call(operation, encoded), options.outputBuffers === "shared");
      this.#throwIfCancelled(options);
      this.reportProgress(1);
      return result as T;
    } catch (error) {
      this.#throwIfCancelled(options);
      throw error;
    } finally {
      for (const bufferId of inputBuffers) this.call("freeBuffer", { bufferId });
      this.#operation = undefined;
      this.#options = undefined;
      this.#deadline = undefined;
    }
  }

  call(operation: OperationName, args: Record<string, unknown>): unknown {
    const request = new TextEncoder().encode(JSON.stringify({ id: this.#nextId++, op: operation, args }));
    const pointer = this.#memory.alloc(request.byteLength);
    if (pointer === 0 && request.byteLength !== 0) throw new Error("kernel request allocation failed");
    let responseLength: number;
    try {
      this.#memory.bytes().set(request, pointer);
      responseLength = this.#memory.handle(pointer, request.byteLength);
    } finally {
      this.#memory.free(pointer);
    }
    if (responseLength === 0) throw new Error("kernel did not return a protocol response");
    const responsePointer = this.#memory.responsePointer();
    const response = JSON.parse(new TextDecoder().decode(
      this.#memory.bytes().slice(responsePointer, responsePointer + responseLength),
    )) as ResponseFrame;
    if (!response.ok) {
      this.#raiseKernelError(response.error ?? { code: "KernelError", message: "kernel request failed" });
    }
    return response.result;
  }

  #materialize(value: unknown, shared: boolean): unknown {
    return materializeProtocolBuffers(value, shared, (descriptor: ProtocolBufferDescriptor, useShared) => {
      const pointer = this.#memory.bufferPointer(descriptor.bufferId);
      const length = this.#memory.bufferLength(descriptor.bufferId);
      if (length !== descriptor.byteLength || (length !== 0 && pointer === 0)) {
        throw new Error("kernel output buffer is unavailable");
      }
      const buffer: BinaryBuffer = useShared ? new SharedArrayBuffer(length) : new ArrayBuffer(length);
      new Uint8Array(buffer).set(this.#memory.bytes().subarray(pointer, pointer + length));
      return { layout: descriptor.layout, data: buffer } satisfies MaterializedBuffer<BinaryBuffer>;
    }, (descriptor) => this.call("freeBuffer", { bufferId: descriptor.bufferId }));
  }

  #throwIfCancelled(options: RequestOptions): void {
    if (options.signal?.aborted) {
      throw options.signal.reason ?? new DOMException("Kernel request aborted", "AbortError");
    }
    if (this.#deadline !== undefined && Date.now() >= this.#deadline) {
      throw new TimeoutError();
    }
  }
}
