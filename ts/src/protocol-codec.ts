/** Walk protocol values without treating binary buffers as ordinary objects. */
export function mapProtocolValue(
  value: unknown,
  transform: (value: unknown, key?: string) => unknown,
  isAtomic: (value: unknown) => boolean = (item) => (
    item instanceof ArrayBuffer
    || (typeof SharedArrayBuffer !== "undefined" && item instanceof SharedArrayBuffer)
    || ArrayBuffer.isView(item)
  ),
): unknown {
  const walk = (item: unknown, key?: string): unknown => {
    const mapped = transform(item, key);
    if (mapped !== item || isAtomic(mapped)) return mapped;
    if (Array.isArray(mapped)) return mapped.map((child) => walk(child));
    if (mapped !== null && typeof mapped === "object") {
      return Object.fromEntries(Object.entries(mapped).map(([name, child]) => [name, walk(child, name)]));
    }
    return mapped;
  };
  return walk(value);
}

export interface ProtocolBufferDescriptor {
  bufferId: number;
  byteLength: number;
  layout: string;
}

export function collectArrayBuffers(value: unknown): ArrayBuffer[] {
  const buffers: ArrayBuffer[] = [];
  const seen = new Set<ArrayBuffer>();
  mapProtocolValue(value, (item) => {
    const buffer = item instanceof ArrayBuffer
      ? item
      : ArrayBuffer.isView(item) && item.buffer instanceof ArrayBuffer
        ? item.buffer
        : undefined;
    if (buffer !== undefined && !seen.has(buffer)) {
      seen.add(buffer);
      buffers.push(buffer);
    }
    return item;
  });
  return buffers;
}

export function mapShapeHandles<T>(
  value: unknown,
  paths: readonly (readonly string[])[],
  visit: (handle: number) => T,
): unknown {
  const walk = (item: unknown, candidates: readonly (readonly string[])[]): unknown => {
    if (candidates.some((path) => path.length === 0)) {
      return typeof item === "number" ? visit(item) : item;
    }
    if (Array.isArray(item)) {
      const nested = candidates
        .filter((path) => path[0] === "*")
        .map((path) => path.slice(1));
      return nested.length === 0 ? item : item.map((entry) => walk(entry, nested));
    }
    if (item === null || typeof item !== "object") return item;
    const source = item as Record<string, unknown>;
    let result: Record<string, unknown> | undefined;
    for (const key of Object.keys(source)) {
      const nested = candidates
        .filter((path) => path[0] === key)
        .map((path) => path.slice(1));
      if (nested.length === 0) continue;
      result ??= { ...source };
      result[key] = walk(source[key], nested);
    }
    return result ?? item;
  };
  return walk(value, paths);
}

export function materializeProtocolBuffers<T>(
  value: unknown,
  shared: boolean,
  materialize: (descriptor: ProtocolBufferDescriptor, shared: boolean) => T,
  release?: (descriptor: ProtocolBufferDescriptor) => void,
): unknown {
  const pending = new Map<number, ProtocolBufferDescriptor>();
  mapProtocolValue(value, (item) => {
    if (
      item !== null
      && typeof item === "object"
      && "bufferId" in item
      && "byteLength" in item
      && "layout" in item
    ) {
      const descriptor = item as ProtocolBufferDescriptor;
      pending.set(descriptor.bufferId, descriptor);
    }
    return item;
  });
  try {
    return mapProtocolValue(value, (item) => {
      if (
        item !== null
        && typeof item === "object"
        && "bufferId" in item
        && "byteLength" in item
        && "layout" in item
      ) {
        const descriptor = item as ProtocolBufferDescriptor;
        const result = materialize(descriptor, shared);
        release?.(descriptor);
        pending.delete(descriptor.bufferId);
        return result;
      }
      return item;
    });
  } catch (error) {
    for (const descriptor of pending.values()) {
      try {
        release?.(descriptor);
      } catch {
        // Preserve the materialization error while still attempting cleanup.
      }
    }
    throw error;
  }
}
