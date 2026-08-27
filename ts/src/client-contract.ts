import type { OperationName } from "./generated.js";
import type { ProtocolRequestMap, ProtocolResultMap } from "./protocol-types.generated.js";
import type { BinaryBuffer, MaterializedBuffer, RequestOptions } from "./types.js";

export interface EngineShapeToken {
  readonly logicalId: number;
  readonly profileId: string;
  readonly placementId: string;
  readonly epoch: number;
}

export type KernelShapeToken = number | EngineShapeToken;

export type InputBufferDescriptor = {
  $inputBuffer: BinaryBuffer;
  byteOffset?: number;
  byteLength?: number;
};

type ProtocolInputBuffer = InputBufferDescriptor | {
  readonly bufferId: number;
  readonly byteOffset?: number;
  readonly byteLength?: number;
};

export type ProtocolRequestArgs<K extends OperationName> =
  ProtocolRequestMap<ShapeHandle, ScopeHandle, number, ProtocolInputBuffer>[K];

export type ProtocolResult<K extends OperationName> =
  ProtocolResultMap<KernelShapeToken, number, number, MaterializedBuffer>[K];

export interface ClientRequestOwner {
  readonly epoch: number;
  request<K extends OperationName>(
    op: K,
    args: ProtocolRequestArgs<K>,
    timeoutOrOptions?: number | RequestOptions,
  ): Promise<ProtocolResult<K>>;
  requestUnsafe<T>(
    op: OperationName,
    args: Record<string, unknown>,
    timeoutOrOptions?: number | RequestOptions,
  ): Promise<T>;
}

let nextEpoch = 1;
const kernelScopeProvenance = Symbol("kernelScopeProvenance");
const kernelShapeProvenance = Symbol("kernelShapeProvenance");

export function allocateClientEpoch(): number {
  return nextEpoch++;
}

export class ScopeHandle {
  readonly #owner: ClientRequestOwner;
  readonly #epoch: number;
  readonly #value: number;

  constructor(owner: ClientRequestOwner, value: number, provenance: typeof kernelScopeProvenance) {
    if (provenance !== kernelScopeProvenance) {
      throw new TypeError("ScopeHandle values can only be created from kernel results");
    }
    this.#owner = owner;
    this.#epoch = owner.epoch;
    this.#value = value;
  }

  encode(owner: ClientRequestOwner): number {
    if (owner !== this.#owner || owner.epoch !== this.#epoch) {
      throw new TypeError("ShapeScope belongs to an expired kernel instance");
    }
    return this.#value;
  }
}

export class ShapeHandle {
  readonly #owner: ClientRequestOwner;
  readonly #epoch: number;
  readonly #token: KernelShapeToken;

  constructor(owner: ClientRequestOwner, token: KernelShapeToken, provenance: typeof kernelShapeProvenance) {
    if (provenance !== kernelShapeProvenance) {
      throw new TypeError("ShapeHandle values can only be created from kernel results");
    }
    this.#owner = owner;
    this.#epoch = owner.epoch;
    this.#token = token;
  }

  encode(owner: ClientRequestOwner): KernelShapeToken {
    if (owner !== this.#owner || owner.epoch !== this.#epoch) {
      throw new TypeError("ShapeHandle belongs to a different or expired kernel instance");
    }
    return this.#token;
  }
}

export function shapeFromKernel(owner: ClientRequestOwner, token: KernelShapeToken): ShapeHandle {
  return new ShapeHandle(owner, token, kernelShapeProvenance);
}

export function scopeFromKernel(owner: ClientRequestOwner, value: number): ScopeHandle {
  return new ScopeHandle(owner, value, kernelScopeProvenance);
}

export type AnnotationWithPresentation = {
  presentation?: { shape: ShapeHandle; name?: string };
};

export type RawAnnotation<T extends AnnotationWithPresentation> =
  Omit<T, "presentation"> & { presentation?: { shape: KernelShapeToken; name?: string } };

export function materializeAnnotationPresentations<T extends AnnotationWithPresentation>(
  owner: ClientRequestOwner,
  items: readonly RawAnnotation<T>[],
): T[] {
  return items.map((item) => ({
    ...item,
    ...(item.presentation === undefined ? {} : {
      presentation: {
        ...item.presentation,
        shape: shapeFromKernel(owner, item.presentation.shape),
      },
    }),
  })) as T[];
}
