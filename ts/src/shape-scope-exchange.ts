import { decodeGLTFCompression } from "./gltf-compression.js";
import { materializeAnnotationPresentations, shapeFromKernel,
  type KernelShapeToken, type ProtocolRequestArgs, type RawAnnotation, type ShapeHandle } from "./client-contract.js";
import type {
  BinaryBuffer,
  GLTFDocumentImport,
  GLTFImportOptions,
  IGESImportOptions,
  MaterializedBuffer,
  MultiRootShapeImport,
  OBJImportOptions,
  OBJMeshImport,
  PLYImportOptions,
  PLYMeshImport,
  STEPDocument,
  STEPImportOptions,
  STLImportOptions,
  STLMeshImport,
  VRMLImportOptions,
  VRMLMeshImport,
  XCAFDocumentFormat,
} from "./types.js";
import { OperationsShapeScope } from "./shape-scope-operations.js";

export class ExchangeShapeScope extends OperationsShapeScope implements AsyncDisposable {
  protected endPromise: Promise<void> | undefined;

  async importBREP(data: BinaryBuffer): Promise<ShapeHandle> {
    const result = await this.client.request("importBREP", {
      scopeId: this.scope,
      data: { $inputBuffer: data },
    });
    return shapeFromKernel(this.client, result.shape);
  }

  async importIGES(
    data: BinaryBuffer,
    options: IGESImportOptions = {},
  ): Promise<MultiRootShapeImport> {
    const result = await this.client.request("importIGES", {
      scopeId: this.scope,
      data: { $inputBuffer: data },
      ...options,
    });
    return {
      shape: shapeFromKernel(this.client, result.shape),
      shapes: result.shapes.map((shape) => shapeFromKernel(this.client, shape)),
      rootCount: result.rootCount,
    };
  }

  async importIGESDocument(
    data: BinaryBuffer,
    options: IGESImportOptions = {},
  ): Promise<STEPDocument> {
    const result = await this.client.request("importIGESDocument", {
      scopeId: this.scope,
      data: { $inputBuffer: data },
      ...options,
    });
    return {
      shape: shapeFromKernel(this.client, result.shape),
      rootCount: result.rootCount,
      roots: result.roots,
      gdt: materializeAnnotationPresentations(this.client, result.gdt),
      datums: materializeAnnotationPresentations(this.client, result.datums),
      geometricTolerances: materializeAnnotationPresentations(this.client, result.geometricTolerances),
      views: result.views,
      shuo: result.shuo,
      nodes: result.nodes.map((node) => ({
        ...node,
        shape: shapeFromKernel(this.client, node.shape),
      })),
    };
  }

  async importGLTF(data: BinaryBuffer): Promise<ShapeHandle>;
  async importGLTF(
    data: BinaryBuffer,
    options: GLTFImportOptions & { includeDocument?: false },
  ): Promise<ShapeHandle>;
  async importGLTF(
    data: BinaryBuffer,
    options: GLTFImportOptions & { includeDocument: true },
  ): Promise<GLTFDocumentImport>;
  async importGLTF(
    data: BinaryBuffer,
    options: GLTFImportOptions = {},
  ): Promise<ShapeHandle | GLTFDocumentImport> {
    const decoded = await decodeGLTFCompression(data, options.resources);
    const importData = decoded?.data ?? data;
    const resources = options.resources === undefined
      ? undefined
      : Object.entries(options.resources).map(([uri, value]) => ({ uri, data: { $inputBuffer: value } }));
    const requestArgs: ProtocolRequestArgs<"importGLTF"> = {
      scopeId: this.scope,
      data: { $inputBuffer: importData },
      ...(options.includeDocument === undefined ? {} : { includeDocument: options.includeDocument }),
      ...(resources === undefined ? {} : { resources }),
      ...(options.morphWeights === undefined ? {} : { morphWeights: options.morphWeights }),
      ...(options.animationIndex === undefined ? {} : { animationIndex: options.animationIndex }),
      ...(options.animationTime === undefined ? {} : { animationTime: options.animationTime }),
    };
    const result = await this.client.request("importGLTF", requestArgs);
    if (!options.includeDocument) {
      if (!("shape" in result) || result.shape === undefined) {
        throw new Error("glTF shape import did not return a shape");
      }
      return shapeFromKernel(this.client, result.shape);
    }
    if (!("positions" in result)) {
      throw new Error("glTF document import did not return required data");
    }
    return {
      ...(result.shape === undefined ? {} : { shape: shapeFromKernel(this.client, result.shape) }),
      positions: new Float32Array(result.positions.data),
      indices: new Uint32Array(result.indices.data),
      ...(result.normals === undefined ? {} : { normals: new Float32Array(result.normals.data) }),
      ...(result.uvs === undefined ? {} : { uvs: new Float32Array(result.uvs.data) }),
      document: decoded?.document ?? result.document,
      ...(result.activeScene === undefined ? {} : { activeScene: result.activeScene }),
      sceneRoots: result.sceneRoots,
      primitives: result.primitives,
      buffers: decoded?.buffers ?? result.buffers.map((buffer) => ({
        ...(buffer.uri === undefined ? {} : { uri: buffer.uri }),
        data: buffer.data.data,
      })),
      resources: result.resources.map((resource) => ({ uri: resource.uri, data: resource.data.data })),
    };
  }

  async importOBJ(data: BinaryBuffer): Promise<ShapeHandle>;
  async importOBJ(
    data: BinaryBuffer,
    options: OBJImportOptions & { includeDocument: true },
  ): Promise<OBJMeshImport>;
  async importOBJ(
    data: BinaryBuffer,
    options: OBJImportOptions = {},
  ): Promise<ShapeHandle | OBJMeshImport> {
    const resources = options.resources === undefined
      ? undefined
      : Object.entries(options.resources).map(([uri, value]) => ({ uri, data: { $inputBuffer: value } }));
    const requestArgs: ProtocolRequestArgs<"importOBJ"> = {
      scopeId: this.scope,
      data: { $inputBuffer: data },
      ...(options.includeDocument === undefined ? {} : { includeDocument: options.includeDocument }),
      ...(resources === undefined ? {} : { resources }),
    };
    const result = await this.client.request("importOBJ", requestArgs);
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeDocument) return resultShape;
    if (!("positions" in result)) {
      throw new Error("OBJ document import did not return required mesh data");
    }
    return {
      shape: resultShape,
      positions: new Float32Array(result.positions.data),
      indices: new Uint32Array(result.indices.data),
      ...(result.normals === undefined ? {} : { normals: new Float32Array(result.normals.data) }),
      ...(result.uvs === undefined ? {} : { uvs: new Float32Array(result.uvs.data) }),
      document: result.document,
    };
  }

  async importPLY(data: BinaryBuffer): Promise<ShapeHandle>;
  async importPLY(data: BinaryBuffer, options: PLYImportOptions & { includeMesh: true }): Promise<PLYMeshImport>;
  async importPLY(data: BinaryBuffer, options: PLYImportOptions = {}): Promise<ShapeHandle | PLYMeshImport> {
    const result = await this.client.request("importPLY", {
      scopeId: this.scope,
      data: { $inputBuffer: data },
      ...options,
    });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeMesh) return resultShape;
    if (!("positions" in result))
      throw new Error("PLY mesh import did not return required mesh buffers");
    return {
      shape: resultShape,
      positions: new Float32Array(result.positions.data),
      indices: new Uint32Array(result.indices.data),
      ...(result.normals === undefined ? {} : { normals: new Float32Array(result.normals.data) }),
      ...(result.uvs === undefined ? {} : { uvs: new Float32Array(result.uvs.data) }),
      ...(result.colors === undefined ? {} : { colors: new Uint8Array(result.colors.data) }),
      document: result.document,
    };
  }

  async importSTEP(
    data: BinaryBuffer,
    options: STEPImportOptions = {},
  ): Promise<MultiRootShapeImport> {
    const result = await this.client.request("importSTEP", {
      scopeId: this.scope,
      data: { $inputBuffer: data },
      ...options,
    });
    return {
      shape: shapeFromKernel(this.client, result.shape),
      shapes: result.shapes.map((shape) => shapeFromKernel(this.client, shape)),
      rootCount: result.rootCount,
    };
  }

  async importSTEPDocument(
    data: BinaryBuffer,
    options: STEPImportOptions = {},
  ): Promise<STEPDocument> {
    const result = await this.client.request("importSTEPDocument", {
      scopeId: this.scope,
      data: { $inputBuffer: data },
      ...options,
    });
    return {
      shape: shapeFromKernel(this.client, result.shape),
      rootCount: result.rootCount,
      roots: result.roots,
      gdt: materializeAnnotationPresentations(this.client, result.gdt),
      datums: materializeAnnotationPresentations(this.client, result.datums),
      geometricTolerances: materializeAnnotationPresentations(this.client, result.geometricTolerances),
      views: result.views,
      shuo: result.shuo,
      nodes: result.nodes.map((node) => ({
        ...node,
        shape: shapeFromKernel(this.client, node.shape),
      })),
    };
  }

  async importXCAF(
    data: BinaryBuffer,
    format: XCAFDocumentFormat = "bin",
  ): Promise<STEPDocument> {
    const result = await this.client.request("importXCAF", {
      scopeId: this.scope,
      data: { $inputBuffer: data },
      format,
    });
    return {
      shape: shapeFromKernel(this.client, result.shape),
      rootCount: result.rootCount,
      roots: result.roots,
      gdt: materializeAnnotationPresentations(this.client, result.gdt),
      datums: materializeAnnotationPresentations(this.client, result.datums),
      geometricTolerances: materializeAnnotationPresentations(this.client, result.geometricTolerances),
      views: result.views,
      shuo: result.shuo,
      nodes: result.nodes.map((node) => ({
        ...node,
        shape: shapeFromKernel(this.client, node.shape),
      })),
    };
  }

  async importSTL(data: BinaryBuffer): Promise<ShapeHandle>;
  async importSTL(data: BinaryBuffer, options: STLImportOptions & { includeMesh: true }): Promise<STLMeshImport>;
  async importSTL(
    data: BinaryBuffer,
    options: STLImportOptions = {},
  ): Promise<ShapeHandle | STLMeshImport> {
    const result = await this.client.request("importSTL", {
      scopeId: this.scope,
      data: { $inputBuffer: data },
      ...options,
    });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeMesh) return resultShape;
    if (!("positions" in result))
      throw new Error("STL mesh import did not return required mesh buffers");
    return {
      shape: resultShape,
      positions: new Float32Array(result.positions.data),
      indices: new Uint32Array(result.indices.data),
      normals: new Float32Array(result.normals.data),
      ...(result.solidName === undefined ? {} : { solidName: result.solidName }),
      ...(result.binaryHeader === undefined
        ? {}
        : { binaryHeader: new Uint8Array(result.binaryHeader.data) }),
    };
  }

  async importVRML(data: BinaryBuffer): Promise<ShapeHandle>;
  async importVRML(data: BinaryBuffer, options: VRMLImportOptions & { includeMesh: true }): Promise<VRMLMeshImport>;
  async importVRML(
    data: BinaryBuffer,
    options: VRMLImportOptions = {},
  ): Promise<ShapeHandle | VRMLMeshImport> {
    const result = await this.client.request("importVRML", {
      scopeId: this.scope,
      data: { $inputBuffer: data },
      ...options,
    });
    const resultShape = shapeFromKernel(this.client, result.shape);
    if (!options.includeMesh) return resultShape;
    if (!("positions" in result))
      throw new Error("VRML mesh import did not return required mesh buffers");
    const mesh: VRMLMeshImport = {
      shape: resultShape,
      positions: new Float32Array(result.positions.data),
      indices: new Uint32Array(result.indices.data),
      normals: new Float32Array(result.normals.data),
    };
    if (result.uvs !== undefined) mesh.uvs = new Float32Array(result.uvs.data);
    if (result.colors !== undefined) mesh.colors = new Uint8Array(result.colors.data);
    return mesh;
  }


  async end(): Promise<void> {
    if (this.endPromise === undefined) {
      this.endPromise = this.client.request("endScope", { scopeId: this.scope }).then(() => undefined);
      void this.endPromise.catch(() => {
        this.endPromise = undefined;
      });
    }
    await this.endPromise;
  }

  async [Symbol.asyncDispose](): Promise<void> {
    await this.end();
  }
}
