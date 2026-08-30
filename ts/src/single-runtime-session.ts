import type { ProfileRuntime, ProfileRuntimeFactory } from "./engine.js";
import type { OperationName } from "./generated.js";
import type { RequestOptions } from "./types.js";
import { EngineError } from "./engine.js";

export type SingleRuntimeFactory = ProfileRuntimeFactory;

interface PendingRequest<T> {
  promise: Promise<T>;
  settled: Promise<void>;
  reject: (error: unknown) => void;
}

/** A small application-facing session that owns at most one profile runtime. */
export class SingleRuntimeSession {
  readonly #factories: ReadonlyMap<string, ProfileRuntimeFactory>;
  readonly #pending = new Set<PendingRequest<unknown>>();
  #runtime: ProfileRuntime | undefined;
  #profileId: string | undefined;
  #switching: Promise<void> | undefined;
  #closed = false;

  constructor(factories: ReadonlyMap<string, ProfileRuntimeFactory> | Readonly<Record<string, ProfileRuntimeFactory>>) {
    this.#factories = factories instanceof Map ? factories : new Map(Object.entries(factories));
  }

  get profileId(): string | undefined { return this.#profileId; }
  get closed(): boolean { return this.#closed; }

  async switchProfile(profileId: string): Promise<void> {
    if (this.#closed) throw new EngineError("InvalidPlacementState", "session is closed");
    if (this.#switching !== undefined) {
      throw new EngineError("InvalidPlacementState", "profile switch already in progress");
    }
    const factory = this.#factories.get(profileId);
    if (factory === undefined) throw new EngineError("UnsupportedCapability", `unknown profile '${profileId}'`);
    this.#switching = this.#performSwitch(profileId, factory);
    try {
      await this.#switching;
    } finally {
      this.#switching = undefined;
    }
  }

  async request<T = unknown>(operation: OperationName, args: Record<string, unknown> = {}, options?: RequestOptions): Promise<T> {
    if (this.#closed || this.#switching !== undefined || this.#runtime === undefined) {
      throw new EngineError("InvalidPlacementState", "no active runtime");
    }
    const runtime = this.#runtime;
    let rejectRequest!: (error: unknown) => void;
    let settle!: () => void;
    const settled = new Promise<void>((resolve) => { settle = resolve; });
    const caller = new Promise<T>((resolve, reject) => {
      rejectRequest = reject;
      void runtime.request<T>(operation, args, options).then(resolve, reject).finally(settle);
    });
    const pending: PendingRequest<T> = { promise: caller, settled, reject: rejectRequest };
    this.#pending.add(pending as PendingRequest<unknown>);
    caller.finally(() => this.#pending.delete(pending as PendingRequest<unknown>)).catch(() => undefined);
    return caller;
  }

  async close(): Promise<void> {
    if (this.#closed) return;
    this.#closed = true;
    for (const pending of this.#pending) pending.reject(new EngineError("InvalidPlacementState", "session closed"));
    this.#pending.clear();
    const runtime = this.#runtime;
    this.#runtime = undefined;
    this.#profileId = undefined;
    await runtime?.close?.();
  }

  async #performSwitch(profileId: string, factory: ProfileRuntimeFactory): Promise<void> {
    for (const pending of this.#pending) {
      pending.reject(new EngineError("InvalidPlacementState", "runtime switch invalidated request"));
    }
    await Promise.all([...this.#pending].map((pending) => pending.settled));
    this.#pending.clear();

    const oldRuntime = this.#runtime;
    this.#runtime = undefined;
    this.#profileId = undefined;
    await oldRuntime?.close?.();

    try {
      const runtime = await factory();
      if (this.#closed) {
        await runtime.close?.();
        throw new EngineError("InvalidPlacementState", "session closed during profile switch");
      }
      this.#runtime = runtime;
      this.#profileId = profileId;
    } catch (error) {
      this.#closed = true;
      this.#runtime = undefined;
      this.#profileId = undefined;
      throw error;
    }
  }
}
