import type { BinaryBuffer, Capabilities, MaterializedBuffer, RequestOptions } from "./types.js";
import { mapProtocolValue, mapShapeHandles } from "./protocol-codec.js";
import { HISTORY_SUPPORT, OPERATION_RESULT_HANDLE_PATHS, PROTOCOL_VERSION, type OperationName } from "./generated.js";
import { BUILD_IDENTITY } from "./build-identity.generated.js";
import { RUNTIME_CONFIG } from "./runtime-manifest.generated.js";
import type { ModulesManifest } from "./runtime-config.js";

// Isolated-profile geometry engine: shape placement,
// clone-based cross-profile transfer, and manifest-driven routing.
//
// Runtime contract (docs/architecture.md):
//   - every Profile is an independent runtime (static WASM + Worker in
//     production); it is created lazily on first use and never proactively.
//   - a logical shape may own several placements across profiles; cloning
//     never invalidates the source placement.
//   - transfers use a versioned envelope around the kernel BREP payload with
//     build-family identity and checksum; mismatches are structured errors.
//   - routing derives operation eligibility from protocol/modules.json and
//     never silently falls back across profiles.

export const TRANSFER_MAGIC = "occt-shape-transfer";
export const TRANSFER_VERSION = 1;

export type EngineErrorCode =
  | "UnsupportedCapability"
  | "TransferChecksumMismatch"
  | "TransferVersionMismatch"
  | "TransferBuildFamilyMismatch"
  | "TransferPayloadTruncated"
  | "AmbiguousProfileRouting"
  | "UnknownLogicalShape"
  | "InvalidPlacementState";

export class EngineError extends Error {
  constructor(
    readonly code: EngineErrorCode,
    message: string,
    readonly details?: Record<string, unknown>,
  ) {
    super(message);
    this.name = "EngineError";
  }
}

/** Minimal request surface the engine needs from a profile runtime. */
export interface ProfileRuntime {
  /** Changes whenever the underlying runtime is rebuilt and loses its handles. */
  readonly epoch?: number;
  request<T = unknown>(
    op: OperationName,
    args: Record<string, unknown>,
    options?: RequestOptions,
  ): Promise<T>;
  close?(): void | Promise<void>;
}

export type ProfileRuntimeFactory = () => Promise<ProfileRuntime>;

interface Placement {
  profileId: string;
  epoch: number;
  handle: number;
  scopeId: number;
  leaseCount: number;
  state: "live" | "transferring" | "invalid";
}

interface LogicalShape {
  id: number;
  placements: Map<string, Placement>;
}

const RUNTIME_OPS: ReadonlySet<OperationName> = new Set([
  "capabilities", "beginScope", "endScope", "release", "releaseAll",
  "createBuffer", "freeBuffer", "stats",
]);

function fnv1a(bytes: Uint8Array): number {
  let hash = 0x811c9dc5;
  for (let i = 0; i < bytes.length; ++i) {
    hash ^= bytes[i] ?? 0;
    hash = Math.imul(hash, 0x01000193) >>> 0;
  }
  return hash >>> 0;
}

interface EnvelopeHeader {
  magic: string;
  version: number;
  buildFamily: string;
  payloadBytes: number;
  checksum: number;
}

export function encodeEnvelope(buildFamily: string, payloadInput: ArrayBufferLike | ArrayBufferView): ArrayBuffer {
  const payloadBytesView: Uint8Array<ArrayBuffer> = ArrayBuffer.isView(payloadInput)
    ? new Uint8Array(payloadInput.buffer as ArrayBuffer, payloadInput.byteOffset, payloadInput.byteLength)
    : new Uint8Array(payloadInput as ArrayBuffer);
  const header: EnvelopeHeader = {
    magic: TRANSFER_MAGIC,
    version: TRANSFER_VERSION,
    buildFamily,
    payloadBytes: payloadBytesView.byteLength,
    checksum: fnv1a(payloadBytesView),
  };
  const headerBytes = new TextEncoder().encode(JSON.stringify(header));
  const frame = new Uint8Array(4 + headerBytes.length + payloadBytesView.byteLength);
  new DataView(frame.buffer).setUint32(0, headerBytes.length, true);
  frame.set(headerBytes, 4);
  frame.set(payloadBytesView, 4 + headerBytes.length);
  return frame.buffer.slice(frame.byteOffset, frame.byteOffset + frame.byteLength);
}

export function decodeEnvelope(frame: ArrayBuffer, expectedBuildFamily: string): ArrayBuffer {
  if (frame.byteLength < 8) throw new EngineError("TransferPayloadTruncated", "transfer frame shorter than envelope");
  const view = new DataView(frame);
  const headerLength = view.getUint32(0, true);
  if (4 + headerLength > frame.byteLength) throw new EngineError("TransferPayloadTruncated", "envelope header exceeds frame");
  let header: EnvelopeHeader;
  try {
    header = JSON.parse(new TextDecoder().decode(new Uint8Array(frame, 4, headerLength))) as EnvelopeHeader;
  } catch {
    throw new EngineError("TransferPayloadTruncated", "envelope header is not valid JSON");
  }
  if (header.magic !== TRANSFER_MAGIC) throw new EngineError("TransferVersionMismatch", "unknown transfer magic");
  if (header.version !== TRANSFER_VERSION) {
    throw new EngineError("TransferVersionMismatch", `unsupported transfer version ${header.version}`);
  }
  if (header.buildFamily !== expectedBuildFamily) {
    throw new EngineError(
      "TransferBuildFamilyMismatch",
      `payload build family '${header.buildFamily}' does not match target '${expectedBuildFamily}'`,
    );
  }
  const payloadOffset = 4 + headerLength;
  const actual = frame.byteLength - payloadOffset;
  if (actual !== header.payloadBytes) {
    throw new EngineError("TransferPayloadTruncated", `payload ${actual} bytes, envelope declares ${header.payloadBytes}`);
  }
  const payload = frame.slice(payloadOffset);
  if (fnv1a(new Uint8Array(payload)) !== header.checksum) {
    throw new EngineError("TransferChecksumMismatch", "payload checksum mismatch");
  }
  return payload;
}

interface ProfileState {
  buildFamily: string;
  factory: ProfileRuntimeFactory;
  runtime: ProfileRuntime | undefined;
  runtimeEpoch: number | undefined;
  starting: Promise<ProfileRuntime> | undefined;
  epoch: number;
}

export class GeometryEngine {
  readonly #profiles = new Map<string, ProfileState>();
  readonly #shapes = new Map<number, LogicalShape>();
  readonly #opEligibility = new Map<OperationName, string[]>();
  readonly #scopeRefs = new Map<string, number>();
  #nextShapeId = 1;
  #nextPlacementId = 1;
  #disposed = false;
  #closePromise: Promise<void> | undefined;

  constructor(manifest: ModulesManifest = RUNTIME_CONFIG.modules) {
    const opsByArtifact = new Map<string, OperationName[]>();
    for (const [artifact, modules] of Object.entries(manifest.artifactModuleCandidates)) {
      const ops: OperationName[] = [];
      for (const moduleName of modules) {
        for (const op of manifest.semanticModules[moduleName] ?? []) ops.push(op);
      }
      opsByArtifact.set(artifact, ops);
    }
    const addEligible = (op: OperationName, profileId: string) => {
      const list = this.#opEligibility.get(op) ?? [];
      if (!list.includes(profileId)) list.push(profileId);
      this.#opEligibility.set(op, list);
    };
    for (const op of manifest.semanticModules.runtime ?? []) {
      for (const profileId of Object.keys(manifest.profiles)) addEligible(op, profileId);
    }
    for (const op of manifest.transferOperations ?? []) {
      for (const profileId of Object.keys(manifest.profiles)) addEligible(op, profileId);
    }
    for (const [profileId, entry] of Object.entries(manifest.profiles)) {
      for (const artifact of entry.artifacts) {
        for (const op of opsByArtifact.get(artifact) ?? []) addEligible(op, profileId);
      }
    }
  }

  registerProfile(profileId: string, buildFamily: string, factory: ProfileRuntimeFactory): void {
    this.#assertActive();
    if (this.#profiles.has(profileId)) {
      throw new EngineError("InvalidPlacementState", `profile '${profileId}' is already registered`);
    }
    this.#profiles.set(profileId, {
      buildFamily,
      factory,
      runtime: undefined,
      runtimeEpoch: undefined,
      starting: undefined,
      epoch: 1,
    });
  }

  eligibleProfiles(operation: OperationName): readonly string[] {
    return [...(this.#opEligibility.get(operation) ?? [])];
  }

  availableOperations(): OperationName[] {
    return [...this.#opEligibility]
      .filter(([, profiles]) => profiles.some((profileId) => this.#profiles.has(profileId)))
      .map(([operation]) => operation)
      .sort();
  }

  async capabilities(): Promise<Capabilities> {
    this.#assertActive();
    if (this.#profiles.size === 0) {
      throw new EngineError("UnsupportedCapability", "no profiles are registered");
    }
    // Reports manifest and build-time identity only. Use probeProfile() when
    // checking capabilities actually registered by a WASM artifact.
    const operations = this.availableOperations();
    const historySupport: Capabilities["historySupport"] = {};
    for (const operation of operations) historySupport[operation] = HISTORY_SUPPORT[operation];
    return {
      protocolVersion: PROTOCOL_VERSION,
      kernelVersion: PROTOCOL_VERSION,
      occtVersion: BUILD_IDENTITY.occtVersion,
      ops: operations,
      historySupport,
      buildFlags: { ...BUILD_IDENTITY.buildFlags },
    };
  }

  async probeProfile(profileId: string): Promise<Capabilities> {
    this.#assertActive();
    if (!this.#profiles.has(profileId)) {
      throw new EngineError("UnsupportedCapability", `unknown profile '${profileId}'`);
    }
    const runtime = await this.#runtimeFor(profileId);
    return runtime.request<Capabilities>("capabilities", {});
  }

  stats(): { shapes: number; placements: number; startedProfiles: string[] } {
    let placements = 0;
    for (const shape of this.#shapes.values()) placements += shape.placements.size;
    return {
      shapes: this.#shapes.size,
      placements,
      startedProfiles: [...this.#profiles.entries()]
        .filter(([, state]) => state.runtime !== undefined || state.starting !== undefined)
        .map(([id]) => id),
    };
  }

  close(): Promise<void> {
    if (this.#closePromise !== undefined) return this.#closePromise;
    if (this.#disposed) return Promise.resolve();
    this.#disposed = true;
    const closing = (async (): Promise<void> => {
      const runtimes = [...this.#profiles.values()]
        .map((state) => state.runtime)
        .filter((runtime): runtime is ProfileRuntime => runtime !== undefined);
      const starting = [...this.#profiles.values()]
        .map((state) => state.starting)
        .filter((runtime): runtime is Promise<ProfileRuntime> => runtime !== undefined);
      for (const state of this.#profiles.values()) {
        state.runtime = undefined;
        state.runtimeEpoch = undefined;
        state.starting = undefined;
        state.epoch += 1;
      }
      let firstError: unknown;
      const recordError = (error: unknown): void => {
        if (firstError === undefined) firstError = error;
      };
      await Promise.all(runtimes.map(async (runtime) => {
        await runtime.request("releaseAll", {}).catch(() => undefined);
        try {
          await runtime.close?.();
        } catch (error) {
          recordError(error);
        }
      }));
      await Promise.allSettled(starting);
      this.#shapes.clear();
      this.#scopeRefs.clear();
      if (firstError !== undefined) throw firstError;
    })();
    this.#closePromise = closing;
    return closing;
  }

  // -------------------------------------------------------------------------
  // Shape creation / routing
  // -------------------------------------------------------------------------

  async create(
    profileId: string,
    operation: OperationName,
    args: Record<string, unknown> = {},
    options: RequestOptions = {},
  ): Promise<EngineShapeRef> {
    this.#assertActive();
    if (RUNTIME_OPS.has(operation)) throw new EngineError("UnsupportedCapability", `use lifecycle APIs for '${operation}'`);
    if (!this.eligibleProfiles(operation).includes(profileId)) {
      throw new EngineError("UnsupportedCapability", `profile '${profileId}' cannot run '${operation}'`);
    }
    const { runtime, scopeId } = await this.#openScope(profileId, options);
    try {
      const result = await runtime.request<{ shape: number }>(operation, { ...args, scopeId }, options);
      this.#assertActive();
      return this.#attachPlacement(profileId, scopeId, result.shape);
    } catch (error) {
      await runtime.request("endScope", { scopeId }).catch(() => undefined);
      throw error;
    }
  }

  /** Routes an operation to its unique eligible profile, co-locating inputs. */
  async request(
    operation: OperationName,
    args: Record<string, unknown>,
    options: RequestOptions = {},
  ): Promise<unknown> {
    this.#assertActive();
    if (RUNTIME_OPS.has(operation)) throw new EngineError("UnsupportedCapability", `use lifecycle APIs for '${operation}'`);
    if ("scopeId" in args) {
      throw new EngineError("InvalidPlacementState", "scopeId is managed by GeometryEngine");
    }
    const inputs: EngineShapeRef[] = [];
    const seenInputs = new Set<string>();
    mapProtocolValue(args, (value) => {
      if (value instanceof EngineShapeRef) {
        const key = `${value.logicalId}:${value.profileId}:${value.placementId}:${value.epoch}`;
        if (!seenInputs.has(key)) {
          seenInputs.add(key);
          inputs.push(value);
        }
      }
      return value;
    });
    const shapes = inputs.map((ref) => {
      ref.assertOwner(this);
      const shape = this.#logical(ref.logicalId);
      this.#placementForRef(shape, ref);
      return shape;
    });
    const targetProfile = this.#uniqueTarget(operation, inputs, shapes);

    const handles = new Map<string, number>();
    const refKey = (ref: EngineShapeRef): string =>
      `${ref.logicalId}:${ref.profileId}:${ref.placementId}:${ref.epoch}`;
    const leased: Placement[] = [];
    try {
      for (let index = 0; index < shapes.length; ++index) {
        const ref = inputs[index]!;
        const shape = shapes[index]!;
        let placement = this.#placementForRef(shape, ref);
        if (ref.profileId !== targetProfile) {
          const clonedRef = await this.cloneTo(ref, targetProfile, options);
          placement = this.#placementForRef(shape, clonedRef);
        }
        placement.leaseCount += 1;
        leased.push(placement);
        handles.set(refKey(ref), placement.handle);
      }

      const encodeShape = (value: unknown): unknown => {
        if (value instanceof EngineShapeRef) {
          value.assertOwner(this);
          const handle = handles.get(refKey(value));
          if (handle === undefined) throw new EngineError("InvalidPlacementState", "shape input was not declared for routing");
          return handle;
        }
        return value;
      };
      const encoded: Record<string, unknown> = {};
      for (const [key, value] of Object.entries(args)) {
        encoded[key] = mapProtocolValue(value, encodeShape);
      }
      let outputScope: { runtime: ProfileRuntime; scopeId: number } | undefined;
      if (leased[0] !== undefined) encoded.scopeId = leased[0].scopeId;
      else {
        outputScope = await this.#openScope(targetProfile, options);
        encoded.scopeId = outputScope.scopeId;
      }

      const runtime = await this.#runtimeFor(targetProfile);
      try {
        const result = await runtime.request(operation, encoded, options);
        this.#assertActive();
        const scopeId = encoded.scopeId as number;
        const materialized = this.#materializeResult(operation, result, targetProfile, scopeId, args);
        if (outputScope !== undefined && materialized.shapeCount === 0) {
          await runtime.request("endScope", { scopeId }).catch(() => undefined);
        }
        return materialized.value;
      } catch (error) {
        if (outputScope !== undefined) {
          await runtime.request("endScope", { scopeId: outputScope.scopeId }).catch(() => undefined);
        }
        throw error;
      }
    } finally {
      for (const placement of leased) placement.leaseCount -= 1;
    }
  }

  // -------------------------------------------------------------------------
  // Transfer (clone semantics: source placement always survives)
  // -------------------------------------------------------------------------

  async cloneTo(
    ref: EngineShapeRef,
    targetProfileId: string,
    options: RequestOptions = {},
  ): Promise<EngineShapeRef> {
    this.#assertActive();
    ref.assertOwner(this);
    const shape = this.#logical(ref.logicalId);
    const source = this.#placementForRef(shape, ref);
    if (!this.#profiles.has(targetProfileId)) {
      throw new EngineError("UnsupportedCapability", `profile '${targetProfileId}' is not registered`);
    }
    if (source.state === "transferring") {
      throw new EngineError("InvalidPlacementState", "a transfer is already in flight for this placement");
    }
    const existing = [...shape.placements].find(
      ([, placement]) => placement.profileId === targetProfileId && placement.state === "live",
    );
    if (existing !== undefined) {
      const [placementId, placement] = existing;
      return new EngineShapeRef(this, shape.id, targetProfileId, placementId, placement.epoch, engineShapeProvenance);
    }

    source.state = "transferring";
    source.leaseCount += 1;
    const sourceEpoch = source.epoch;
    try {
      const sourceRuntime = await this.#runtimeFor(ref.profileId);
      const payload = await sourceRuntime.request<{ data: MaterializedBuffer<BinaryBuffer> }>(
        "exportBREP",
        { shape: source.handle },
        options,
      );
      this.#assertActive();
      if (source.state !== "transferring" || this.#profileState(ref.profileId).epoch !== sourceEpoch) {
        throw new EngineError("InvalidPlacementState", "source placement changed during transfer");
      }
      // DirectClient materializes buffers as { layout, data } where data is
      // the actual ArrayBuffer.
      const rawPayload = payload.data.data;
      const frame = encodeEnvelope(this.#buildFamily(ref.profileId), rawPayload);

      const target = await this.#openScope(targetProfileId, options);
      const targetEpoch = this.#profileState(targetProfileId).epoch;
      try {
        const payloadOnly = decodeEnvelope(frame, this.#buildFamily(targetProfileId));
        const imported = await target.runtime.request<{ shape: number }>("importBREP", {
          scopeId: target.scopeId,
          data: { $inputBuffer: payloadOnly },
        }, options);
        this.#assertActive();
        if (this.#profileState(targetProfileId).epoch !== targetEpoch) {
          throw new EngineError("InvalidPlacementState", "target profile changed during transfer");
        }
        const placementId = `${targetProfileId}@${imported.shape}#${this.#nextPlacementId++}`;
        const placement: Placement = {
          profileId: targetProfileId,
          epoch: this.#profileState(targetProfileId).epoch,
          handle: imported.shape,
          scopeId: target.scopeId,
          leaseCount: 0,
          state: "live",
        };
        shape.placements.set(placementId, placement);
        this.#retainScope(targetProfileId, target.scopeId);
        return new EngineShapeRef(
          this,
          shape.id,
          targetProfileId,
          placementId,
          placement.epoch,
          engineShapeProvenance,
        );
      } catch (error) {
        await target.runtime.request("endScope", { scopeId: target.scopeId }).catch(() => undefined);
        throw error;
      }
    } finally {
      if (source.state === "transferring" && this.#profileState(ref.profileId).epoch === sourceEpoch) {
        source.state = "live";
      }
      source.leaseCount -= 1;
    }
  }

  // -------------------------------------------------------------------------
  // Lifecycle semantics
  // -------------------------------------------------------------------------

  async releasePlacement(ref: EngineShapeRef): Promise<void> {
    this.#assertActive();
    ref.assertOwner(this);
    const shape = this.#logical(ref.logicalId);
    const placement = this.#placementForRef(shape, ref);
    if (placement.leaseCount > 0) throw new EngineError("InvalidPlacementState", "placement is leased");
    await this.#releasePlacement(shape, ref.placementId, placement);
    if (shape.placements.size === 0) this.#shapes.delete(shape.id);
  }

  async releaseShape(logicalId: number): Promise<void> {
    this.#assertActive();
    const shape = this.#logical(logicalId);
    if ([...shape.placements.values()].some((placement) => placement.leaseCount > 0)) {
      throw new EngineError("InvalidPlacementState", "cannot release a shape with leased placements");
    }
    let firstError: unknown;
    for (const [key, placement] of [...shape.placements]) {
      try {
        await this.#releasePlacement(shape, key, placement);
      } catch (error) {
        if (firstError === undefined) firstError = error;
      }
    }
    if (shape.placements.size === 0) this.#shapes.delete(logicalId);
    if (firstError !== undefined) throw firstError;
  }

  async releaseAll(): Promise<void> {
    this.#assertActive();
    for (const shape of this.#shapes.values()) {
      if ([...shape.placements.values()].some((placement) => placement.leaseCount > 0)) {
        throw new EngineError("InvalidPlacementState", "cannot release shapes with leased placements");
      }
    }
    let firstError: unknown;
    for (const logicalId of [...this.#shapes.keys()]) {
      try {
        await this.releaseShape(logicalId);
      } catch (error) {
        if (firstError === undefined) firstError = error;
      }
    }
    if (firstError !== undefined) throw firstError;
  }

  /** Worker-crash semantics: every placement of one profile becomes invalid. */
  invalidateProfileEpoch(profileId: string): void {
    const state = this.#profiles.get(profileId);
    if (state) {
      void Promise.resolve(state.runtime?.close?.()).catch(() => undefined);
      state.runtime = undefined;
      state.runtimeEpoch = undefined;
      state.starting = undefined;
      state.epoch += 1;
    }
    this.#invalidatePlacements(profileId);
  }

  // -------------------------------------------------------------------------
  // Internals
  // -------------------------------------------------------------------------

  #assertActive(): void {
    if (this.#disposed) throw new EngineError("InvalidPlacementState", "engine disposed");
  }

  #profileState(profileId: string): ProfileState {
    const state = this.#profiles.get(profileId);
    if (!state) throw new EngineError("UnsupportedCapability", `profile '${profileId}' is not registered`);
    const runtimeEpoch = state.runtime?.epoch;
    if (runtimeEpoch !== undefined) {
      if (state.runtimeEpoch !== undefined && runtimeEpoch !== state.runtimeEpoch) {
        state.epoch += 1;
        this.#invalidatePlacements(profileId);
      }
      state.runtimeEpoch = runtimeEpoch;
    }
    return state;
  }

  #invalidatePlacements(profileId: string): void {
    for (const shape of this.#shapes.values()) {
      for (const placement of shape.placements.values()) {
        if (placement.profileId === profileId) placement.state = "invalid";
      }
    }
  }

  #buildFamily(profileId: string): string {
    return this.#profileState(profileId).buildFamily;
  }

  async #runtimeFor(profileId: string): Promise<ProfileRuntime> {
    const state = this.#profileState(profileId);
    if (state.runtime) return state.runtime;
    if (!state.starting) {
      const epoch = state.epoch;
      const starting = state.factory()
        .then(async (runtime) => {
          if (this.#disposed || state.epoch !== epoch) {
            await runtime.close?.();
            throw new EngineError("InvalidPlacementState", `profile '${profileId}' started after it was invalidated`);
          }
          state.runtime = runtime;
          state.runtimeEpoch = runtime.epoch;
          if (state.starting === starting) state.starting = undefined;
          return runtime;
        })
        .catch((error) => {
          if (state.starting === starting) state.starting = undefined;
          throw error;
        });
      state.starting = starting;
    }
    return state.starting;
  }

  async #openScope(
    profileId: string,
    options: RequestOptions = {},
  ): Promise<{ runtime: ProfileRuntime; scopeId: number }> {
    const runtime = await this.#runtimeFor(profileId);
    const result = await runtime.request<{ scopeId: number }>("beginScope", {}, options);
    return { runtime, scopeId: result.scopeId };
  }

  #logical(logicalId: number): LogicalShape {
    const shape = this.#shapes.get(logicalId);
    if (!shape) throw new EngineError("UnknownLogicalShape", `logical shape ${logicalId} does not exist`);
    return shape;
  }

  #placementForRef(shape: LogicalShape, ref: EngineShapeRef): Placement {
    this.#profileState(ref.profileId);
    const placement = shape.placements.get(ref.placementId);
    if (
      placement === undefined
      || placement.profileId !== ref.profileId
      || placement.epoch !== ref.epoch
      || placement.state !== "live"
    ) {
      throw new EngineError("InvalidPlacementState", "shape placement is stale or no longer live");
    }
    return placement;
  }

  #uniqueTarget(
    operation: OperationName,
    refs: readonly EngineShapeRef[],
    shapes: readonly LogicalShape[],
  ): string {
    const eligible = this.eligibleProfiles(operation);
    if (eligible.length === 0) {
      throw new EngineError("UnsupportedCapability", `operation '${operation}' has no eligible profile`);
    }

    // Batch must execute inside a single profile; never split silently.
    const requestedProfiles = new Set(refs.map((ref) => ref.profileId));
    if (operation === "batch" && requestedProfiles.size > 1) {
        throw new EngineError(
          "AmbiguousProfileRouting",
          "batch operations must run inside a single profile",
          { profiles: [...requestedProfiles] },
        );
    }

    if (requestedProfiles.size === 1) {
      const requested = refs[0]!.profileId;
      if (eligible.includes(requested)) return requested;
    }

    const registered = eligible.filter((profileId) => this.#profiles.has(profileId));
    if (registered.length === 0) {
      throw new EngineError("UnsupportedCapability", `no registered profile can run '${operation}'`, { eligible });
    }
    const common = registered.filter((profileId) => shapes.every((shape) =>
      [...shape.placements.values()].some((placement) => placement.profileId === profileId && placement.state === "live"),
    ));
    if (common.length > 0) return common[0]!;
    return registered[0]!;
  }

  #attachPlacement(profileId: string, scopeId: number, handle: number): EngineShapeRef {
    const placementId = `${profileId}@${handle}#${this.#nextPlacementId++}`;
    const placement: Placement = {
      profileId,
      epoch: this.#profileState(profileId).epoch,
      handle,
      scopeId,
      leaseCount: 0,
      state: "live",
    };
    const logical: LogicalShape = {
      id: this.#nextShapeId++,
      placements: new Map(),
    };
    logical.placements.set(placementId, placement);
    this.#shapes.set(logical.id, logical);
    this.#retainScope(profileId, scopeId);
    return new EngineShapeRef(
      this,
      logical.id,
      profileId,
      placementId,
      placement.epoch,
      engineShapeProvenance,
    );
  }

  #materializeResult(
    operation: OperationName,
    value: unknown,
    profileId: string,
    scopeId: number,
    args: Record<string, unknown>,
  ): { value: unknown; shapeCount: number } {
    const refs = new Map<number, EngineShapeRef>();
    let shapeCount = 0;
    const materialize = (handle: number): EngineShapeRef => {
      let ref = refs.get(handle);
      if (ref === undefined) {
        ref = this.#attachPlacement(profileId, scopeId, handle);
        refs.set(handle, ref);
        shapeCount += 1;
      }
      return ref;
    };
    const batchOperations = args.ops;
    if (
      operation === "batch"
      && value !== null
      && typeof value === "object"
      && Array.isArray((value as { results?: unknown }).results)
      && Array.isArray(batchOperations)
    ) {
      const source = value as Record<string, unknown> & { results: unknown[] };
      const results = source.results.map((entry, index) => {
        const item = batchOperations[index];
        if (item === null || typeof item !== "object") return entry;
        const nestedOperation = (item as { op?: unknown }).op;
        if (typeof nestedOperation !== "string" || !(nestedOperation in OPERATION_RESULT_HANDLE_PATHS)) return entry;
        return mapShapeHandles(
          entry,
          OPERATION_RESULT_HANDLE_PATHS[nestedOperation as OperationName],
          materialize,
        );
      });
      return { value: { ...source, results }, shapeCount };
    }
    return { value: mapShapeHandles(value, OPERATION_RESULT_HANDLE_PATHS[operation], materialize), shapeCount };
  }

  #retainScope(profileId: string, scopeId: number): void {
    const key = `${profileId}@${scopeId}`;
    this.#scopeRefs.set(key, (this.#scopeRefs.get(key) ?? 0) + 1);
  }

  async #releasePlacement(shape: LogicalShape, key: string, placement: Placement): Promise<void> {
    const state = this.#profileState(placement.profileId);
    if (placement.epoch === state.epoch && state.runtime !== undefined) {
      await state.runtime.request("release", { shape: placement.handle });
    }
    shape.placements.delete(key);
    const scopeKey = `${placement.profileId}@${placement.scopeId}`;
    const remaining = (this.#scopeRefs.get(scopeKey) ?? 1) - 1;
    if (remaining === 0) {
      this.#scopeRefs.delete(scopeKey);
      if (placement.epoch === state.epoch && state.runtime !== undefined) {
        await state.runtime.request("endScope", { scopeId: placement.scopeId }).catch(() => undefined);
      }
    } else {
      this.#scopeRefs.set(scopeKey, remaining);
    }
  }
}

const engineShapeProvenance = Symbol("engineShapeProvenance");

/** Opaque reference handed to callers; carries routing metadata only. */
export class EngineShapeRef {
  constructor(
    private readonly engine: GeometryEngine,
    readonly logicalId: number,
    readonly profileId: string,
    readonly placementId: string,
    readonly epoch: number,
    provenance: typeof engineShapeProvenance,
  ) {
    if (provenance !== engineShapeProvenance) {
      throw new TypeError("EngineShapeRef values can only be created from engine results");
    }
  }

  assertOwner(engine: GeometryEngine): void {
    if (this.engine !== engine) throw new EngineError("InvalidPlacementState", "shape belongs to a different engine");
  }

  cloneTo(targetProfileId: string): Promise<EngineShapeRef> {
    return this.engine.cloneTo(this, targetProfileId);
  }

  releasePlacement(): Promise<void> {
    return this.engine.releasePlacement(this);
  }
}
