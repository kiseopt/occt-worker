// Compatibility boundary between GeometryEngine and the high-level API
// (ParametricModel / Sketch / ShapeScope).
//
// Runtime contract (docs/architecture.md):
//   - EngineShapeRef never extends the legacy u32 ShapeHandle class.
//   - Request routing walks nested args recursively and rejects handles from
//     other backends before any message leaves the caller.

import { BaseClient } from "./client.js";
import { EngineError, EngineShapeRef, type GeometryEngine } from "./engine.js";
import { type OperationName } from "./generated.js";
import { mapProtocolValue } from "./protocol-codec.js";
import type { RequestOptions } from "./types.js";

export function isEngineShapeRef(value: unknown): value is EngineShapeRef {
  return value instanceof EngineShapeRef;
}

function visitEngineRefs(value: unknown, visit: (ref: EngineShapeRef) => void): void {
  mapProtocolValue(value, (item) => {
    if (isEngineShapeRef(item)) visit(item);
    return item;
  });
}

/**
 * Adapter exposing the engine through a BaseClient-shaped `request()` so
 * legacy call sites keep their calling convention while gaining routing.
 */
export class EngineCompatClient extends BaseClient {
  readonly #engine: GeometryEngine;
  readonly #scopes = new Map<number, Set<number>>();
  #nextScopeId = 1;
  #closed = false;
  #closePromise: Promise<void> | undefined;

  constructor(engine: GeometryEngine) {
    super();
    this.#engine = engine;
  }

  close(): Promise<void> {
    if (this.#closePromise !== undefined) return this.#closePromise;
    if (this.#closed) return Promise.resolve();
    this.#closed = true;
    const closing = (async (): Promise<void> => {
      try {
        await this.#engine.close();
      } finally {
        this.#scopes.clear();
        this.invalidateEpoch();
      }
    })();
    this.#closePromise = closing;
    return closing;
  }

  protected send<T>(
    operation: OperationName,
    args: Record<string, unknown>,
    options: RequestOptions = {},
  ): Promise<T> {
    return this.#sendRaw(operation, args, options) as Promise<T>;
  }

  async #sendRaw(
    operation: OperationName,
    args: Record<string, unknown>,
    options: RequestOptions = {},
  ): Promise<unknown> {
    if (this.#closed) throw new Error("EngineCompatClient closed");
    if (options.signal?.aborted) {
      throw options.signal.reason ?? new DOMException("Engine request aborted", "AbortError");
    }

    if (operation === "capabilities") {
      return this.#engine.capabilities();
    }
    if (operation === "beginScope") {
      const scopeId = this.#nextScopeId++;
      this.#scopes.set(scopeId, new Set());
      return { scopeId };
    }
    if (operation === "endScope") {
      const scopeId = this.#requiredScope(args);
      const shapes = this.#scopes.get(scopeId);
      if (shapes === undefined) throw new TypeError("ShapeScope belongs to an expired engine scope");
      let firstError: unknown;
      for (const logicalId of [...shapes]) {
        try {
          await this.#engine.releaseShape(logicalId);
          shapes.delete(logicalId);
        } catch (error) {
          if (error instanceof EngineError && error.code === "UnknownLogicalShape") {
            shapes.delete(logicalId);
            continue;
          }
          if (firstError === undefined) firstError = error;
        }
      }
      if (shapes.size === 0) this.#scopes.delete(scopeId);
      if (firstError !== undefined) throw firstError;
      return {};
    }
    if (operation === "release") {
      const ref = args.shape;
      if (!isEngineShapeRef(ref)) throw new TypeError("release requires an engine shape");
      await this.#engine.releaseShape(ref.logicalId);
      for (const shapes of this.#scopes.values()) shapes.delete(ref.logicalId);
      return {};
    }
    if (operation === "releaseAll") {
      await this.#engine.releaseAll();
      for (const shapes of this.#scopes.values()) shapes.clear();
      return {};
    }
    if (operation === "stats") {
      const stats = this.#engine.stats();
      return {
        liveShapeHandles: stats.shapes,
        liveBufferBytes: 0,
        wasmMemorySize: 0,
      };
    }
    if (operation === "createBuffer" || operation === "freeBuffer") {
      throw new TypeError(`${operation} is not available across isolated profile runtimes`);
    }

    const scopeId = typeof args.scopeId === "number" ? args.scopeId : undefined;
    if (scopeId !== undefined && !this.#scopes.has(scopeId)) {
      throw new TypeError("ShapeScope belongs to an expired engine scope");
    }
    const routedArgs = { ...args };
    delete routedArgs.scopeId;
    visitEngineRefs(routedArgs, () => undefined);
    const result = await this.#engine.request(operation, routedArgs, options);
    if (scopeId !== undefined) {
      const owned = this.#scopes.get(scopeId)!;
      visitEngineRefs(result, (ref) => owned.add(ref.logicalId));
    }
    return result;
  }

  #requiredScope(args: Record<string, unknown>): number {
    if (typeof args.scopeId !== "number") throw new TypeError("scopeId is required");
    return args.scopeId;
  }
}
