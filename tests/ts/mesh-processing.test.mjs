import assert from "node:assert/strict";
import test from "node:test";
import {
  EdgeSelectionMap,
  FaceSelectionMap,
  fillPlanarHoles,
  findBoundaryLoops,
  generateTangents,
  recalculateVertexNormals,
  simplifyTessellation,
  subdivideTessellation,
  weldTessellation,
} from "../../dist/index.js";

function squareRing() {
  return {
    positions: new Float32Array([
      0, 0, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0,
      1, 1, 0, 3, 1, 0, 3, 3, 0, 1, 3, 0,
    ]),
    normals: new Float32Array(Array.from({ length: 8 }, () => [0, 0, 1]).flat()),
    indices: new Uint32Array([
      0, 1, 5, 0, 5, 4,
      1, 2, 6, 1, 6, 5,
      2, 3, 7, 2, 7, 6,
      3, 0, 4, 3, 4, 7,
    ]),
    faceGroups: new Uint32Array([2, 0, 24]),
  };
}

function duplicatedQuad() {
  return {
    positions: new Float32Array([
      0, 0, 0, 1, 0, 0, 1, 1, 0,
      0, 0, 0, 1, 1, 0, 0, 1, 0,
    ]),
    normals: new Float32Array([
      0, 0, 1, 0, 0, 1, 0, 0, 1,
      0, 0, 1, 0, 0, 1, 0, 0, 1,
    ]),
    uvs: new Float32Array([
      0, 0, 1, 0, 1, 1,
      0, 0, 1, 1, 0, 1,
    ]),
    indices: new Uint32Array([0, 1, 2, 3, 4, 5]),
    faceGroups: new Uint32Array([7, 0, 6]),
  };
}

test("mesh welding merges duplicate attributes and preserves face groups", () => {
  const welded = weldTessellation(duplicatedQuad());
  assert.equal(welded.positions.length / 3, 4);
  assert.deepEqual([...welded.indices], [0, 1, 2, 0, 2, 3]);
  assert.deepEqual([...welded.faceGroups], [7, 0, 6]);
  assert.deepEqual([...welded.uvs], [0, 0, 1, 0, 1, 1, 0, 1]);
});

test("mesh welding respects hard normal and UV seams", () => {
  const mesh = duplicatedQuad();
  mesh.normals[9 + 2] = -1;
  assert.equal(weldTessellation(mesh).positions.length / 3, 5);

  const uvSeam = duplicatedQuad();
  uvSeam.uvs[6] = 0.5;
  assert.equal(weldTessellation(uvSeam).positions.length / 3, 5);
});

test("normal recalculation and tangent generation produce render-ready attributes", () => {
  const mesh = duplicatedQuad();
  mesh.normals.fill(0);
  const withNormals = recalculateVertexNormals(mesh);
  for (let index = 0; index < withNormals.normals.length; index += 3) {
    assert.deepEqual([...withNormals.normals.slice(index, index + 3)], [0, 0, 1]);
  }
  const tangents = generateTangents(withNormals);
  assert.equal(tangents.length, 24);
  for (let index = 0; index < tangents.length; index += 4) {
    assert.deepEqual([...tangents.slice(index, index + 4)], [1, 0, 0, 1]);
  }
});

test("mesh simplification reduces triangles and rebuilds render attributes", () => {
  const size = 6;
  const positions = [];
  const normals = [];
  const indices = [];
  for (let y = 0; y <= size; y++) {
    for (let x = 0; x <= size; x++) {
      positions.push(x, y, 0);
      normals.push(0, 0, 1);
    }
  }
  const vertex = (x, y) => y * (size + 1) + x;
  for (let y = 0; y < size; y++) {
    for (let x = 0; x < size; x++) {
      const a = vertex(x, y);
      const b = vertex(x + 1, y);
      const c = vertex(x + 1, y + 1);
      const d = vertex(x, y + 1);
      indices.push(a, b, c, a, c, d);
    }
  }
  const simplified = simplifyTessellation({
    positions: new Float32Array(positions),
    normals: new Float32Array(normals),
    indices: new Uint32Array(indices),
    faceGroups: new Uint32Array([4, 0, indices.length]),
  }, { targetTriangles: 12 });
  assert.ok(simplified.indices.length / 3 <= 12);
  assert.ok(simplified.indices.length > 0);
  assert.equal(simplified.positions.length, simplified.normals.length);
  assert.deepEqual([...simplified.faceGroups.slice(0, 1)], [4]);
  assert.equal(simplified.faceGroups[2], simplified.indices.length);
});

test("mesh subdivision refines triangles and keeps face groups", () => {
  const refined = subdivideTessellation({
    positions: new Float32Array([0, 0, 0, 1, 0, 0, 0, 1, 0]),
    normals: new Float32Array([0, 0, 1, 0, 0, 1, 0, 0, 1]),
    indices: new Uint32Array([0, 1, 2]),
    faceGroups: new Uint32Array([9, 0, 3]),
  }, { levels: 2 });
  assert.equal(refined.indices.length / 3, 16);
  assert.deepEqual([...refined.faceGroups], [9, 0, 48]);
  assert.equal(refined.normals.length, refined.positions.length);
});

test("topology selection maps resolve primitives and reverse ranges", () => {
  const mesh = duplicatedQuad();
  mesh.faceGroups = new Uint32Array([7, 0, 3, 9, 3, 3]);
  const faces = new FaceSelectionMap(mesh);
  assert.deepEqual(faces.faceIndices, [7, 9]);
  assert.equal(faces.faceAtTriangle(0), 7);
  assert.equal(faces.faceAtTriangle(1), 9);
  assert.deepEqual(faces.rangesForFace(9), [{ faceIndex: 9, triangleStart: 1, triangleCount: 1 }]);
  assert.throws(() => faces.faceAtTriangle(2), /outside/);

  const edges = new EdgeSelectionMap({
    positions: new Float32Array([0, 0, 0, 1, 0, 0, 2, 0, 0, 2, 1, 0, 2, 2, 0]),
    edgeGroups: new Uint32Array([2, 0, 3, 5, 3, 2]),
  });
  assert.deepEqual(edges.edgeIndices, [2, 5]);
  assert.equal(edges.edgeAtVertex(1), 2);
  assert.equal(edges.edgeAtVertex(4), 5);
  assert.deepEqual(edges.rangesForEdge(5), [{ edgeIndex: 5, vertexStart: 3, vertexCount: 2 }]);

  assert.throws(() => new FaceSelectionMap({
    ...mesh,
    faceGroups: new Uint32Array([1, 0, 6, 2, 3, 3]),
  }), /overlapping/);
  assert.throws(() => new EdgeSelectionMap({
    positions: new Float32Array([0, 0, 0, 1, 0, 0]),
    edgeGroups: new Uint32Array([1, 0, 2, 2, 1, 1]),
  }), /overlapping|invalid/);
});

test("mesh processing retains topology selection maps", () => {
  const welded = weldTessellation(duplicatedQuad());
  const weldedSelection = new FaceSelectionMap(welded);
  assert.equal(weldedSelection.faceAtTriangle(0), 7);
  assert.equal(weldedSelection.faceAtTriangle(1), 7);

  const refined = subdivideTessellation(welded, { levels: 1 });
  const refinedSelection = new FaceSelectionMap(refined);
  for (let triangle = 0; triangle < refined.indices.length / 3; triangle++) {
    assert.equal(refinedSelection.faceAtTriangle(triangle), 7);
  }
});

test("boundary loops are deterministic and planar hole filling preserves groups", () => {
  const mesh = squareRing();
  const loops = findBoundaryLoops(mesh);
  assert.deepEqual(loops.map((loop) => loop.vertexIndices), [
    [0, 1, 2, 3],
    [4, 7, 6, 5],
  ]);
  assert.deepEqual(loops.map((loop) => loop.perimeter), [16, 8]);

  const filled = fillPlanarHoles(mesh, { loopIndices: [1], faceIndex: 9 });
  assert.equal(filled.addedTriangleCount, 2);
  assert.deepEqual(filled.filledLoopIndices, [1]);
  assert.deepEqual([...filled.mesh.faceGroups], [2, 0, 24, 9, 24, 6]);
  assert.deepEqual(findBoundaryLoops(filled.mesh).map((loop) => loop.vertexIndices), [[0, 1, 2, 3]]);
  for (let offset = 0; offset < filled.mesh.normals.length; offset += 3) {
    assert.ok(Math.abs(filled.mesh.normals[offset]) < 1e-7);
    assert.ok(Math.abs(filled.mesh.normals[offset + 1]) < 1e-7);
    assert.ok(Math.abs(filled.mesh.normals[offset + 2] - 1) < 1e-7);
  }
});

test("planar hole filling triangulates a concave simple loop", () => {
  const mesh = {
    positions: new Float32Array([
      0, 0, 0, 3, 0, 0, 3, 3, 0, 1.5, 1.5, 0, 0, 3, 0,
      1.5, 1.5, 1,
    ]),
    normals: new Float32Array(18),
    indices: new Uint32Array([0, 1, 5, 1, 2, 5, 2, 3, 5, 3, 4, 5, 4, 0, 5]),
    faceGroups: new Uint32Array([1, 0, 15]),
  };
  const filled = fillPlanarHoles(mesh);
  assert.equal(filled.addedTriangleCount, 3);
  assert.deepEqual(findBoundaryLoops(filled.mesh), []);
  assert.deepEqual([...filled.mesh.faceGroups], [1, 0, 15, 2, 15, 9]);
});

test("boundary processing rejects non-manifold and non-planar input", () => {
  const nonManifold = {
    positions: new Float32Array([
      0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 1,
    ]),
    normals: new Float32Array(15),
    indices: new Uint32Array([0, 1, 2, 1, 0, 3, 0, 1, 4]),
    faceGroups: new Uint32Array([0, 0, 9]),
  };
  assert.throws(() => findBoundaryLoops(nonManifold), /non-manifold/);

  const warped = {
    positions: new Float32Array([0, 0, 0, 1, 0, 0, 1, 1, 0.1, 0, 1, 0]),
    normals: new Float32Array(12),
    indices: new Uint32Array([0, 1, 2, 0, 2, 3]),
    faceGroups: new Uint32Array([0, 0, 6]),
  };
  assert.throws(() => fillPlanarHoles(warped, { planarityTolerance: 1e-4 }), /not planar/);
});

test("mesh processing rejects non-finite attributes at its boundary", () => {
  const mesh = duplicatedQuad();
  mesh.positions[0] = Number.NaN;
  assert.throws(() => weldTessellation(mesh), /finite/);

  const uvMesh = duplicatedQuad();
  uvMesh.uvs[0] = Number.POSITIVE_INFINITY;
  assert.throws(() => generateTangents(uvMesh), /finite/);

  assert.throws(() => new EdgeSelectionMap({
    positions: new Float32Array([0, 0, 0, Number.NaN, 0, 0]),
    edgeGroups: new Uint32Array([0, 0, 2]),
  }), /finite/);
});
