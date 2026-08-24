import type { EdgeTessellation, Tessellation } from "./types.js";

export interface MeshWeldOptions {
  tolerance?: number;
  respectNormals?: boolean;
  normalTolerance?: number;
  respectUVs?: boolean;
  uvTolerance?: number;
}

export interface MeshSimplifyOptions {
  targetTriangles?: number;
  ratio?: number;
}

export interface MeshSubdivisionOptions {
  levels?: number;
}

export interface MeshBoundaryLoop {
  vertexIndices: readonly number[];
  perimeter: number;
}

export interface PlanarHoleFillOptions {
  /** Zero-based indices into the deterministic boundary-loop list. Defaults to every loop. */
  loopIndices?: readonly number[];
  planarityTolerance?: number;
  /** Face index assigned to the appended fill triangles. Defaults to one past the current maximum. */
  faceIndex?: number;
}

export interface PlanarHoleFillResult {
  mesh: Tessellation;
  boundaryLoops: readonly MeshBoundaryLoop[];
  filledLoopIndices: readonly number[];
  addedTriangleCount: number;
}

export interface FaceTriangleRange {
  faceIndex: number;
  triangleStart: number;
  triangleCount: number;
}

export interface EdgeVertexRange {
  edgeIndex: number;
  vertexStart: number;
  vertexCount: number;
}

export class FaceSelectionMap {
  readonly ranges: readonly FaceTriangleRange[];
  readonly faceIndices: readonly number[];
  readonly #triangleFaces: Uint32Array;
  readonly #assigned: Uint8Array;

  constructor(mesh: Tessellation) {
    validateMesh(mesh);
    const triangleCount = mesh.indices.length / 3;
    this.#triangleFaces = new Uint32Array(triangleCount);
    this.#assigned = new Uint8Array(triangleCount);
    const ranges: FaceTriangleRange[] = [];
    const faceIndices: number[] = [];
    const seenFaces = new Set<number>();
    for (let group = 0; group < mesh.faceGroups.length; group += 3) {
      const faceIndex = mesh.faceGroups[group]!;
      const indexStart = mesh.faceGroups[group + 1]!;
      const indexCount = mesh.faceGroups[group + 2]!;
      if (indexStart % 3 !== 0 || indexCount % 3 !== 0 || indexStart + indexCount > mesh.indices.length) {
        throw new RangeError("Mesh faceGroups contain an invalid index range");
      }
      const triangleStart = indexStart / 3;
      const groupTriangleCount = indexCount / 3;
      for (let triangle = triangleStart; triangle < triangleStart + groupTriangleCount; triangle++) {
        if (this.#assigned[triangle] !== 0) throw new RangeError("Mesh faceGroups contain overlapping ranges");
        this.#assigned[triangle] = 1;
        this.#triangleFaces[triangle] = faceIndex;
      }
      ranges.push(Object.freeze({ faceIndex, triangleStart, triangleCount: groupTriangleCount }));
      if (!seenFaces.has(faceIndex)) {
        seenFaces.add(faceIndex);
        faceIndices.push(faceIndex);
      }
    }
    this.ranges = Object.freeze(ranges);
    this.faceIndices = Object.freeze(faceIndices);
  }

  faceAtTriangle(triangleIndex: number): number | undefined {
    if (!Number.isInteger(triangleIndex) || triangleIndex < 0 || triangleIndex >= this.#assigned.length) {
      throw new RangeError("Triangle index is outside the mesh");
    }
    return this.#assigned[triangleIndex] === 0 ? undefined : this.#triangleFaces[triangleIndex];
  }

  rangesForFace(faceIndex: number): readonly FaceTriangleRange[] {
    if (!Number.isInteger(faceIndex) || faceIndex < 0) throw new RangeError("Face index must be a non-negative integer");
    return this.ranges.filter((range) => range.faceIndex === faceIndex);
  }
}

export class EdgeSelectionMap {
  readonly ranges: readonly EdgeVertexRange[];
  readonly edgeIndices: readonly number[];
  readonly #vertexEdges: Uint32Array;
  readonly #assigned: Uint8Array;

  constructor(mesh: EdgeTessellation) {
    if (mesh.positions.length % 3 !== 0) throw new TypeError("Edge mesh positions must contain xyz values");
    if (mesh.edgeGroups.length % 3 !== 0) throw new TypeError("Mesh edgeGroups must contain u32 triples");
    const vertexCount = mesh.positions.length / 3;
    this.#vertexEdges = new Uint32Array(vertexCount);
    this.#assigned = new Uint8Array(vertexCount);
    const ranges: EdgeVertexRange[] = [];
    const edgeIndices: number[] = [];
    const seenEdges = new Set<number>();
    for (let group = 0; group < mesh.edgeGroups.length; group += 3) {
      const edgeIndex = mesh.edgeGroups[group]!;
      const vertexStart = mesh.edgeGroups[group + 1]!;
      const groupVertexCount = mesh.edgeGroups[group + 2]!;
      if (vertexStart + groupVertexCount > vertexCount) {
        throw new RangeError("Mesh edgeGroups contain an invalid vertex range");
      }
      for (let vertex = vertexStart; vertex < vertexStart + groupVertexCount; vertex++) {
        if (this.#assigned[vertex] !== 0) throw new RangeError("Mesh edgeGroups contain overlapping ranges");
        this.#assigned[vertex] = 1;
        this.#vertexEdges[vertex] = edgeIndex;
      }
      ranges.push(Object.freeze({ edgeIndex, vertexStart, vertexCount: groupVertexCount }));
      if (!seenEdges.has(edgeIndex)) {
        seenEdges.add(edgeIndex);
        edgeIndices.push(edgeIndex);
      }
    }
    this.ranges = Object.freeze(ranges);
    this.edgeIndices = Object.freeze(edgeIndices);
  }

  edgeAtVertex(vertexIndex: number): number | undefined {
    if (!Number.isInteger(vertexIndex) || vertexIndex < 0 || vertexIndex >= this.#assigned.length) {
      throw new RangeError("Vertex index is outside the edge mesh");
    }
    return this.#assigned[vertexIndex] === 0 ? undefined : this.#vertexEdges[vertexIndex];
  }

  rangesForEdge(edgeIndex: number): readonly EdgeVertexRange[] {
    if (!Number.isInteger(edgeIndex) || edgeIndex < 0) throw new RangeError("Edge index must be a non-negative integer");
    return this.ranges.filter((range) => range.edgeIndex === edgeIndex);
  }
}

function validateMesh(mesh: Tessellation): number {
  if (mesh.positions.length % 3 !== 0 || mesh.normals.length !== mesh.positions.length) {
    throw new TypeError("Mesh positions and normals must contain matching xyz values");
  }
  const vertexCount = mesh.positions.length / 3;
  if (mesh.indices.length % 3 !== 0) throw new TypeError("Mesh indices must contain triangles");
  if (mesh.uvs !== undefined && mesh.uvs.length !== vertexCount * 2) {
    throw new TypeError("Mesh UVs must contain one uv pair per vertex");
  }
  if (mesh.faceGroups.length % 3 !== 0) throw new TypeError("Mesh faceGroups must contain u32 triples");
  for (const index of mesh.indices) {
    if (index >= vertexCount) throw new RangeError("Mesh index is outside the vertex array");
  }
  return vertexCount;
}

function copyTessellation(mesh: Tessellation): Tessellation {
  return {
    positions: new Float32Array(mesh.positions),
    normals: new Float32Array(mesh.normals),
    indices: new Uint32Array(mesh.indices),
    faceGroups: new Uint32Array(mesh.faceGroups),
    ...(mesh.uvs === undefined ? {} : { uvs: new Float32Array(mesh.uvs) }),
  };
}

/** Finds deterministic, consistently oriented boundary loops from indexed triangle topology. */
export function findBoundaryLoops(mesh: Tessellation): readonly MeshBoundaryLoop[] {
  validateMesh(mesh);
  const edges = new Map<string, Array<{ from: number; to: number }>>();
  for (let offset = 0; offset < mesh.indices.length; offset += 3) {
    const triangle = [mesh.indices[offset]!, mesh.indices[offset + 1]!, mesh.indices[offset + 2]!];
    if (triangle[0] === triangle[1] || triangle[1] === triangle[2] || triangle[2] === triangle[0]) {
      throw new RangeError("Boundary detection requires triangles with three distinct vertices");
    }
    for (let edge = 0; edge < 3; edge++) {
      const from = triangle[edge]!;
      const to = triangle[(edge + 1) % 3]!;
      const key = from < to ? `${from}:${to}` : `${to}:${from}`;
      const uses = edges.get(key);
      if (uses === undefined) edges.set(key, [{ from, to }]);
      else uses.push({ from, to });
    }
  }

  const outgoing = new Map<number, number>();
  const incoming = new Map<number, number>();
  for (const uses of edges.values()) {
    if (uses.length > 2) throw new RangeError("Boundary detection does not support non-manifold edges");
    if (uses.length === 2) {
      if (uses[0]!.from !== uses[1]!.to || uses[0]!.to !== uses[1]!.from) {
        throw new RangeError("Boundary detection requires consistently oriented triangles");
      }
      continue;
    }
    const { from, to } = uses[0]!;
    if (outgoing.has(from) || incoming.has(to)) {
      throw new RangeError("Boundary detection does not support ambiguous or branching boundary vertices");
    }
    outgoing.set(from, to);
    incoming.set(to, from);
  }
  for (const vertex of new Set([...outgoing.keys(), ...incoming.keys()])) {
    if (!outgoing.has(vertex) || !incoming.has(vertex)) {
      throw new RangeError("Boundary detection requires closed boundary loops");
    }
  }

  const loops: number[][] = [];
  const unused = new Set(outgoing.keys());
  while (unused.size > 0) {
    let start = Number.MAX_SAFE_INTEGER;
    for (const vertex of unused) start = Math.min(start, vertex);
    const loop: number[] = [];
    let vertex = start;
    do {
      if (!unused.delete(vertex)) {
        throw new RangeError("Boundary detection found an ambiguous boundary cycle");
      }
      loop.push(vertex);
      vertex = outgoing.get(vertex)!;
    } while (vertex !== start);
    if (loop.length < 3) throw new RangeError("Boundary loops must contain at least three vertices");
    loops.push(loop);
  }
  loops.sort((left, right) => {
    for (let index = 0; index < Math.min(left.length, right.length); index++) {
      if (left[index] !== right[index]) return left[index]! - right[index]!;
    }
    return left.length - right.length;
  });
  return Object.freeze(loops.map((vertexIndices) => {
    let perimeter = 0;
    for (let index = 0; index < vertexIndices.length; index++) {
      const first = vertexIndices[index]! * 3;
      const second = vertexIndices[(index + 1) % vertexIndices.length]! * 3;
      perimeter += Math.hypot(
        mesh.positions[second]! - mesh.positions[first]!,
        mesh.positions[second + 1]! - mesh.positions[first + 1]!,
        mesh.positions[second + 2]! - mesh.positions[first + 2]!,
      );
    }
    return Object.freeze({ vertexIndices: Object.freeze(vertexIndices), perimeter });
  }));
}

/** Fills selected simple planar boundary loops without adding vertices. */
export function fillPlanarHoles(
  mesh: Tessellation,
  options: PlanarHoleFillOptions = {},
): PlanarHoleFillResult {
  validateMesh(mesh);
  const tolerance = options.planarityTolerance ?? 1e-6;
  if (!(tolerance >= 0) || !Number.isFinite(tolerance)) {
    throw new RangeError("Hole-fill planarity tolerance must be finite and non-negative");
  }
  const boundaryLoops = findBoundaryLoops(mesh);
  const selected = options.loopIndices === undefined
    ? boundaryLoops.map((_, index) => index)
    : [...options.loopIndices];
  const unique = new Set<number>();
  for (const index of selected) {
    if (!Number.isInteger(index) || index < 0 || index >= boundaryLoops.length) {
      throw new RangeError("Hole-fill loop index is outside the boundary-loop list");
    }
    if (unique.has(index)) throw new RangeError("Hole-fill loop indices must be unique");
    unique.add(index);
  }
  selected.sort((left, right) => left - right);
  const currentFaceIndices = Array.from({ length: mesh.faceGroups.length / 3 }, (_, index) => mesh.faceGroups[index * 3]!);
  const faceIndex = options.faceIndex ?? (currentFaceIndices.length === 0 ? 0 : Math.max(...currentFaceIndices) + 1);
  if (!Number.isInteger(faceIndex) || faceIndex < 0 || faceIndex > 0xffffffff) {
    throw new RangeError("Hole-fill faceIndex must be a non-negative u32 integer");
  }

  const addedIndices: number[] = [];
  for (const loopIndex of selected) {
    const boundary = boundaryLoops[loopIndex]!.vertexIndices;
    const vertices = [...boundary].reverse();
    let nx = 0;
    let ny = 0;
    let nz = 0;
    for (let index = 0; index < vertices.length; index++) {
      const first = vertices[index]! * 3;
      const second = vertices[(index + 1) % vertices.length]! * 3;
      const x1 = mesh.positions[first]!;
      const y1 = mesh.positions[first + 1]!;
      const z1 = mesh.positions[first + 2]!;
      const x2 = mesh.positions[second]!;
      const y2 = mesh.positions[second + 1]!;
      const z2 = mesh.positions[second + 2]!;
      nx += (y1 - y2) * (z1 + z2);
      ny += (z1 - z2) * (x1 + x2);
      nz += (x1 - x2) * (y1 + y2);
    }
    const normalLength = Math.hypot(nx, ny, nz);
    if (normalLength <= Number.EPSILON) throw new RangeError("Hole-fill boundary is degenerate");
    nx /= normalLength;
    ny /= normalLength;
    nz /= normalLength;
    const origin = vertices[0]! * 3;
    for (const vertex of vertices) {
      const offset = vertex * 3;
      const distance = Math.abs(
        (mesh.positions[offset]! - mesh.positions[origin]!) * nx
        + (mesh.positions[offset + 1]! - mesh.positions[origin + 1]!) * ny
        + (mesh.positions[offset + 2]! - mesh.positions[origin + 2]!) * nz
      );
      if (distance > tolerance) throw new RangeError("Hole-fill boundary is not planar within tolerance");
    }

    const dropAxis = Math.abs(nx) >= Math.abs(ny) && Math.abs(nx) >= Math.abs(nz)
      ? 0
      : Math.abs(ny) >= Math.abs(nz) ? 1 : 2;
    const point = (vertex: number): readonly [number, number] => {
      const offset = vertex * 3;
      if (dropAxis === 0) return [mesh.positions[offset + 1]!, mesh.positions[offset + 2]!];
      if (dropAxis === 1) return [mesh.positions[offset]!, mesh.positions[offset + 2]!];
      return [mesh.positions[offset]!, mesh.positions[offset + 1]!];
    };
    const cross = (a: number, b: number, c: number): number => {
      const [ax, ay] = point(a);
      const [bx, by] = point(b);
      const [cx, cy] = point(c);
      return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    };
    const intersects = (a: number, b: number, c: number, d: number): boolean => {
      const abC = cross(a, b, c);
      const abD = cross(a, b, d);
      const cdA = cross(c, d, a);
      const cdB = cross(c, d, b);
      if (abC * abD < 0 && cdA * cdB < 0) return true;
      const [ax, ay] = point(a);
      const [bx, by] = point(b);
      const [cx, cy] = point(c);
      const [dx, dy] = point(d);
      const onSegment = (
        value: number,
        x1: number,
        y1: number,
        x2: number,
        y2: number,
        px: number,
        py: number,
      ): boolean => value === 0
        && px >= Math.min(x1, x2) && px <= Math.max(x1, x2)
        && py >= Math.min(y1, y2) && py <= Math.max(y1, y2);
      return onSegment(abC, ax, ay, bx, by, cx, cy)
        || onSegment(abD, ax, ay, bx, by, dx, dy)
        || onSegment(cdA, cx, cy, dx, dy, ax, ay)
        || onSegment(cdB, cx, cy, dx, dy, bx, by);
    };
    for (let first = 0; first < vertices.length; first++) {
      const firstNext = (first + 1) % vertices.length;
      for (let second = first + 1; second < vertices.length; second++) {
        const secondNext = (second + 1) % vertices.length;
        if (first === secondNext || firstNext === second) continue;
        if (intersects(vertices[first]!, vertices[firstNext]!, vertices[second]!, vertices[secondNext]!)) {
          throw new RangeError("Hole-fill boundary must be a simple polygon");
        }
      }
    }

    let signedArea = 0;
    for (let index = 0; index < vertices.length; index++) {
      const [x1, y1] = point(vertices[index]!);
      const [x2, y2] = point(vertices[(index + 1) % vertices.length]!);
      signedArea += x1 * y2 - x2 * y1;
    }
    const winding = Math.sign(signedArea);
    if (winding === 0) throw new RangeError("Hole-fill boundary is degenerate");
    const polygon = [...vertices];
    while (polygon.length > 3) {
      let ear = -1;
      for (let index = 0; index < polygon.length; index++) {
        const a = polygon[(index + polygon.length - 1) % polygon.length]!;
        const b = polygon[index]!;
        const c = polygon[(index + 1) % polygon.length]!;
        if (cross(a, b, c) * winding <= 0) continue;
        let containsVertex = false;
        for (const candidate of polygon) {
          if (candidate === a || candidate === b || candidate === c) continue;
          const first = cross(a, b, candidate) * winding;
          const second = cross(b, c, candidate) * winding;
          const third = cross(c, a, candidate) * winding;
          if (first >= 0 && second >= 0 && third >= 0) {
            containsVertex = true;
            break;
          }
        }
        if (!containsVertex) {
          ear = index;
          addedIndices.push(a, b, c);
          break;
        }
      }
      if (ear < 0) throw new RangeError("Hole-fill boundary cannot be triangulated as a simple polygon");
      polygon.splice(ear, 1);
    }
    addedIndices.push(polygon[0]!, polygon[1]!, polygon[2]!);
  }

  const indices = new Uint32Array(mesh.indices.length + addedIndices.length);
  indices.set(mesh.indices);
  indices.set(addedIndices, mesh.indices.length);
  const faceGroups = new Uint32Array(mesh.faceGroups.length + (addedIndices.length === 0 ? 0 : 3));
  faceGroups.set(mesh.faceGroups);
  if (addedIndices.length > 0) faceGroups.set([faceIndex, mesh.indices.length, addedIndices.length], mesh.faceGroups.length);
  const filled = recalculateVertexNormals({
    positions: new Float32Array(mesh.positions),
    normals: new Float32Array(mesh.normals),
    indices,
    faceGroups,
    ...(mesh.uvs === undefined ? {} : { uvs: new Float32Array(mesh.uvs) }),
  });
  return {
    mesh: filled,
    boundaryLoops,
    filledLoopIndices: Object.freeze(selected),
    addedTriangleCount: addedIndices.length / 3,
  };
}

/** Deterministic grid-cluster simplification for render tessellations. */
export function simplifyTessellation(mesh: Tessellation, options: MeshSimplifyOptions = {}): Tessellation {
  const vertexCount = validateMesh(mesh);
  const triangleCount = mesh.indices.length / 3;
  if (!Number.isInteger(options.targetTriangles ?? triangleCount) || (options.targetTriangles ?? triangleCount) <= 0) {
    throw new RangeError("Mesh targetTriangles must be a positive integer");
  }
  if (options.ratio !== undefined && (!(options.ratio > 0) || options.ratio > 1 || !Number.isFinite(options.ratio))) {
    throw new RangeError("Mesh simplification ratio must be greater than zero and at most one");
  }
  const requestedTarget = options.targetTriangles ?? Math.max(1, Math.floor(triangleCount * (options.ratio ?? 1)));
  const targetTriangles = Math.min(triangleCount, requestedTarget);
  if (triangleCount <= targetTriangles || vertexCount === 0) return copyTessellation(mesh);

  let minX = Infinity;
  let minY = Infinity;
  let minZ = Infinity;
  let maxX = -Infinity;
  let maxY = -Infinity;
  let maxZ = -Infinity;
  for (let index = 0; index < mesh.positions.length; index += 3) {
    minX = Math.min(minX, mesh.positions[index]!);
    minY = Math.min(minY, mesh.positions[index + 1]!);
    minZ = Math.min(minZ, mesh.positions[index + 2]!);
    maxX = Math.max(maxX, mesh.positions[index]!);
    maxY = Math.max(maxY, mesh.positions[index + 1]!);
    maxZ = Math.max(maxZ, mesh.positions[index + 2]!);
  }
  const diagonal = Math.hypot(maxX - minX, maxY - minY, maxZ - minZ);
  let cellSize = Math.max(diagonal / Math.sqrt(targetTriangles), diagonal * 1e-9, Number.EPSILON);
  let best: Tessellation | undefined;
  for (let attempt = 0; attempt < 48; attempt++) {
    const clusters = new Map<string, number>();
    const remap = new Uint32Array(vertexCount);
    const positions: number[] = [];
    const normalSums: number[] = [];
    const uvSums: number[] = [];
    const counts: number[] = [];
    const key = (x: number, y: number, z: number): string => `${Math.floor(x / cellSize)},${Math.floor(y / cellSize)},${Math.floor(z / cellSize)}`;
    for (let source = 0; source < vertexCount; source++) {
      const px = mesh.positions[source * 3]!;
      const py = mesh.positions[source * 3 + 1]!;
      const pz = mesh.positions[source * 3 + 2]!;
      const clusterKey = key(px, py, pz);
      let cluster = clusters.get(clusterKey);
      if (cluster === undefined) {
        cluster = positions.length / 3;
        clusters.set(clusterKey, cluster);
        positions.push(0, 0, 0);
        normalSums.push(0, 0, 0);
        if (mesh.uvs !== undefined) uvSums.push(0, 0);
        counts.push(0);
      }
      remap[source] = cluster;
      positions[cluster * 3] = positions[cluster * 3]! + px;
      positions[cluster * 3 + 1] = positions[cluster * 3 + 1]! + py;
      positions[cluster * 3 + 2] = positions[cluster * 3 + 2]! + pz;
      normalSums[cluster * 3] = normalSums[cluster * 3]! + mesh.normals[source * 3]!;
      normalSums[cluster * 3 + 1] = normalSums[cluster * 3 + 1]! + mesh.normals[source * 3 + 1]!;
      normalSums[cluster * 3 + 2] = normalSums[cluster * 3 + 2]! + mesh.normals[source * 3 + 2]!;
      if (mesh.uvs !== undefined) {
        uvSums[cluster * 2] = uvSums[cluster * 2]! + mesh.uvs[source * 2]!;
        uvSums[cluster * 2 + 1] = uvSums[cluster * 2 + 1]! + mesh.uvs[source * 2 + 1]!;
      }
      counts[cluster] = counts[cluster]! + 1;
    }
    for (let cluster = 0; cluster < counts.length; cluster++) {
      const count = counts[cluster]!;
      positions[cluster * 3] = positions[cluster * 3]! / count;
      positions[cluster * 3 + 1] = positions[cluster * 3 + 1]! / count;
      positions[cluster * 3 + 2] = positions[cluster * 3 + 2]! / count;
      if (mesh.uvs !== undefined) {
        uvSums[cluster * 2] = uvSums[cluster * 2]! / count;
        uvSums[cluster * 2 + 1] = uvSums[cluster * 2 + 1]! / count;
      }
    }
    const indices: number[] = [];
    const faceGroups: number[] = [];
    const appendGroup = (faceIndex: number, start: number, count: number): void => {
      const groupStart = indices.length;
      for (let offset = start; offset < start + count; offset += 3) {
        const a = remap[mesh.indices[offset]!]!;
        const b = remap[mesh.indices[offset + 1]!]!;
        const c = remap[mesh.indices[offset + 2]!]!;
        if (a === b || b === c || a === c) continue;
        indices.push(a, b, c);
      }
      if (indices.length > groupStart) faceGroups.push(faceIndex, groupStart, indices.length - groupStart);
    };
    for (let group = 0; group < mesh.faceGroups.length; group += 3) {
      const faceIndex = mesh.faceGroups[group]!;
      const start = mesh.faceGroups[group + 1]!;
      const count = mesh.faceGroups[group + 2]!;
      if (start % 3 !== 0 || count % 3 !== 0 || start + count > mesh.indices.length) {
        throw new RangeError("Mesh faceGroups contain an invalid index range");
      }
      appendGroup(faceIndex, start, count);
    }
    if (indices.length === 0) break;
    const candidate: Tessellation = {
      positions: new Float32Array(positions),
      normals: new Float32Array(normalSums),
      indices: new Uint32Array(indices),
      faceGroups: new Uint32Array(faceGroups),
      ...(mesh.uvs === undefined ? {} : { uvs: new Float32Array(uvSums) }),
    };
    candidate.normals = recalculateVertexNormals(candidate).normals;
    best = candidate;
    if (indices.length / 3 <= targetTriangles) break;
    cellSize *= 1.5;
  }
  return best ?? copyTessellation(mesh);
}

export function subdivideTessellation(mesh: Tessellation, options: MeshSubdivisionOptions = {}): Tessellation {
  validateMesh(mesh);
  const levels = options.levels ?? 1;
  if (!Number.isInteger(levels) || levels < 1 || levels > 8) {
    throw new RangeError("Mesh subdivision levels must be an integer between 1 and 8");
  }
  let result = copyTessellation(mesh);
  for (let level = 0; level < levels; level++) {
    const positions = Array.from(result.positions);
    const normals = Array.from(result.normals);
    const uvs = result.uvs === undefined ? undefined : Array.from(result.uvs);
    const midpointCache = new Map<string, number>();
    const midpoint = (first: number, second: number): number => {
      const key = first < second ? `${first}:${second}` : `${second}:${first}`;
      const cached = midpointCache.get(key);
      if (cached !== undefined) return cached;
      const index = positions.length / 3;
      positions.push(
        (result.positions[first * 3]! + result.positions[second * 3]!) / 2,
        (result.positions[first * 3 + 1]! + result.positions[second * 3 + 1]!) / 2,
        (result.positions[first * 3 + 2]! + result.positions[second * 3 + 2]!) / 2,
      );
      normals.push(
        (result.normals[first * 3]! + result.normals[second * 3]!) / 2,
        (result.normals[first * 3 + 1]! + result.normals[second * 3 + 1]!) / 2,
        (result.normals[first * 3 + 2]! + result.normals[second * 3 + 2]!) / 2,
      );
      if (uvs !== undefined) {
        uvs.push(
          (result.uvs![first * 2]! + result.uvs![second * 2]!) / 2,
          (result.uvs![first * 2 + 1]! + result.uvs![second * 2 + 1]!) / 2,
        );
      }
      midpointCache.set(key, index);
      return index;
    };
    const indices: number[] = [];
    const faceGroups: number[] = [];
    for (let group = 0; group < result.faceGroups.length; group += 3) {
      const faceIndex = result.faceGroups[group]!;
      const start = result.faceGroups[group + 1]!;
      const count = result.faceGroups[group + 2]!;
      if (start % 3 !== 0 || count % 3 !== 0 || start + count > result.indices.length) {
        throw new RangeError("Mesh faceGroups contain an invalid index range");
      }
      const groupStart = indices.length;
      for (let offset = start; offset < start + count; offset += 3) {
        const a = result.indices[offset]!;
        const b = result.indices[offset + 1]!;
        const c = result.indices[offset + 2]!;
        const ab = midpoint(a, b);
        const bc = midpoint(b, c);
        const ca = midpoint(c, a);
        indices.push(a, ab, ca, ab, b, bc, ca, bc, c, ab, bc, ca);
      }
      if (indices.length > groupStart) faceGroups.push(faceIndex, groupStart, indices.length - groupStart);
    }
    result = {
      positions: new Float32Array(positions),
      normals: new Float32Array(normals),
      indices: new Uint32Array(indices),
      faceGroups: new Uint32Array(faceGroups),
      ...(uvs === undefined ? {} : { uvs: new Float32Array(uvs) }),
    };
    result.normals = recalculateVertexNormals(result).normals;
  }
  return result;
}

export function weldTessellation(mesh: Tessellation, options: MeshWeldOptions = {}): Tessellation {
  const vertexCount = validateMesh(mesh);
  const tolerance = options.tolerance ?? 1e-7;
  const normalTolerance = options.normalTolerance ?? 1e-3;
  const uvTolerance = options.uvTolerance ?? tolerance;
  if (!(tolerance > 0) || !Number.isFinite(tolerance)) throw new RangeError("Mesh weld tolerance must be positive and finite");
  if (!(normalTolerance >= 0) || !Number.isFinite(normalTolerance)) throw new RangeError("Mesh normal tolerance must be finite and non-negative");
  if (!(uvTolerance >= 0) || !Number.isFinite(uvTolerance)) throw new RangeError("Mesh UV tolerance must be finite and non-negative");

  const respectNormals = options.respectNormals ?? true;
  const respectUVs = options.respectUVs ?? true;
  const toleranceSquared = tolerance * tolerance;
  const normalCosine = Math.cos(normalTolerance);
  const uvToleranceSquared = uvTolerance * uvTolerance;
  const cells = new Map<string, number[]>();
  const remap = new Uint32Array(vertexCount);
  const positions: number[] = [];
  const normalSums: number[] = [];
  const uvSums: number[] = [];
  const counts: number[] = [];
  const key = (x: number, y: number, z: number): string => `${x},${y},${z}`;

  for (let source = 0; source < vertexCount; source++) {
    const px = mesh.positions[source * 3]!;
    const py = mesh.positions[source * 3 + 1]!;
    const pz = mesh.positions[source * 3 + 2]!;
    const cx = Math.floor(px / tolerance);
    const cy = Math.floor(py / tolerance);
    const cz = Math.floor(pz / tolerance);
    let target = -1;
    for (let dz = -1; dz <= 1 && target === -1; dz++) {
      for (let dy = -1; dy <= 1 && target === -1; dy++) {
        for (let dx = -1; dx <= 1 && target === -1; dx++) {
          for (const candidate of cells.get(key(cx + dx, cy + dy, cz + dz)) ?? []) {
            const qx = positions[candidate * 3]!;
            const qy = positions[candidate * 3 + 1]!;
            const qz = positions[candidate * 3 + 2]!;
            const distanceSquared = (px - qx) ** 2 + (py - qy) ** 2 + (pz - qz) ** 2;
            if (distanceSquared > toleranceSquared) continue;
            if (respectNormals) {
              const normalLength = Math.hypot(
                normalSums[candidate * 3]!, normalSums[candidate * 3 + 1]!, normalSums[candidate * 3 + 2]!,
              );
              const sourceLength = Math.hypot(
                mesh.normals[source * 3]!, mesh.normals[source * 3 + 1]!, mesh.normals[source * 3 + 2]!,
              );
              const dot = normalLength === 0 || sourceLength === 0 ? -1 : (
                normalSums[candidate * 3]! * mesh.normals[source * 3]!
                + normalSums[candidate * 3 + 1]! * mesh.normals[source * 3 + 1]!
                + normalSums[candidate * 3 + 2]! * mesh.normals[source * 3 + 2]!
              ) / (normalLength * sourceLength);
              if (dot < normalCosine) continue;
            }
            if (respectUVs && mesh.uvs !== undefined) {
              const count = counts[candidate]!;
              const u = uvSums[candidate * 2]! / count;
              const v = uvSums[candidate * 2 + 1]! / count;
              if ((mesh.uvs[source * 2]! - u) ** 2 + (mesh.uvs[source * 2 + 1]! - v) ** 2
                  > uvToleranceSquared) continue;
            }
            target = candidate;
            break;
          }
        }
      }
    }
    if (target === -1) {
      target = positions.length / 3;
      positions.push(px, py, pz);
      normalSums.push(mesh.normals[source * 3]!, mesh.normals[source * 3 + 1]!, mesh.normals[source * 3 + 2]!);
      if (mesh.uvs !== undefined) uvSums.push(mesh.uvs[source * 2]!, mesh.uvs[source * 2 + 1]!);
      counts.push(1);
      const cellKey = key(cx, cy, cz);
      const entries = cells.get(cellKey);
      if (entries === undefined) cells.set(cellKey, [target]);
      else entries.push(target);
    } else {
      normalSums[target * 3] = normalSums[target * 3]! + mesh.normals[source * 3]!;
      normalSums[target * 3 + 1] = normalSums[target * 3 + 1]! + mesh.normals[source * 3 + 1]!;
      normalSums[target * 3 + 2] = normalSums[target * 3 + 2]! + mesh.normals[source * 3 + 2]!;
      if (mesh.uvs !== undefined) {
        uvSums[target * 2] = uvSums[target * 2]! + mesh.uvs[source * 2]!;
        uvSums[target * 2 + 1] = uvSums[target * 2 + 1]! + mesh.uvs[source * 2 + 1]!;
      }
      counts[target] = counts[target]! + 1;
    }
    remap[source] = target;
  }

  const normals = new Float32Array(positions.length);
  for (let vertex = 0; vertex < counts.length; vertex++) {
    const length = Math.hypot(normalSums[vertex * 3]!, normalSums[vertex * 3 + 1]!, normalSums[vertex * 3 + 2]!);
    if (length > 0) {
      normals[vertex * 3] = normalSums[vertex * 3]! / length;
      normals[vertex * 3 + 1] = normalSums[vertex * 3 + 1]! / length;
      normals[vertex * 3 + 2] = normalSums[vertex * 3 + 2]! / length;
    }
  }
  const uvs = mesh.uvs === undefined ? undefined : new Float32Array(counts.length * 2);
  if (uvs !== undefined) {
    for (let vertex = 0; vertex < counts.length; vertex++) {
      uvs[vertex * 2] = uvSums[vertex * 2]! / counts[vertex]!;
      uvs[vertex * 2 + 1] = uvSums[vertex * 2 + 1]! / counts[vertex]!;
    }
  }

  const indices: number[] = [];
  const faceGroups: number[] = [];
  for (let group = 0; group < mesh.faceGroups.length; group += 3) {
    const faceIndex = mesh.faceGroups[group]!;
    const sourceStart = mesh.faceGroups[group + 1]!;
    const sourceCount = mesh.faceGroups[group + 2]!;
    if (sourceStart + sourceCount > mesh.indices.length || sourceStart % 3 !== 0 || sourceCount % 3 !== 0) {
      throw new RangeError("Mesh faceGroup index range is invalid");
    }
    const targetStart = indices.length;
    for (let index = sourceStart; index < sourceStart + sourceCount; index += 3) {
      const a = remap[mesh.indices[index]!]!;
      const b = remap[mesh.indices[index + 1]!]!;
      const c = remap[mesh.indices[index + 2]!]!;
      if (a !== b && b !== c && a !== c) indices.push(a, b, c);
    }
    faceGroups.push(faceIndex, targetStart, indices.length - targetStart);
  }
  return {
    positions: new Float32Array(positions),
    normals,
    indices: new Uint32Array(indices),
    faceGroups: new Uint32Array(faceGroups),
    ...(uvs === undefined ? {} : { uvs }),
  };
}

export function recalculateVertexNormals(mesh: Tessellation): Tessellation {
  const vertexCount = validateMesh(mesh);
  const normals = new Float32Array(vertexCount * 3);
  for (let index = 0; index < mesh.indices.length; index += 3) {
    const a = mesh.indices[index]!;
    const b = mesh.indices[index + 1]!;
    const c = mesh.indices[index + 2]!;
    const abx = mesh.positions[b * 3]! - mesh.positions[a * 3]!;
    const aby = mesh.positions[b * 3 + 1]! - mesh.positions[a * 3 + 1]!;
    const abz = mesh.positions[b * 3 + 2]! - mesh.positions[a * 3 + 2]!;
    const acx = mesh.positions[c * 3]! - mesh.positions[a * 3]!;
    const acy = mesh.positions[c * 3 + 1]! - mesh.positions[a * 3 + 1]!;
    const acz = mesh.positions[c * 3 + 2]! - mesh.positions[a * 3 + 2]!;
    const nx = aby * acz - abz * acy;
    const ny = abz * acx - abx * acz;
    const nz = abx * acy - aby * acx;
    for (const vertex of [a, b, c]) {
      normals[vertex * 3] = normals[vertex * 3]! + nx;
      normals[vertex * 3 + 1] = normals[vertex * 3 + 1]! + ny;
      normals[vertex * 3 + 2] = normals[vertex * 3 + 2]! + nz;
    }
  }
  for (let vertex = 0; vertex < vertexCount; vertex++) {
    const length = Math.hypot(normals[vertex * 3]!, normals[vertex * 3 + 1]!, normals[vertex * 3 + 2]!);
    if (length > 0) {
      normals[vertex * 3] = normals[vertex * 3]! / length;
      normals[vertex * 3 + 1] = normals[vertex * 3 + 1]! / length;
      normals[vertex * 3 + 2] = normals[vertex * 3 + 2]! / length;
    }
  }
  return { ...mesh, normals };
}

export function generateTangents(mesh: Tessellation): Float32Array {
  const vertexCount = validateMesh(mesh);
  if (mesh.uvs === undefined) throw new TypeError("Mesh tangent generation requires UVs");
  const tangent = new Float64Array(vertexCount * 3);
  const bitangent = new Float64Array(vertexCount * 3);
  for (let index = 0; index < mesh.indices.length; index += 3) {
    const a = mesh.indices[index]!;
    const b = mesh.indices[index + 1]!;
    const c = mesh.indices[index + 2]!;
    const x1 = mesh.positions[b * 3]! - mesh.positions[a * 3]!;
    const y1 = mesh.positions[b * 3 + 1]! - mesh.positions[a * 3 + 1]!;
    const z1 = mesh.positions[b * 3 + 2]! - mesh.positions[a * 3 + 2]!;
    const x2 = mesh.positions[c * 3]! - mesh.positions[a * 3]!;
    const y2 = mesh.positions[c * 3 + 1]! - mesh.positions[a * 3 + 1]!;
    const z2 = mesh.positions[c * 3 + 2]! - mesh.positions[a * 3 + 2]!;
    const s1 = mesh.uvs[b * 2]! - mesh.uvs[a * 2]!;
    const t1 = mesh.uvs[b * 2 + 1]! - mesh.uvs[a * 2 + 1]!;
    const s2 = mesh.uvs[c * 2]! - mesh.uvs[a * 2]!;
    const t2 = mesh.uvs[c * 2 + 1]! - mesh.uvs[a * 2 + 1]!;
    const determinant = s1 * t2 - s2 * t1;
    if (Math.abs(determinant) <= Number.EPSILON) continue;
    const r = 1 / determinant;
    const tx = (x1 * t2 - x2 * t1) * r;
    const ty = (y1 * t2 - y2 * t1) * r;
    const tz = (z1 * t2 - z2 * t1) * r;
    const bx = (x2 * s1 - x1 * s2) * r;
    const by = (y2 * s1 - y1 * s2) * r;
    const bz = (z2 * s1 - z1 * s2) * r;
    for (const vertex of [a, b, c]) {
      tangent[vertex * 3] = tangent[vertex * 3]! + tx;
      tangent[vertex * 3 + 1] = tangent[vertex * 3 + 1]! + ty;
      tangent[vertex * 3 + 2] = tangent[vertex * 3 + 2]! + tz;
      bitangent[vertex * 3] = bitangent[vertex * 3]! + bx;
      bitangent[vertex * 3 + 1] = bitangent[vertex * 3 + 1]! + by;
      bitangent[vertex * 3 + 2] = bitangent[vertex * 3 + 2]! + bz;
    }
  }
  const result = new Float32Array(vertexCount * 4);
  for (let vertex = 0; vertex < vertexCount; vertex++) {
    const nx = mesh.normals[vertex * 3]!;
    const ny = mesh.normals[vertex * 3 + 1]!;
    const nz = mesh.normals[vertex * 3 + 2]!;
    const dot = nx * tangent[vertex * 3]! + ny * tangent[vertex * 3 + 1]! + nz * tangent[vertex * 3 + 2]!;
    let tx = tangent[vertex * 3]! - nx * dot;
    let ty = tangent[vertex * 3 + 1]! - ny * dot;
    let tz = tangent[vertex * 3 + 2]! - nz * dot;
    const length = Math.hypot(tx, ty, tz);
    if (length > 0) {
      tx /= length;
      ty /= length;
      tz /= length;
    }
    const cx = ny * tz - nz * ty;
    const cy = nz * tx - nx * tz;
    const cz = nx * ty - ny * tx;
    result[vertex * 4] = tx;
    result[vertex * 4 + 1] = ty;
    result[vertex * 4 + 2] = tz;
    result[vertex * 4 + 3] = cx * bitangent[vertex * 3]!
      + cy * bitangent[vertex * 3 + 1]! + cz * bitangent[vertex * 3 + 2]! < 0 ? -1 : 1;
  }
  return result;
}
