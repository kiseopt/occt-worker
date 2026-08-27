import { BaseClient } from "./client.js";
import { InProcessTransport } from "./in-process-transport.js";
import type { OperationName } from "./generated.js";
import type { RequestOptions } from "./types.js";

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
  transport?: InProcessTransport;
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

export class DirectClient extends BaseClient {
  readonly #exports: WasmExports;
  readonly #transport: InProcessTransport;
  readonly #directBuffers = new WeakMap<DirectBuffer, { bufferId: number; released: boolean }>();
  readonly #liveDirectBuffers = new Set<{ bufferId: number; released: boolean }>();

  private constructor(exports: WasmExports, transport: InProcessTransport) {
    super();
    this.#exports = exports;
    this.#transport = transport;
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
    const runtimeState: RuntimeState = {};
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
        return runtimeState.transport?.isCancelled() || runtimeState.hooks?.isCancelled() ? 1 : 0;
      },
      occt_worker_progress: (fraction: number): void => {
        runtimeState.transport?.reportProgress(fraction);
        runtimeState.hooks?.onProgress(fraction);
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
    let client: DirectClient;
    const transport = new InProcessTransport({
      bytes: () => new Uint8Array(exports.memory.buffer),
      alloc: (length) => exports.k_alloc(length),
      free: (pointer) => exports.k_free(pointer),
      handle: (pointer, length) => exports.k_handle(pointer, length),
      responsePointer: () => exports.k_response_ptr(),
      bufferPointer: (bufferId) => exports.k_buffer_ptr(bufferId),
      bufferLength: (bufferId) => exports.k_buffer_len(bufferId),
    }, (error) => client.kernelError(error));
    client = new DirectClient(exports, transport);
    runtimeState.transport = transport;
    await client.initialize();
    return client;
  }

  async createBuffer(byteLength: number): Promise<DirectBuffer> {
    const descriptor = this.#transport.call("createBuffer", { byteLength }) as {
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
    this.#transport.call("freeBuffer", { bufferId: state.bufferId });
    state.released = true;
    this.#liveDirectBuffers.delete(state);
  }

  override async releaseAll(): Promise<void> {
    await super.releaseAll();
    for (const state of this.#liveDirectBuffers) state.released = true;
    this.#liveDirectBuffers.clear();
  }

  protected async send<T>(
    op: OperationName,
    args: Record<string, unknown>,
    options: RequestOptions = {},
  ): Promise<T> {
    return this.#transport.request<T>(op, args, options);
  }
}
