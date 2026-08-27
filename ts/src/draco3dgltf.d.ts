declare module "draco3dgltf" {
  export interface DracoArray {
    ptr: number;
    GetValue(index: number): number;
    size(): number;
  }

  export interface DracoFloat32Array extends DracoArray {}
  export interface DracoInt32Array extends DracoArray {}

  export interface DracoStatus {
    ok(): boolean;
    error_msg(): string;
  }

  export interface DracoBuffer {
    ptr: number;
    Init(data: Int8Array, byteLength: number): void;
  }

  export interface DracoMesh {
    ptr: number;
    num_points(): number;
    num_faces(): number;
  }

  export interface DracoAttribute {
    ptr: number;
  }

  export interface DracoDecoder {
    ptr: number;
    GetEncodedGeometryType(buffer: DracoBuffer): number;
    DecodeBufferToMesh(buffer: DracoBuffer, mesh: DracoMesh): DracoStatus;
    GetAttributeByUniqueId(mesh: DracoMesh, uniqueId: number): DracoAttribute;
    GetAttributeFloatForAllPoints(
      mesh: DracoMesh,
      attribute: DracoAttribute,
      values: DracoFloat32Array,
    ): boolean;
    GetFaceFromMesh(mesh: DracoMesh, faceIndex: number, face: DracoInt32Array): boolean;
  }

  export interface DracoModule {
    TRIANGULAR_MESH: number;
    DracoFloat32Array: new () => DracoFloat32Array;
    DracoInt32Array: new () => DracoInt32Array;
    DecoderBuffer: new () => DracoBuffer;
    Decoder: new () => DracoDecoder;
    Mesh: new () => DracoMesh;
    destroy(value: { ptr: number }): void;
  }

  interface DracoModuleFactory {
    createDecoderModule(options?: Record<string, unknown>): Promise<DracoModule>;
  }

  const factory: DracoModuleFactory;
  export default factory;
}
