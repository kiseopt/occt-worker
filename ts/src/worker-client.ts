import { BaseClient } from "./client.js";
import { COOPERATIVE_OPERATIONS, type OperationName } from "./generated.js";
import { collectArrayBuffers } from "./protocol-codec.js";
import {
  KernelError,
  TimeoutError,
  type KernelErrorData,
  type RequestOptions,
  type SharedBufferResult,
} from "./types.js";

interface WorkerLike {
  postMessage(message: unknown, transfer?: Transferable[]): void;
  terminate(): void;
  addEventListener(type: "message", listener: (event: MessageEvent) => void): void;
  addEventListener(type: "error", listener: (event: ErrorEvent) => void): void;
}

export type WorkerInitializationStage = "compiling" | "instantiating" | "ready";

export interface WorkerClientOptions {
  onInitializationStage?(stage: WorkerInitializationStage): void;
}

interface QueuedCall {
  id: number;
  op: OperationName;
  args: Record<string, unknown>;
  transfers: Transferable[];
  timeoutMs?: number;
  signal?: AbortSignal;
  abortListener?: () => void;
  cancelFlag?: Int32Array;
  abortReason?: unknown;
  onProgress?: RequestOptions["onProgress"];
  outputBuffers?: RequestOptions["outputBuffers"];
  resolve(value: unknown): void;
  reject(reason: unknown): void;
  timer?: ReturnType<typeof setTimeout>;
}

const cooperativeOperations = new Set<OperationName>(COOPERATIVE_OPERATIONS);

export class WorkerClient extends BaseClient {
  readonly #factory: () => WorkerLike;
  readonly #wasm: ArrayBuffer;
  readonly #options: WorkerClientOptions;
  #worker: WorkerLike | undefined;
  #ready!: Promise<void>;
  #rejectReady: ((reason: unknown) => void) | undefined;
  #isReady = false;
  #closed = false;
  #nextId = 1;
  #active: QueuedCall | undefined;
  #queue: QueuedCall[] = [];

  private constructor(factory: () => WorkerLike, wasm: ArrayBuffer, options: WorkerClientOptions) {
    super();
    this.#factory = factory;
    this.#wasm = wasm;
    this.#options = options;
    this.#startWorker();
  }

  static async create(
    factory: () => WorkerLike,
    wasm: ArrayBuffer,
    options: WorkerClientOptions = {},
  ): Promise<WorkerClient> {
    const client = new WorkerClient(factory, wasm, options);
    try {
      await client.#ready;
      await client.initialize();
      return client;
    } catch (error) {
      client.close();
      throw error;
    }
  }

  protected async send<T>(
    op: OperationName,
    args: Record<string, unknown>,
    options: RequestOptions = {},
  ): Promise<T> {
    await this.#ready;
    if (this.#closed) throw new Error("WorkerClient closed");
    if (options.signal?.aborted) {
      throw options.signal.reason ?? new DOMException("Worker request aborted", "AbortError");
    }
    const id = this.#nextId++;
    const transfers: Transferable[] = collectArrayBuffers(args);

    return new Promise<T>((resolve, reject) => {
      const call: QueuedCall = {
        id,
        op,
        args,
        transfers,
        resolve: (value) => resolve(value as T),
        reject,
      };
      if (options.timeoutMs !== undefined) call.timeoutMs = options.timeoutMs;
      if (options.onProgress !== undefined) call.onProgress = options.onProgress;
      if (options.outputBuffers !== undefined) call.outputBuffers = options.outputBuffers;
      if (options.signal !== undefined) {
        call.signal = options.signal;
        if (cooperativeOperations.has(op) && typeof SharedArrayBuffer !== "undefined") {
          call.cancelFlag = new Int32Array(new SharedArrayBuffer(Int32Array.BYTES_PER_ELEMENT));
        }
        call.abortListener = () => {
          const reason = options.signal?.reason ?? new DOMException("Worker request aborted", "AbortError");
          if (this.#active === call) {
            if (call.cancelFlag !== undefined) {
              call.abortReason = reason;
              Atomics.store(call.cancelFlag, 0, 1);
              return;
            }
            this.#restart(reason);
            return;
          }
          const index = this.#queue.indexOf(call);
          if (index === -1) return;
          this.#queue.splice(index, 1);
          this.#clearCall(call);
          call.reject(reason);
        };
        options.signal.addEventListener("abort", call.abortListener, { once: true });
      }
      this.#queue.push(call);
      this.#dispatchNext();
    });
  }

  async requestShared<T>(
    op: OperationName,
    args: Record<string, unknown>,
    timeoutOrOptions?: number | Omit<RequestOptions, "outputBuffers">,
  ): Promise<SharedBufferResult<T>> {
    if (typeof SharedArrayBuffer === "undefined") {
      throw new TypeError("SharedArrayBuffer is not available in this host");
    }
    const options = typeof timeoutOrOptions === "number"
      ? { timeoutMs: timeoutOrOptions, outputBuffers: "shared" as const }
      : { ...timeoutOrOptions, outputBuffers: "shared" as const };
    return await this.requestUnsafe<T>(op, args, options) as SharedBufferResult<T>;
  }

  close(): void {
    if (this.#closed) return;
    this.#closed = true;
    this.#worker?.terminate();
    this.#worker = undefined;
    const error = new Error("WorkerClient closed");
    this.#rejectReady?.(error);
    this.#rejectReady = undefined;
    this.#failCalls(error);
    this.invalidateEpoch();
  }

  #startWorker(): void {
    this.#isReady = false;
    let worker: WorkerLike;
    try {
      worker = this.#factory();
    } catch (error) {
      this.#rejectReady = undefined;
      this.#ready = Promise.reject(error);
      void this.#ready.catch(() => undefined);
      return;
    }
    this.#worker = worker;
    this.#ready = new Promise((resolve, reject) => {
      this.#rejectReady = reject;
      const onMessage = (event: MessageEvent): void => {
        if (worker !== this.#worker || this.#closed) return;
        const message = event.data as {
          type: string;
          id?: number;
          ok?: boolean;
          result?: unknown;
          error?: KernelErrorData;
          fraction?: number;
          stage?: WorkerInitializationStage;
        };
        if (message.type === "init-progress" && message.stage !== undefined) {
          this.#options.onInitializationStage?.(message.stage);
          return;
        }
        if (message.type === "ready") {
          this.#isReady = true;
          this.#rejectReady = undefined;
          this.#options.onInitializationStage?.("ready");
          resolve();
          this.#dispatchNext();
          return;
        }
        if (message.type === "init-error") {
          const error = message.error === undefined
            ? new Error("Worker initialization failed")
            : new KernelError(message.error);
          this.#failInitialization(worker, error, reject);
          return;
        }
        if (message.type === "fatal") {
          const error = message.error === undefined
            ? new Error("Worker runtime failed")
            : new KernelError(message.error);
          this.#restart(error);
          return;
        }
        if (message.type === "progress" && message.id !== undefined && message.fraction !== undefined) {
          const active = this.#active;
          if (active?.id === message.id) {
            active.onProgress?.({ operation: active.op, fraction: message.fraction });
          }
          return;
        }
        if (message.type !== "response" || message.id === undefined) return;
        const active = this.#active;
        if (active === undefined || active.id !== message.id) return;
        this.#active = undefined;
        this.#clearCall(active);
        if (active.abortReason !== undefined) active.reject(active.abortReason);
        else if (message.ok) active.resolve(message.result);
        else active.reject(
          message.error === undefined
            ? new Error("Worker request failed")
            : new KernelError(message.error),
        );
        this.#dispatchNext();
      };
      worker.addEventListener("message", onMessage);
      worker.addEventListener("error", (event) => {
        if (worker !== this.#worker || this.#closed) return;
        const error = event.error ?? new Error(event.message);
        if (!this.#isReady) {
          this.#failInitialization(worker, error, reject);
          return;
        }
        this.#restart(error);
      });
      try {
        worker.postMessage({ type: "init", wasm: this.#wasm });
      } catch (error) {
        this.#failInitialization(worker, error, reject);
      }
    });
    // A restart can happen before another request awaits readiness. Keep a
    // failed replacement initialization from becoming an unhandled rejection.
    void this.#ready.catch(() => undefined);
  }

  #restart(reason: unknown): void {
    this.#worker?.terminate();
    this.#worker = undefined;
    this.#failCalls(reason);
    this.invalidateEpoch();
    this.#startWorker();
  }

  #dispatchNext(): void {
    const worker = this.#worker;
    if (!this.#isReady || this.#closed || this.#active !== undefined || worker === undefined) return;
    const call = this.#queue.shift();
    if (call === undefined) return;
    this.#active = call;
    if (call.timeoutMs !== undefined) {
      call.timer = setTimeout(() => {
        if (this.#active === call) this.#restart(new TimeoutError());
      }, call.timeoutMs);
    }
    try {
      worker.postMessage(
        {
          type: "call",
          id: call.id,
          op: call.op,
          args: call.args,
          ...(call.outputBuffers === undefined ? {} : { outputBuffers: call.outputBuffers }),
          ...(call.cancelFlag === undefined ? {} : { cancelBuffer: call.cancelFlag.buffer }),
        },
        call.transfers,
      );
    } catch (error) {
      this.#clearCall(call);
      this.#active = undefined;
      call.reject(error);
      this.#dispatchNext();
    }
  }

  #failInitialization(
    worker: WorkerLike,
    reason: unknown,
    reject: (reason: unknown) => void,
  ): void {
    if (worker !== this.#worker || this.#isReady) return;
    worker.terminate();
    this.#worker = undefined;
    this.#rejectReady = undefined;
    reject(reason);
  }

  #failCalls(reason: unknown): void {
    if (this.#active !== undefined) {
      this.#clearCall(this.#active);
      this.#active.reject(reason);
      this.#active = undefined;
    }
    for (const call of this.#queue) {
      this.#clearCall(call);
      call.reject(reason);
    }
    this.#queue = [];
  }

  #clearCall(call: QueuedCall): void {
    if (call.timer !== undefined) clearTimeout(call.timer);
    if (call.signal !== undefined && call.abortListener !== undefined) {
      call.signal.removeEventListener("abort", call.abortListener);
    }
  }
}
