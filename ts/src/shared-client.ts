import { BaseClient } from "./client.js";
import { InProcessTransport } from "./in-process-transport.js";
import { HISTORY_SUPPORT, OPERATIONS, type OperationName } from "./generated.js";
import { resolveArtifact, type ResolveOptions } from "./artifact-resolver.js";
import { RUNTIME_CONFIG } from "./runtime-manifest.generated.js";
import type { SharedRuntimeConfig } from "./runtime-config.js";
import {
  SharedKernelLoader,
  SharedKernelLoadError,
  type SharedMainFactory,
  type SideDescriptor,
} from "./shared-loader.js";
import type {
  Capabilities,
  RequestOptions,
} from "./types.js";

export interface SharedClientOptions extends ResolveOptions {
  mainJsUrl?: string;
  mainWasmUrl?: string;
  importFactory(mainJsUrl: string): Promise<SharedMainFactory>;
  config?: SharedRuntimeConfig;
}

/**
 * BaseClient adapter for one shared Main plus dynamically loaded Side modules.
 * Side modules remain resident for this Main epoch; disposing this client
 * invalidates all handles and the caller may then terminate its Worker.
 */
export class SharedClient extends BaseClient {
  readonly #loader: SharedKernelLoader;
  readonly #buildFamily: string;
  readonly #sides: ReadonlyMap<string, SideDescriptor>;
  readonly #operationSides: Readonly<Partial<Record<OperationName, string>>>;
  readonly #transport: InProcessTransport;
  #closed = false;

  private constructor(
    loader: SharedKernelLoader,
    transport: InProcessTransport,
    config: SharedRuntimeConfig,
    sides: readonly SideDescriptor[],
  ) {
    super();
    this.#loader = loader;
    this.#transport = transport;
    this.#buildFamily = config.buildFamily;
    this.#sides = new Map(sides.map((side) => [side.name, side]));
    this.#operationSides = config.operationSides;
  }

  static async create(options: SharedClientOptions): Promise<SharedClient> {
    const config = options.config ?? RUNTIME_CONFIG.shared;
    const [mainJsUrl, mainWasmUrl, ...sideUrls] = await Promise.all([
      options.mainJsUrl ?? resolveArtifact(config.mainJs, options),
      options.mainWasmUrl ?? resolveArtifact(config.mainWasm, options),
      ...config.sides.map((side) => resolveArtifact(side, options)),
    ]);
    const sides = config.sides.map((side, index) => ({ name: side.name, url: sideUrls[index]! }));
    const sideNames = new Set<string>();
    for (const side of sides) {
      if (sideNames.has(side.name)) throw new TypeError(`Duplicate shared side '${side.name}'`);
      sideNames.add(side.name);
    }
    for (const [operation, sideName] of Object.entries(config.operationSides)) {
      if (!(OPERATIONS as readonly string[]).includes(operation)) {
        throw new TypeError(`Unknown shared operation '${operation}'`);
      }
      if (!sideNames.has(sideName)) {
        throw new TypeError(`Shared operation '${operation}' maps to unconfigured side '${sideName}'`);
      }
    }

    let transport: InProcessTransport | undefined;
    const loader = new SharedKernelLoader({
      importFactory: options.importFactory,
      mainWasmUrl,
      isCancelled: () => transport?.isCancelled() ?? false,
      onProgress: (fraction) => transport?.reportProgress(fraction),
    });
    try {
      await loader.instantiate(mainJsUrl);
      const actualBuildFamily = loader.module.UTF8ToString(loader.module._occt_host_build_family());
      if (actualBuildFamily !== config.buildFamily) {
        throw new SharedKernelLoadError(
          `shared Main build family '${actualBuildFamily}' does not match configured '${config.buildFamily}'`,
          "BuildFamilyMismatch",
        );
      }
      let client: SharedClient;
      const module = loader.module;
      transport = new InProcessTransport({
        bytes: () => module.HEAPU8,
        alloc: (length) => module._k_alloc(length),
        free: (pointer) => module._k_free(pointer),
        handle: (pointer, length) => module._k_handle(pointer, length),
        responsePointer: () => module._k_response_ptr(),
        bufferPointer: (bufferId) => module._k_buffer_ptr(bufferId),
        bufferLength: (bufferId) => module._k_buffer_len(bufferId),
      }, (error) => client.kernelError(error));
      client = new SharedClient(loader, transport, config, sides);
      await client.initialize();
      return client;
    } catch (error) {
      loader.dispose();
      throw error;
    }
  }

  close(): void {
    if (this.#closed) return;
    this.#closed = true;
    this.#loader.dispose();
    this.invalidateEpoch();
  }

  protected async send<T>(
    operation: OperationName,
    args: Record<string, unknown>,
    options: RequestOptions = {},
  ): Promise<T> {
    if (this.#closed) throw new Error("SharedClient closed");
    await this.#ensureSides(operation, args);
    return this.#transport.request<T>(operation, args, options);
  }

  #sideNamesFor(operation: OperationName, args: Record<string, unknown>): string[] {
    const names: string[] = [];
    const add = (op: unknown, operationArgs?: unknown): void => {
      if (typeof op !== "string" || !(OPERATIONS as readonly string[]).includes(op)) return;
      const sideName = this.#operationSides[op as OperationName];
      if (sideName !== undefined && !names.includes(sideName)) names.push(sideName);
      if (op === "batch" && operationArgs !== null && typeof operationArgs === "object") {
        const nested = (operationArgs as { ops?: unknown }).ops;
        if (Array.isArray(nested)) {
          for (const item of nested) {
            if (item !== null && typeof item === "object") {
              const request = item as { op?: unknown; args?: unknown };
              add(request.op, request.args);
            }
          }
        }
      }
    };
    add(operation, args);
    return names;
  }

  async #ensureSides(operation: OperationName, args: Record<string, unknown>): Promise<void> {
    for (const sideName of this.#sideNamesFor(operation, args)) {
      const side = this.#sides.get(sideName);
      if (side === undefined) throw new Error(`shared side '${sideName}' is not configured`);
      await this.#loader.ensureSide(side, this.#buildFamily);
    }
  }

  override async initialize(): Promise<Capabilities> {
    const capabilities = this.validateCapabilities(
      this.#transport.call("capabilities", {}) as Capabilities,
    );
    const ops = [...new Set<OperationName>([
      ...capabilities.ops,
      ...Object.keys(this.#operationSides) as OperationName[],
    ])].sort();
    return {
      ...capabilities,
      ops,
      historySupport: Object.fromEntries(ops.map((operation) => [
        operation,
        capabilities.historySupport[operation] ?? HISTORY_SUPPORT[operation],
      ])),
    };
  }
}
