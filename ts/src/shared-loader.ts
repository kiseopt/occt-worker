// Shared Main + Side loader (stage 4).
//
// Responsibilities (see tmp-docs/wasm-implementation-runbook.zh-CN.md §7):
//   - instantiate the shared Main module from its ES6 glue factory
//   - asynchronously load Side modules via Module.loadDynamicLibrary before
//     the first k_handle call that needs them
//   - de-duplicate concurrent first loads of the same Side behind one Promise
//   - validate build-family identity and descriptor registration atomically
//   - treat Sides as nodelete: reclamation happens by destroying the Worker
//   - expose an epoch that invalidates all handles after a rebuild

export interface SideDescriptor {
  /** Stable artifact name, e.g. "mesh.side.wasm". */
  name: string;
  /** URL/path resolved relative to the worker's own module. */
  url: string;
  semanticModules?: readonly string[];
}

export interface SharedMainFactoryOptions {
  locateFile?: (path: string, prefix: string) => string;
  occtWorkerCancelled?: () => boolean;
  occtWorkerProgress?: (fraction: number) => void;
}

export type SharedMainFactory = (options?: SharedMainFactoryOptions) => Promise<SharedMainModule>;

export interface SharedMainModule {
  loadDynamicLibrary(
    url: string,
    flags?: { global?: boolean; nodelete?: boolean; loadAsync?: boolean },
  ): Promise<boolean>;
  UTF8ToString(pointer: number): string;
  _k_alloc(length: number): number;
  _k_free(pointer: number): void;
  _k_handle(requestPointer: number, length: number): number;
  _k_response_ptr(): number;
  _k_buffer_ptr(id: number): number;
  _k_buffer_len(id: number): number;
  _occt_host_build_family(): number;
  _occt_host_plugin_count(): number;
  _occt_host_plugin_error(): number;
  _occt_host_plugin_name(): number;
  memory: WebAssembly.Memory;
  HEAPU8: Uint8Array;
}

export class SharedKernelLoadError extends Error {
  constructor(
    message: string,
    readonly code:
      | "BuildFamilyMismatch"
      | "SideLoadFailed"
      | "RegistrationFailed"
      | "MainAlreadyInstantiated",
    readonly sideName?: string,
  ) {
    super(message);
    this.name = "SharedKernelLoadError";
  }
}

interface LoadedSideState {
  promise: Promise<void>;
  url: string;
  expectedBuildFamily: string;
}

export interface SharedLoaderHost {
  /** Imports the ES6 main glue and returns its modularized factory. */
  importFactory(mainJsUrl: string): Promise<SharedMainFactory>;
  /** Locates the main .wasm next to the glue. */
  mainWasmUrl: string;
  isCancelled?: () => boolean;
  onProgress?: (fraction: number) => void;
}

export class SharedKernelLoader {
  #module: SharedMainModule | undefined;
  #epoch = 0;
  #instantiating: Promise<void> | undefined;
  readonly #sides = new Map<string, LoadedSideState>();
  readonly #host: SharedLoaderHost;
  #sideLoadTail: Promise<void> = Promise.resolve();

  constructor(host: SharedLoaderHost) {
    this.#host = host;
  }

  get epoch(): number {
    return this.#epoch;
  }

  get loadedSides(): string[] {
    return [...this.#sides.keys()];
  }

  get module(): SharedMainModule {
    if (this.#module === undefined) {
      throw new SharedKernelLoadError("shared main not instantiated", "SideLoadFailed");
    }
    return this.#module;
  }

  async instantiate(mainJsUrl: string): Promise<void> {
    if (this.#module !== undefined || this.#instantiating !== undefined) {
      throw new SharedKernelLoadError("shared main is already instantiated", "MainAlreadyInstantiated");
    }
    const epoch = this.#epoch;
    const instantiating = (async (): Promise<void> => {
      const factory = await this.#host.importFactory(mainJsUrl);
      const mainBase = this.#host.mainWasmUrl.split(/[\\/]/).pop() ?? "";
      const module = await factory({
        locateFile: (path) => (path === mainBase ? this.#host.mainWasmUrl : path),
        ...(this.#host.isCancelled === undefined
          ? {}
          : { occtWorkerCancelled: this.#host.isCancelled }),
        ...(this.#host.onProgress === undefined
          ? {}
          : { occtWorkerProgress: this.#host.onProgress }),
      });
      if (this.#epoch !== epoch) {
        throw new SharedKernelLoadError("shared main instantiation became stale", "SideLoadFailed");
      }
      this.#module = module;
      this.#epoch += 1;
      this.#sides.clear();
    })();
    this.#instantiating = instantiating;
    try {
      await instantiating;
    } finally {
      if (this.#instantiating === instantiating) this.#instantiating = undefined;
    }
  }

  /**
   * Loads a Side exactly once per epoch. Concurrent callers share the same
   * in-flight promise; a rejected load is retried on the next call.
   */
  async ensureSide(side: SideDescriptor, expectedBuildFamily: string): Promise<void> {
    const module = this.#module;
    if (!module) throw new SharedKernelLoadError("shared main not instantiated", "SideLoadFailed");
    const epoch = this.#epoch;

    const reserved = this.#sides.get(side.name);
    if (reserved) {
      if (reserved.url !== side.url || reserved.expectedBuildFamily !== expectedBuildFamily) {
        throw new SharedKernelLoadError(
          `side '${side.name}' is already reserved with a different descriptor`,
          "SideLoadFailed",
          side.name,
        );
      }
      // Concurrent caller: share the exact in-flight promise.
      await reserved.promise;
      return;
    }

    const state: LoadedSideState = {
      promise: Promise.resolve(),
      url: side.url,
      expectedBuildFamily,
    };
    this.#sides.set(side.name, state);
    const load = async (): Promise<void> => {
      if (this.#module !== module || this.#epoch !== epoch) {
        throw new SharedKernelLoadError(`side '${side.name}' loaded into a stale Main epoch`, "SideLoadFailed", side.name);
      }
      const actualBuildFamily = module.UTF8ToString(module._occt_host_build_family());
      if (actualBuildFamily !== expectedBuildFamily) {
        throw new SharedKernelLoadError(
          `side '${side.name}' expects build family '${expectedBuildFamily}', Main is '${actualBuildFamily}'`,
          "BuildFamilyMismatch",
          side.name,
        );
      }
      const registrationCount = module._occt_host_plugin_count();
      try {
        const loaded = await module.loadDynamicLibrary(side.url, { loadAsync: true, global: true, nodelete: true });
        if (!loaded) throw new Error("loader returned false");
      } catch (error) {
        throw new SharedKernelLoadError(
          `side '${side.name}' failed to load: ${String(error)}`,
          "SideLoadFailed",
          side.name,
        );
      }
      if (this.#module !== module || this.#epoch !== epoch) {
        throw new SharedKernelLoadError(`side '${side.name}' loaded into a stale Main epoch`, "SideLoadFailed", side.name);
      }
      const registrationCountExpected = side.semanticModules?.length ?? 1;
      if (module._occt_host_plugin_error() !== 0 || module._occt_host_plugin_count() !== registrationCount + registrationCountExpected) {
        throw new SharedKernelLoadError(`side '${side.name}' descriptor registration failed`, "RegistrationFailed", side.name);
      }
    };
    const scheduled = this.#sideLoadTail.then(load);
    this.#sideLoadTail = scheduled.catch(() => undefined);
    state.promise = scheduled.catch((error: unknown) => {
      if (this.#sides.get(side.name) === state) this.#sides.delete(side.name);
      throw error;
    });
    await state.promise;
  }

  /** Destroys loader state; the caller must terminate the underlying worker. */
  dispose(): void {
    this.#epoch += 1;
    this.#sides.clear();
    this.#module = undefined;
  }
}
