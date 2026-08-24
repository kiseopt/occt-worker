import { BaseClient } from "./client.js";
import type { BinaryBuffer, KernelErrorData, MaterializedBuffer, RequestOptions } from "./types.js";

export interface DirectClientRuntimeHooks {
  isCancelled(): boolean;
  onProgress(fraction: number): void;
}

export interface DirectBuffer {
  readonly bufferId: number;
  readonly byteLength: number;
  readonly layout: string;
  /** Reacquire after every wasm call because memory growth invalidates prior views. */
  view(): Uint8Array;
}

interface RuntimeState {
  operation: string | undefined;
  options: RequestOptions | undefined;
  hooks?: DirectClientRuntimeHooks;
}

interface WasmExports extends WebAssembly.Exports {
  memory: WebAssembly.Memory;
  _initialize?: () => void;
  emscripten_stack_init?: () => void;
  emscripten_stack_get_base?: () => number;
  emscripten_stack_get_end?: () => number;
  __set_stack_limits?: (base: number, end: number) => void;
  k_alloc: (length: number) => number;
  k_free: (pointer: number) => void;
  k_handle: (pointer: number, length: number) => number;
  k_response_ptr: () => number;
  k_buffer_ptr: (bufferId: number) => number;
  k_buffer_len: (bufferId: number) => number;
}

interface ResponseFrame {
  id: number;
  ok: boolean;
  result?: unknown;
  error?: KernelErrorData;
}

export class DirectClient extends BaseClient {
  readonly #exports: WasmExports;
  readonly #runtimeState: RuntimeState;
  readonly #directBuffers = new WeakMap<DirectBuffer, { bufferId: number; released: boolean }>();
  readonly #liveDirectBuffers = new Set<{ bufferId: number; released: boolean }>();
  #nextId = 1;

  private constructor(exports: WasmExports, runtimeState: RuntimeState) {
    super();
    this.#exports = exports;
    this.#runtimeState = runtimeState;
  }

  static async create(
    wasm: BufferSource | WebAssembly.Module,
    imports: WebAssembly.Imports = {},
    hooks?: DirectClientRuntimeHooks,
  ): Promise<DirectClient> {
    // OCCT requires both a valid wall-clock value and forward progress in timer loops.
    // Advance a deterministic synthetic clock instead of reading the host clock.
    let memory: WebAssembly.Memory | undefined;
    let clockNanoseconds = 1767225600000000000n;
    const runtimeState: RuntimeState = { operation: undefined, options: undefined };
    if (hooks !== undefined) runtimeState.hooks = hooks;
    const stubs: Record<string, (...args: never[]) => number | void> = {
      clock_time_get: (_clockId: number, _precision: bigint, timePointer: number): number => {
        if (memory !== undefined) {
          new DataView(memory.buffer).setBigUint64(timePointer, clockNanoseconds, true);
          clockNanoseconds += 1000000n;
        }
        return 0;
      },
      fd_write: (_fd: number, iovPointer: number, iovCount: number, writtenPointer: number): number => {
        if (memory !== undefined) {
          const view = new DataView(memory.buffer);
          let written = 0;
          for (let index = 0; index < iovCount; index++) {
            written += view.getUint32(iovPointer + index * 8 + 4, true);
          }
          view.setUint32(writtenPointer, written, true);
        }
        return 0;
      },
      fd_read: (_fd: number, _iovPointer: number, _iovCount: number, readPointer: number): number => {
        if (memory !== undefined) new DataView(memory.buffer).setUint32(readPointer, 0, true);
        return 0;
      },
      environ_sizes_get: (countPointer: number, sizePointer: number): number => {
        if (memory !== undefined) {
          const view = new DataView(memory.buffer);
          view.setUint32(countPointer, 0, true);
          view.setUint32(sizePointer, 1, true);
        }
        return 0;
      },
      environ_get: (_environPointer: number, bufferPointer: number): number => {
        if (memory !== undefined) new Uint8Array(memory.buffer)[bufferPointer] = 0;
        return 0;
      },
      occt_worker_cancelled: (): number => {
        return runtimeState.options?.signal?.aborted || runtimeState.hooks?.isCancelled() ? 1 : 0;
      },
      occt_worker_progress: (fraction: number): void => {
        const operation = runtimeState.operation;
        if (operation !== undefined) {
          runtimeState.options?.onProgress?.({ operation, fraction });
          runtimeState.hooks?.onProgress(fraction);
        }
      },
    };
    const wasi = new Proxy(stubs, {
      get: (target, property) => (typeof property === "string" && property in target ? target[property] : () => 0),
    });
    const env = new Proxy(
      { ...stubs, ...(imports.env as WebAssembly.ModuleImports | undefined) },
      {
        get: (target, property) => (
          typeof property === "string" && property in target
            ? target[property as keyof typeof target]
            : () => 0
        ),
      },
    );
    const mergedImports: WebAssembly.Imports = {
      ...imports,
      wasi_snapshot_preview1: wasi,
      env,
    };
    const instance = wasm instanceof WebAssembly.Module
      ? await WebAssembly.instantiate(wasm, mergedImports)
      : (await WebAssembly.instantiate(wasm, mergedImports)).instance;
    const exports = instance.exports as WasmExports;
    memory = exports.memory;
    if (
      exports.emscripten_stack_init !== undefined
      && exports.emscripten_stack_get_base !== undefined
      && exports.emscripten_stack_get_end !== undefined
      && exports.__set_stack_limits !== undefined
    ) {
      exports.emscripten_stack_init();
      exports.__set_stack_limits(exports.emscripten_stack_get_base(), exports.emscripten_stack_get_end());
    }
    exports._initialize?.();
    const client = new DirectClient(exports, runtimeState);
    await client.initialize();
    return client;
  }

  async createBuffer(byteLength: number): Promise<DirectBuffer> {
    const descriptor = this.#call("createBuffer", { byteLength }) as {
      bufferId: number;
      byteLength: number;
      layout: string;
    };
    const state = { bufferId: descriptor.bufferId, released: false };
    const buffer: DirectBuffer = Object.freeze({
      ...descriptor,
      view: (): Uint8Array => {
        if (state.released) throw new TypeError("DirectBuffer has been released");
        const length = this.#exports.k_buffer_len(state.bufferId);
        const pointer = this.#exports.k_buffer_ptr(state.bufferId);
        if (length !== descriptor.byteLength || (length !== 0 && pointer === 0)) {
          state.released = true;
          this.#liveDirectBuffers.delete(state);
          throw new TypeError("DirectBuffer is no longer valid");
        }
        return new Uint8Array(this.#exports.memory.buffer, pointer, length);
      },
    });
    this.#directBuffers.set(buffer, state);
    this.#liveDirectBuffers.add(state);
    return buffer;
  }

  async freeBuffer(buffer: DirectBuffer): Promise<void> {
    const state = this.#directBuffers.get(buffer);
    if (state === undefined) throw new TypeError("DirectBuffer belongs to a different DirectClient");
    if (state.released) throw new TypeError("DirectBuffer has already been released");
    this.#call("freeBuffer", { bufferId: state.bufferId });
    state.released = true;
    this.#liveDirectBuffers.delete(state);
  }

  override async releaseAll(): Promise<void> {
    await super.releaseAll();
    for (const state of this.#liveDirectBuffers) state.released = true;
    this.#liveDirectBuffers.clear();
  }

  protected async send(
    op: string,
    args: Record<string, unknown>,
    options: RequestOptions = {},
  ): Promise<unknown> {
    if (options.signal?.aborted) {
      throw options.signal.reason ?? new DOMException("Kernel request aborted", "AbortError");
    }
    if (options.outputBuffers === "shared" && typeof SharedArrayBuffer === "undefined") {
      throw new TypeError("SharedArrayBuffer is not available in this host");
    }
    const inputBuffers: number[] = [];
    const prepareInputs = (value: unknown): unknown => {
      if (value !== null && typeof value === "object" && "$inputBuffer" in value) {
        const input = value as {
          $inputBuffer: BinaryBuffer;
          byteOffset?: number;
          byteLength?: number;
        };
        const byteOffset = input.byteOffset ?? 0;
        const byteLength = input.byteLength ?? input.$inputBuffer.byteLength;
        const source = new Uint8Array(input.$inputBuffer, byteOffset, byteLength);
        const descriptor = this.#call("createBuffer", { byteLength }) as {
          bufferId: number;
        };
        const pointer = this.#exports.k_buffer_ptr(descriptor.bufferId);
        new Uint8Array(this.#exports.memory.buffer, pointer, byteLength).set(source);
        inputBuffers.push(descriptor.bufferId);
        return { bufferId: descriptor.bufferId };
      }
      if (Array.isArray(value)) return value.map(prepareInputs);
      if (value !== null && typeof value === "object") {
        return Object.fromEntries(Object.entries(value).map(([key, item]) => [key, prepareInputs(item)]));
      }
      return value;
    };

    this.#runtimeState.operation = op;
    this.#runtimeState.options = options;
    try {
      options.onProgress?.({ operation: op, fraction: 0 });
      const result = this.#materialize(
        this.#call(op, prepareInputs(args) as Record<string, unknown>),
        options.outputBuffers === "shared",
      );
      options.onProgress?.({ operation: op, fraction: 1 });
      return result;
    } catch (error) {
      if (options.signal?.aborted) {
        throw options.signal.reason ?? new DOMException("Kernel request aborted", "AbortError");
      }
      throw error;
    } finally {
      for (const bufferId of inputBuffers) this.#call("freeBuffer", { bufferId });
      this.#runtimeState.operation = undefined;
      this.#runtimeState.options = undefined;
    }
  }

  #call(op: string, args: Record<string, unknown>): unknown {
    const request = new TextEncoder().encode(JSON.stringify({ id: this.#nextId++, op, args }));
    const pointer = this.#exports.k_alloc(request.byteLength);
    if (pointer === 0 && request.byteLength !== 0) throw new Error("Kernel request allocation failed");
    let responseLength: number;
    try {
      new Uint8Array(this.#exports.memory.buffer, pointer, request.byteLength).set(request);
      responseLength = this.#exports.k_handle(pointer, request.byteLength);
    } finally {
      this.#exports.k_free(pointer);
    }
    if (responseLength === 0) throw new Error("Kernel instance could not construct a protocol response");
    const responsePointer = this.#exports.k_response_ptr();
    const response = JSON.parse(
      new TextDecoder().decode(
        new Uint8Array(this.#exports.memory.buffer, responsePointer, responseLength),
      ),
    ) as ResponseFrame;
    if (!response.ok) this.kernelError(response.error!);
    return response.result;
  }

  #materialize(value: unknown, shared: boolean): unknown {
    if (
      value !== null
      && typeof value === "object"
      && "bufferId" in value
      && "byteLength" in value
      && "layout" in value
    ) {
      const descriptor = value as { bufferId: number; byteLength: number; layout: string };
      const pointer = this.#exports.k_buffer_ptr(descriptor.bufferId);
      const length = this.#exports.k_buffer_len(descriptor.bufferId);
      const buffer: BinaryBuffer = shared ? new SharedArrayBuffer(length) : new ArrayBuffer(length);
      new Uint8Array(buffer).set(new Uint8Array(this.#exports.memory.buffer, pointer, length));
      this.#call("freeBuffer", { bufferId: descriptor.bufferId });
      return { layout: descriptor.layout, data: buffer } satisfies MaterializedBuffer<BinaryBuffer>;
    }
    if (Array.isArray(value)) return value.map((item) => this.#materialize(item, shared));
    if (value !== null && typeof value === "object") {
      return Object.fromEntries(
        Object.entries(value).map(([key, item]) => [key, this.#materialize(item, shared)]),
      );
    }
    return value;
  }
}
