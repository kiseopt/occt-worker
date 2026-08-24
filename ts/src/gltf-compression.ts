import { KernelError, type BinaryBuffer, type GLTFResolvedBuffer, type JSONObject } from "./types.js";

type MutableJSON = Record<string, any>;

export interface DecodedGLTF {
  data: ArrayBuffer;
  document: JSONObject;
  buffers: readonly GLTFResolvedBuffer[];
}

let dracoDecoderModule: Promise<any> | undefined;
let meshoptDecoderModule: Promise<any> | undefined;

async function loadDracoDecoder(): Promise<any> {
  const packageModule = await import("draco3dgltf");
  return packageModule.default.createDecoderModule({});
}

async function loadMeshoptDecoder(): Promise<any> {
  const packageModule = await import("meshoptimizer/decoder");
  return packageModule.MeshoptDecoder;
}

const failure = (message: string): never => {
  throw new KernelError({ code: "ImportExportFailed", message, details: { operation: "importGLTF", format: "gltf" } });
};

const uint32 = (view: DataView, offset: number): number => view.getUint32(offset, true);

function parseDocument(data: BinaryBuffer): { document: MutableJSON; glbBuffer?: Uint8Array } {
  const bytes = new Uint8Array(data);
  if (bytes.byteLength >= 4 && uint32(new DataView(data), 0) === 0x46546c67) {
    const view = new DataView(data);
    if (bytes.byteLength < 12 || uint32(view, 4) !== 2 || uint32(view, 8) !== bytes.byteLength) {
      return failure("GLB header is invalid");
    }
    let offset = 12;
    let jsonBytes: Uint8Array | undefined;
    let glbBuffer: Uint8Array | undefined;
    while (offset < bytes.byteLength) {
      if (bytes.byteLength - offset < 8) return failure("GLB chunk header is truncated");
      const length = uint32(view, offset);
      const type = uint32(view, offset + 4);
      offset += 8;
      if (length > bytes.byteLength - offset) return failure("GLB chunk is truncated");
      if (type === 0x4e4f534a && jsonBytes === undefined) jsonBytes = bytes.slice(offset, offset + length);
      else if (type === 0x004e4942 && glbBuffer === undefined) glbBuffer = bytes.slice(offset, offset + length);
      offset += length;
    }
    if (jsonBytes === undefined) return failure("glTF JSON document is missing");
    const text = new TextDecoder().decode(jsonBytes).replace(/[\0\x20\t\r\n]+$/u, "");
    try {
      return { document: JSON.parse(text) as MutableJSON, ...(glbBuffer === undefined ? {} : { glbBuffer }) };
    } catch {
      return failure("glTF JSON document is invalid");
    }
  }
  try {
    return { document: JSON.parse(new TextDecoder().decode(bytes)) as MutableJSON };
  } catch {
    return failure("glTF JSON document is invalid");
  }
}

function decodeDataUri(uri: string): Uint8Array {
  const comma = uri.indexOf(",");
  if (!uri.startsWith("data:") || comma < 0) return failure("glTF buffer URI is invalid");
  const metadata = uri.slice(5, comma);
  const payload = uri.slice(comma + 1);
  if (metadata.endsWith(";base64")) {
    try {
      const decoded = atob(payload);
      return Uint8Array.from(decoded, (value) => value.charCodeAt(0));
    } catch {
      return failure("glTF buffer has invalid base64 data");
    }
  }
  try {
    return new TextEncoder().encode(decodeURIComponent(payload));
  } catch {
    return failure("glTF buffer URI is invalid");
  }
}

function resolveBuffers(
  document: MutableJSON,
  glbBuffer: Uint8Array | undefined,
  resources: Readonly<Record<string, BinaryBuffer>> | undefined,
): { bytes: Uint8Array[]; publicBuffers: GLTFResolvedBuffer[] } {
  if (!Array.isArray(document.buffers)) return { bytes: [], publicBuffers: [] };
  return document.buffers.reduce<{ bytes: Uint8Array[]; publicBuffers: GLTFResolvedBuffer[] }>(
    (result, definition: MutableJSON, index: number) => {
      let value: Uint8Array;
      const uri = definition?.uri;
      if (typeof uri === "string" && uri.startsWith("data:")) value = decodeDataUri(uri);
      else if (typeof uri === "string") {
        const resource = resources?.[uri];
        if (resource === undefined) return failure(`glTF external resource is missing: ${uri}`);
        value = new Uint8Array(resource.slice(0));
      } else if (index === 0 && glbBuffer !== undefined) value = glbBuffer;
      else return failure("glTF buffer has no embedded data");
      if (!Number.isSafeInteger(definition.byteLength) || definition.byteLength < 0
          || value.byteLength < definition.byteLength) return failure("glTF buffer is truncated");
      const exact = value.slice(0, definition.byteLength);
      result.bytes.push(exact);
      result.publicBuffers.push({
        ...(typeof uri === "string" ? { uri } : {}),
        data: exact.buffer,
      });
      return result;
    },
    { bytes: [], publicBuffers: [] },
  );
}

function viewBytes(document: MutableJSON, buffers: readonly Uint8Array[], viewIndex: number): Uint8Array {
  const definition = document.bufferViews?.[viewIndex];
  if (definition === undefined || !Number.isSafeInteger(definition.buffer)
      || !Number.isSafeInteger(definition.byteLength)) return failure("glTF buffer view is invalid");
  const buffer = buffers[definition.buffer];
  if (buffer === undefined) return failure("glTF buffer index is out of range");
  const offset = definition.byteOffset ?? 0;
  if (!Number.isSafeInteger(offset) || offset < 0 || definition.byteLength < 0
      || offset > buffer.byteLength || definition.byteLength > buffer.byteLength - offset) {
    return failure("glTF buffer view is truncated");
  }
  return buffer.subarray(offset, offset + definition.byteLength);
}

function encodeBase64(value: Uint8Array): string {
  let result = "";
  const chunkSize = 0x8000;
  for (let offset = 0; offset < value.byteLength; offset += chunkSize) {
    result += String.fromCharCode(...value.subarray(offset, offset + chunkSize));
  }
  return btoa(result);
}

function appendBuffer(document: MutableJSON, buffers: Uint8Array[], value: Uint8Array): number {
  const index = buffers.length;
  buffers.push(value);
  document.buffers.push({
    byteLength: value.byteLength,
    uri: `data:application/octet-stream;base64,${encodeBase64(value)}`,
  });
  return index;
}

function removeExtensionName(document: MutableJSON, name: string): void {
  for (const key of ["extensionsUsed", "extensionsRequired"]) {
    if (!Array.isArray(document[key])) continue;
    document[key] = document[key].filter((value: unknown) => value !== name);
    if (document[key].length === 0) delete document[key];
  }
}

async function decodeMeshopt(document: MutableJSON, buffers: Uint8Array[]): Promise<boolean> {
  if (!Array.isArray(document.bufferViews)) return false;
  const compressed = document.bufferViews.filter(
    (view: MutableJSON) => view?.extensions?.EXT_meshopt_compression !== undefined,
  );
  if (compressed.length === 0) return false;
  meshoptDecoderModule ??= loadMeshoptDecoder();
  let meshopt: any;
  try {
    meshopt = await meshoptDecoderModule;
    await meshopt.ready;
  } catch {
    return failure("glTF meshopt decoder runtime is unavailable");
  }
  for (const view of compressed) {
    const extension = view.extensions.EXT_meshopt_compression as MutableJSON;
    const sourceBuffer = buffers[extension.buffer];
    const offset = extension.byteOffset ?? 0;
    const length = extension.byteLength;
    const count = extension.count;
    const stride = extension.byteStride;
    if (sourceBuffer === undefined || !Number.isSafeInteger(offset) || offset < 0
        || !Number.isSafeInteger(length) || length < 0 || offset > sourceBuffer.byteLength
        || length > sourceBuffer.byteLength - offset || !Number.isSafeInteger(count) || count < 0
        || !Number.isSafeInteger(stride) || stride <= 0 || typeof extension.mode !== "string") {
      return failure("glTF meshopt buffer view is invalid");
    }
    let decoded: Uint8Array;
    try {
      decoded = await meshopt.decodeGltfBufferAsync(
        count, stride, sourceBuffer.subarray(offset, offset + length), extension.mode, extension.filter,
      );
    } catch {
      return failure("glTF meshopt buffer could not be decoded");
    }
    view.buffer = appendBuffer(document, buffers, decoded);
    view.byteOffset = 0;
    view.byteLength = decoded.byteLength;
    if (extension.mode === "ATTRIBUTES") view.byteStride = stride;
    else delete view.byteStride;
    delete view.extensions.EXT_meshopt_compression;
    if (Object.keys(view.extensions).length === 0) delete view.extensions;
  }
  removeExtensionName(document, "EXT_meshopt_compression");
  return true;
}

function accessorComponents(type: unknown): number {
  if (type === "SCALAR") return 1;
  if (type === "VEC2") return 2;
  if (type === "VEC3") return 3;
  if (type === "VEC4") return 4;
  return failure("glTF Draco attribute accessor type is unsupported");
}

function dracoFloatAttribute(module: any, decoder: any, mesh: any, attribute: any, count: number): Float32Array {
  const values = new module.DracoFloat32Array();
  try {
    if (!decoder.GetAttributeFloatForAllPoints(mesh, attribute, values) || values.size() !== count) {
      return failure("glTF Draco attribute could not be decoded");
    }
    const result = new Float32Array(count);
    for (let index = 0; index < count; ++index) result[index] = values.GetValue(index);
    return result;
  } finally {
    module.destroy(values);
  }
}

async function decodeDraco(document: MutableJSON, buffers: Uint8Array[]): Promise<boolean> {
  if (!Array.isArray(document.meshes)) return false;
  const primitives = document.meshes.flatMap((mesh: MutableJSON) => Array.isArray(mesh?.primitives)
    ? mesh.primitives.filter((primitive: MutableJSON) => primitive?.extensions?.KHR_draco_mesh_compression !== undefined)
    : []);
  if (primitives.length === 0) return false;
  dracoDecoderModule ??= loadDracoDecoder();
  let module: any;
  try {
    module = await dracoDecoderModule;
  } catch {
    return failure("glTF Draco decoder runtime is unavailable");
  }
  for (const primitive of primitives) {
    const extension = primitive.extensions.KHR_draco_mesh_compression as MutableJSON;
    if (!Number.isSafeInteger(extension.bufferView) || extension.bufferView < 0
        || extension.attributes === null || typeof extension.attributes !== "object") {
      return failure("glTF Draco primitive is invalid");
    }
    const encoded = viewBytes(document, buffers, extension.bufferView);
    const decoderBuffer = new module.DecoderBuffer();
    const decoder = new module.Decoder();
    const mesh = new module.Mesh();
    try {
      decoderBuffer.Init(new Int8Array(encoded.buffer, encoded.byteOffset, encoded.byteLength), encoded.byteLength);
      if (decoder.GetEncodedGeometryType(decoderBuffer) !== module.TRIANGULAR_MESH) {
        return failure("glTF Draco primitive is not a triangle mesh");
      }
      const status = decoder.DecodeBufferToMesh(decoderBuffer, mesh);
      if (!status.ok() || mesh.ptr === 0) return failure(`glTF Draco primitive could not be decoded: ${status.error_msg()}`);
      const pointCount = mesh.num_points();
      const faceCount = mesh.num_faces();
      for (const [semantic, uniqueId] of Object.entries(extension.attributes)) {
        const oldAccessorIndex = primitive.attributes?.[semantic];
        const oldAccessor = document.accessors?.[oldAccessorIndex];
        if (!Number.isSafeInteger(uniqueId) || oldAccessor === undefined) {
          return failure("glTF Draco attribute definition is invalid");
        }
        const attribute = decoder.GetAttributeByUniqueId(mesh, uniqueId);
        if (attribute.ptr === 0) return failure(`glTF Draco attribute is missing: ${semantic}`);
        const components = accessorComponents(oldAccessor.type);
        const floats = dracoFloatAttribute(module, decoder, mesh, attribute, pointCount * components);
        let bytes: Uint8Array;
        let componentType = 5126;
        if (semantic.startsWith("JOINTS_")) {
          const joints = new Uint16Array(floats.length);
          for (let index = 0; index < floats.length; ++index) {
            const value = floats[index]!;
            if (!Number.isInteger(value) || value < 0 || value > 65535) {
              return failure("glTF Draco joint attribute is invalid");
            }
            joints[index] = value;
          }
          bytes = new Uint8Array(joints.buffer);
          componentType = 5123;
        } else bytes = new Uint8Array(floats.buffer);
        const buffer = appendBuffer(document, buffers, bytes);
        const bufferView = document.bufferViews.push({ buffer, byteOffset: 0, byteLength: bytes.byteLength }) - 1;
        const accessor = {
          ...oldAccessor,
          bufferView,
          byteOffset: 0,
          componentType,
          count: pointCount,
        };
        delete accessor.sparse;
        delete accessor.normalized;
        primitive.attributes[semantic] = document.accessors.push(accessor) - 1;
      }
      const indices = new Uint32Array(faceCount * 3);
      const face = new module.DracoInt32Array();
      try {
        for (let faceIndex = 0; faceIndex < faceCount; ++faceIndex) {
          if (!decoder.GetFaceFromMesh(mesh, faceIndex, face)) return failure("glTF Draco indices could not be decoded");
          indices[faceIndex * 3] = face.GetValue(0);
          indices[faceIndex * 3 + 1] = face.GetValue(1);
          indices[faceIndex * 3 + 2] = face.GetValue(2);
        }
      } finally {
        module.destroy(face);
      }
      const indexBytes = new Uint8Array(indices.buffer);
      const indexBuffer = appendBuffer(document, buffers, indexBytes);
      const indexView = document.bufferViews.push({ buffer: indexBuffer, byteOffset: 0, byteLength: indexBytes.byteLength }) - 1;
      const oldIndexAccessor = document.accessors?.[primitive.indices] ?? { type: "SCALAR" };
      primitive.indices = document.accessors.push({
        ...oldIndexAccessor,
        bufferView: indexView,
        byteOffset: 0,
        componentType: 5125,
        count: indices.length,
        type: "SCALAR",
      }) - 1;
      delete primitive.extensions.KHR_draco_mesh_compression;
      if (Object.keys(primitive.extensions).length === 0) delete primitive.extensions;
    } finally {
      module.destroy(mesh);
      module.destroy(decoder);
      module.destroy(decoderBuffer);
    }
  }
  removeExtensionName(document, "KHR_draco_mesh_compression");
  return true;
}

export async function decodeGLTFCompression(
  data: BinaryBuffer,
  resources?: Readonly<Record<string, BinaryBuffer>>,
): Promise<DecodedGLTF | undefined> {
  const parsed = parseDocument(data);
  const sourceDocument = structuredClone(parsed.document) as JSONObject;
  const resolved = resolveBuffers(parsed.document, parsed.glbBuffer, resources);
  const sourceBuffers = resolved.publicBuffers;
  const decodedMeshopt = await decodeMeshopt(parsed.document, resolved.bytes);
  const decodedDraco = await decodeDraco(parsed.document, resolved.bytes);
  if (!decodedMeshopt && !decodedDraco) return undefined;
  return {
    data: new TextEncoder().encode(JSON.stringify(parsed.document)).buffer,
    document: sourceDocument,
    buffers: sourceBuffers,
  };
}
