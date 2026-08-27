import type { BinaryBuffer, LengthUnit, Vec3 } from "./types-base.js";

export interface PLYMeshData {
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
  uvs?: Float32Array;
  colors?: Uint8Array;
  document?: PLYDocument;
}

export interface PLYDocument {
  comments: string[];
  objectInfo: string[];
}

export interface PLYMeshImport extends PLYMeshData {
  shape: import("./client.js").ShapeHandle;
  document: PLYDocument;
}

export interface STLMeshData {
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
}


export interface TessellatedShapeExportOptions {
  linearDeflection?: number;
  angularDeflection?: number;
  relative?: boolean;
}

export interface STLExportOptions extends TessellatedShapeExportOptions {
  encoding?: "binary" | "ascii";
  solidName?: string;
  binaryHeader?: Uint8Array;
}

export interface STLMeshExportOptions {
  encoding?: "binary" | "ascii";
  solidName?: string;
  binaryHeader?: Uint8Array;
}

export interface STLImportOptions {
  includeMesh?: boolean;
}

export interface IGESExportOptions {
  unit?: LengthUnit;
  mode?: "faces" | "brep";
}

export interface IGESImportOptions {
  unit?: LengthUnit;
}

export interface STEPExportOptions {
  unit?: LengthUnit;
  timestamp?: string;
  schema?: "AP203" | "AP214" | "AP242";
}

export interface STEPImportOptions {
  unit?: LengthUnit;
}

export interface GLTFExportOptions extends TessellatedShapeExportOptions {
  format?: "glb" | "gltf";
}

export interface GLTFDocumentExportOptions {
  format?: "glb" | "gltf";
}

export interface OBJExportOptions extends TessellatedShapeExportOptions {}

export interface OBJMeshExportOptions {
  materialLibrary?: string;
}

export interface OBJImportOptions {
  includeDocument?: boolean;
  resources?: Readonly<Record<string, BinaryBuffer>>;
}

export interface PLYExportOptions extends TessellatedShapeExportOptions {
  encoding?: "ascii" | "binary_little_endian" | "binary_big_endian";
}

export interface PLYMeshExportOptions {
  encoding?: "ascii" | "binary_little_endian" | "binary_big_endian";
}

export interface PLYImportOptions {
  includeMesh?: boolean;
}

export interface VRMLExportOptions extends TessellatedShapeExportOptions {
  version?: 1 | 2;
}

export interface VRMLMeshData {
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
  uvs?: Float32Array;
  /** Per-vertex u8 RGBA. VRML stores RGB only, so alpha values must be 255. */
  colors?: Uint8Array;
}

export interface VRMLImportOptions {
  includeMesh?: boolean;
}

export interface STLMeshImport extends STLMeshData {
  shape: import("./client.js").ShapeHandle;
  normals: Float32Array;
  solidName?: string;
  binaryHeader?: Uint8Array;
}

export interface VRMLMeshImport {
  shape: import("./client.js").ShapeHandle;
  positions: Float32Array;
  indices: Uint32Array;
  normals: Float32Array;
  uvs?: Float32Array;
  /** Per-vertex u8 RGBA with opaque alpha reconstructed from VRML RGB colors. */
  colors?: Uint8Array;
}

export interface OBJPrimitive {
  indexStart: number;
  indexCount: number;
  object?: string;
  groups: readonly string[];
  material?: string;
  smoothingGroup?: string;
}

export interface OBJMaterial {
  name: string;
  ambient?: Vec3;
  diffuse?: Vec3;
  specular?: Vec3;
  emissive?: Vec3;
  opacity?: number;
  shininess?: number;
  opticalDensity?: number;
  illuminationModel?: number;
  diffuseMap?: string;
  specularMap?: string;
  opacityMap?: string;
  bumpMap?: string;
}

export interface OBJDocument {
  primitives: readonly OBJPrimitive[];
  materials: readonly OBJMaterial[];
  materialLibraries: readonly string[];
}

export interface OBJMeshImport {
  shape: import("./client.js").ShapeHandle;
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
  uvs?: Float32Array;
  document: OBJDocument;
}

export interface OBJMeshData {
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
  uvs?: Float32Array;
  document?: OBJDocument;
}

export interface OBJMeshExport {
  data: ArrayBuffer;
  resources: Readonly<Record<string, ArrayBuffer>>;
}

export type JSONValue = null | boolean | number | string | readonly JSONValue[] | JSONObject;
export interface JSONObject { readonly [key: string]: JSONValue }

export interface GLTFPrimitiveInstance {
  nodeIndex?: number;
  meshIndex: number;
  primitiveIndex: number;
  mode: number;
  material?: number;
  vertexStart: number;
  vertexCount: number;
  indexStart: number;
  indexCount: number;
  hasNormals: boolean;
  hasUVs: boolean;
}

export interface GLTFResolvedBuffer {
  uri?: string;
  data: BinaryBuffer;
}

export interface GLTFResolvedResource {
  uri: string;
  data: BinaryBuffer;
}

export type GLTFImportOptions = {
  includeDocument?: boolean;
  resources?: Readonly<Record<string, BinaryBuffer>>;
  morphWeights?: readonly number[];
} & (
  | { animationIndex?: never; animationTime?: never }
  | { animationIndex: number; animationTime: number }
);

export interface GLTFDocumentImport {
  shape?: import("./client.js").ShapeHandle;
  positions: Float32Array;
  indices: Uint32Array;
  normals?: Float32Array;
  uvs?: Float32Array;
  document: JSONObject;
  activeScene?: number;
  sceneRoots: readonly number[];
  primitives: readonly GLTFPrimitiveInstance[];
  buffers: readonly GLTFResolvedBuffer[];
  resources: readonly GLTFResolvedResource[];
}

export interface GLTFDocumentData {
  document: JSONObject;
  buffers: readonly GLTFResolvedBuffer[];
  resources?: readonly GLTFResolvedResource[];
}

export interface GLTFDocumentExport {
  data: ArrayBuffer;
  resources: Readonly<Record<string, ArrayBuffer>>;
}

