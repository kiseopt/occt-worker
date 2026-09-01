# Protocol v1.2.0

[中文说明 / Chinese guide](protocol.zh-CN.md)

> **Version note:** This page specifies Protocol Wire Version `v1.2.0` (as recorded in `protocol/artifacts.json`). The npm package release version is tracked independently in `package.json`.

## Requests and responses

The authoritative protocol definitions live in `protocol/`. `operation-contracts.json` defines every operation's arguments and result. Generation fails when it or `historySupport` omits an operation. `protocol.schema.json` contains the generated request and result schemas.

Protocol frames use these shapes:

- Request: `{ id, op, args }`
- Success: `{ id, ok: true, result }`
- Failure: `{ id, ok: false, error: { code, message, details? } }`

IDs are client-owned and monotonic. `message` is for human diagnosis; `code` and documented `details` are stable fields for programmatic branching. Cooperative cancellation uses `Cancelled`, not an algorithm, tessellation, or exchange failure code.

`details` is optional. A `KernelFailure` from a parsed operation includes `details.operation`. Exchange failures also include `details.format`: `step`, `xcaf`, `iges`, `stl`, `obj`, `ply`, `gltf`, `vrml`, or `brep`. Malformed frames and runtime failures can omit `details`. `probeFormat()` identifies native XCAF binary by its `BINFILE` header and XML by the `XmlXCAF` document format.

### Message frame examples

**1. Request frame:**
```json
{
  "id": 1,
  "op": "makeBox",
  "args": {
    "scopeId": 1,
    "size": [100, 60, 4]
  }
}
```

**2. Success response frame:**
```json
{
  "id": 1,
  "ok": true,
  "result": {
    "shape": 1
  }
}
```

**3. Error response frame:**
```json
{
  "id": 2,
  "ok": false,
  "error": {
    "code": "InvalidArgs",
    "message": "Box dimensions must be positive",
    "details": {
      "operation": "makeBox"
    }
  }
}
```

### WebAssembly serial message ABI

The WebAssembly boundary is synchronous and serial. A single execution turn follows these steps:

1. The host invokes `_initialize` once per instance.
2. The host allocates linear memory for the UTF-8 JSON request using `k_alloc(len)`.
3. The host writes the request bytes into the allocated pointer.
4. The host invokes `k_handle(ptr, len)` and receives the response length in bytes.
5. The host reads exactly that many bytes starting at `k_response_ptr()`.
6. The host frees the request buffer using `k_free(ptr)`.

The public message ABI consists of initialization, the message entry point, allocation/free, and buffer accessors; the complete required WebAssembly import/export surface is frozen in `wasm-surface.json`.

## Handles, scopes, and buffers

- A shape handle is a `u32` value valid only inside one kernel instance.
- The typed API wraps it in an epoch-checked `ShapeHandle`.
- The low-level `request()` method is explicitly unsafe. Raw `u32` values bypass TypeScript owner, epoch, and scope provenance checks.
- Kernel generation checks reject stale handles, but the raw number does not identify its instance. A colliding value can therefore resolve in another live instance.
- A scope owns every shape created with its `scopeId`. `endScope` releases all remaining shapes in that scope; `release` rejects stale or unknown handles.

Buffers are described by `{ bufferId, byteLength, layout }`:

```json
{
  "bufferId": 1,
  "byteLength": 2400,
  "layout": "f32x3"
}
```

- Normal `DirectClient` requests copy output buffers into host memory and immediately free their kernel descriptors.
- `DirectClient.createBuffer()` creates a reusable same-thread `DirectBuffer`. Reacquire `view()` after every WASM call and release the buffer with `freeBuffer()`.
- Worker clients do not expose kernel buffer IDs. Normal requests create transferable `ArrayBuffer` results.
- `WorkerClient.requestShared()` creates a `SharedArrayBuffer` for each output descriptor and copies data from WASM linear memory into it.
- Shared inputs cross the Worker boundary without transfer or detachment. Both Worker output modes still perform one copy between WASM linear memory and host memory.

## Batch execution

`batch` executes operations in order and stops at the first failure. A `{"$ref": N}` value resolves to the `shape` result of the zero-based `N`-th earlier operation.

```json
{
  "id": 3,
  "op": "batch",
  "args": {
    "scopeId": 1,
    "ops": [
      { "op": "makeBox", "args": { "size": [100, 60, 4] } },
      { "op": "makeCylinder", "args": { "radius": 8, "height": 4, "origin": [50, 30, 0] } },
      { "op": "booleanCut", "args": { "base": { "$ref": 0 }, "tools": [{ "$ref": 1 }] } }
    ]
  }
}
```

The batch is **not transactional**: shapes created before a failure remain owned by its scope. The outer protocol response remains successful; a nested failure is returned as `result.error` with `details.operation: "batch"`, `nestedOperation`, and (for exchange operations) `format`.

## Mesh layout and matrices

Tessellation uses these layouts:

- positions: `f32x3`
- normals: `f32x3`
- triangle indices: `u32`
- face groups: `u32x3` as `(faceIndex, indexStart, indexCount)`
- optional UVs: `f32x2`, aligned with positions

Every face has an isolated vertex range; consumers must not infer sharing between faces. Face-group offsets count scalar entries in `indices`, so one triangle contributes three entries. Edge groups use `(edgeIndex, vertexStart, vertexCount)`, with offsets measured in `f32x3` vertices. Face, edge, vertex, and batch indices are zero-based.

`matrix12` is the row-major array `[m11,m12,m13,m14,m21,m22,m23,m24,m31,m32,m33,m34]`. It maps a point to `(m11*x+m12*y+m13*z+m14, m21*x+m22*y+m23*z+m24, m31*x+m32*y+m33*z+m34)`.

For `transform`, `matrix` takes precedence and other transform components are ignored. Without `matrix`, composition is `T*R*S*M`, so points experience mirror, scale, rotation, then translation. `massProps.inertia` is row-major `[Ixx,Ixy,Ixz,Iyx,Iyy,Iyz,Izx,Izy,Izz]` in global model coordinates about the returned center.

## Capabilities, versions, and defaults

Public defaults are defined in `operation-contracts.json` under `semantics.defaults` and generated into `PROTOCOL_SEMANTICS` and `protocol.schema.json`'s `x-protocolSemantics`. Important defaults include:

- classification tolerance: `1e-7`;
- modeling and healing tolerance: `1e-6`;
- tessellation/STL deflection: `0.1` linear and `0.5` angular;
- origin: `(0,0,0)` and axis direction: `(0,0,1)`;
- history and UV output: false;
- polygon closure and transform copy: true; and
- omitted transform components: identity.

Exchange defaults are defined per operation: STEP/IGES use `mm`; STEP timestamps use `2026-01-01T00:00:00`; STL exports binary; PLY exports ASCII; glTF exports GLB; VRML exports version 2; native XCAF uses binary persistence; and optional mesh/document import results default off. Shape-based STL, OBJ, PLY, glTF, and VRML exporters share linear/angular deflection and relative-mode options. Buffer-based overloads ignore tessellation options.

## History and ShapeUpgrade

History is a local mapping for one operation, not persistent topological naming. Cross-operation naming and feature-tree references belong in the higher-level parametric SDK.

`historySupport` classifies every operation:

- `full`: booleans, `cylindricalHole`, `defeature`, `extrude`, `revolve`, `loft`, `sweepPipe`, `fillet`, `chamfer`, `sew`, `fixShape`, `unifySameDomain`, `transform`, `generalTransform`, `translate`, `rotate`, `scale`, and `mirror`;
- `partial`: `sweepPipeShell`, because OCCT maps the spine and profiles but not an auxiliary spine; and
- `unsupported`: operations without a reliable OCCT history source.

`shapeUpgrade` wraps the shape-level OCCT ShapeUpgrade algorithms. Its modes are `continuity`, `angle`, `area`, `closedFaces`, `closedEdges`, `convertToBezier`, `removeInternalWires`, and `removeLocations`.

- Continuity criteria are `c0` through `c3` or `cn`.
- Angles are in radians.
- Area division selects exactly one of maximum area, approximate part count, or fixed U/V counts.
- Internal-wire removal uses projected contour area.
- Location removal can start at the shape, compound, solid, shell, or face level.
- Divide and conversion modes share precision bounds, surface-segment mode, and edge selection (`0` free, `1` shared, `2` all).

Shape upgrade changes topology: input face and edge indices do not carry over, and no operation history is reported.

## Operation catalog

### Semantic modules

The current v1.2.0 operation set is grouped by semantic module below. The exact operation names and profile membership remain authoritative in [`protocol/modules.json`](../protocol/modules.json) and [`protocol/operations.json`](../protocol/operations.json).

| Semantic module | Scope |
| --- | --- |
| `runtime` | Capabilities, scopes, release, buffers, statistics, and batch execution |
| `geometry-topology` | Curves, surfaces, construction, approximation, editing, continuity, and tolerances |
| `topology-query` | Subshape catalogs, adjacency, type, identity, bounding boxes, and OBB |
| `modeling` | Primitives, sweeps, booleans' feature builders, offsets, fillets, chamfers, holes, and transforms |
| `algorithms` | Boolean/section analysis, projections, extrema, intersections, HLR, healing, and ShapeUpgrade |
| `tessellation` | Shape and edge tessellation plus existing triangulation access |
| `mesh` | Triangulation validation, replacement, and repair |
| `exchange-mesh` | STL, OBJ, PLY, glTF/GLB, and VRML mesh/document exchange |
| `step-shape-exchange` | BREP, STEP shape exchange, and format probing |
| `cad-document-exchange` | STEP/IGES XCAF documents and native XCAF persistence |

`capabilities` is authoritative for the exact operation set available in a loaded build. All
operation arguments and results are defined by `operation-contracts.json`; this page summarizes
the wire-level rules and does not replace those generated contracts.

## Queries, sections, and HLR

- `distance` reports all minimum-distance solutions, `innerSolution`, and vertex/edge/face support for both inputs. Support records use stable zero-based indices when the support belongs to that input and include edge parameters or face UV parameters when available.
- `middlePath` extracts the OCCT center path of a pipe-like shape between two of its face or wire subshapes.
- `sectionAnalysis` reports indexed section edges, lengths, ancestor faces on both inputs, isolated section vertices, optional curve approximation, and optional input pcurves.
- `projectHLR` uses the exact `TKHLR` B-Rep algorithm and returns scoped `visible` and `hidden` edge compounds. `direction` points from the camera toward the model; `up` fixes the view orientation. Parallel projection is the default. Perspective projection requires a positive `focus` distance from the Z=0 projection plane to the eye. Every result edge lies on Z=0 in projection coordinates.
- `getSubShape` returns a scope-owned handle for a zero-based indexed subshape.

Curve evaluation uses the native OCCT edge parameter domain and returns the point, first and second derivatives, plus optional tangent and curvature fields with availability flags. Surface evaluation uses the restricted face UV domain and returns derivative data plus the topologically oriented normal and signed principal, mean, and Gaussian curvatures when defined. Reversing a face reverses its normal and signed principal/mean curvatures; Gaussian curvature is unchanged.

## Curves, surfaces, and mesh operations

The BSpline editing operations `editCurveBSpline` and `editSurfaceBSpline` use explicit action
values for knot insertion/removal, degree elevation, periodicity conversion, direction
exchange/reversal, and complete control-net replacement. Indices are zero-based at the protocol
boundary; each operation returns a new scoped shape and leaves its input unchanged.

- `makeEdgeArc` accepts three points or a circle definition: `center`, `normal`, `radius`, `startAngle`, `endAngle`, and optional `xDirection`.
- Hyperbola and parabola construction requires explicit finite parameter limits because the natural OCCT domains are unbounded.
- `makeEdgeOffset` preserves the basis edge domain and follows OCCT's tangent/reference-direction cross-product convention.
- `makeEdgeBSpline` accepts JSON points or an `f64x3` input buffer. Interpolation is the default; `mode: "controlPoints"` accepts degree, knots, multiplicities, weights, and periodicity.
- `makeEdgeHelix` constructs an edge from a two-dimensional line on a cylindrical surface and supports both handedness values.
- Derived surfaces use finite input-edge or input-face topology domains. Surface offsets preserve the input outer and hole wires.

## Parametric CAD

The protocol exposes the serializable `ParametricModel` feature layer through the TypeScript
client. Expressions use named arithmetic without `eval`; feature dependencies are topologically
ordered and cycles are rejected. A recompute builds in a fresh scope and publishes atomically.
Feature definitions support primitives, profiles, sweeps, fillets, chamfers, hollow/offset/draft,
local features, holes, defeaturing, healing, transforms, booleans, section, and split. Bezier and
BSpline curve/surface control geometry may contain expressions for every pole coordinate, with
rational weights and the supported degree/knot/periodic fields.

Serialized output uses `schemaVersion: 1`; unversioned input is migrated to v1 and unknown future
versions are rejected. Features may be suppressed; diagnostics report `ok`, `suppressed`, or
`failed`, and a failed dependency prevents publication of a partial recompute. Face and edge
references persist geometry-family and parent-normalized-bound signatures. References may follow
an upstream `source: "id"` through the unique feature-history path; deleted, unmapped, and
one-to-many results are reported as `missing` or `ambiguous`. Strict matching is the default,
with explicit `disambiguation: "initialIndex"` and `allowGeometryReplacement` opt-ins for the
documented cases.

`solveSketch` accepts serializable 2D line, circle, arc, and BSpline entities, construction
geometry, geometric and dimensional constraints, and an arbitrary 3D sketch plane. It returns
convergence, residual, iteration, degree-of-freedom, and per-constraint diagnostics. BSpline
endpoints can participate in point-reference constraints; BSpline tangency is outside the bounded
v1 solver. Without profiles a sketch result is one wire; ordered profiles produce a compound and
stable `sketchId.profileId` wire references suitable for face outer and hole inputs.

## Exchange, cache, and errors

### Exchange and cache

All exchange operations use memory streams; they do not access the filesystem.

#### STEP and XCAF

- STEP `unit` is `"mm"` by default and also accepts `"cm"`, `"m"`, `"inch"`, or `"foot"`. It defines the model length represented by the kernel's dimensionless coordinates.
- `exportSTEP` writes coordinates in the selected unit and returns `data` with layout `step-utf8`. `importSTEP` converts the file's declared unit into the requested model unit and returns `shape` plus `rootCount`.
- `exportSTEPDocument` accepts an indexed tree of `part` and `assembly` nodes. Child nodes carry local `matrix12` transforms; nodes can carry names, RGBA colors, layers, visibility, physical material metadata, and validation properties.
- Material density must be non-negative. Omitted density metadata defaults to `kg/m^3` and `mass density`. Native XCAF retains every material field; STEP can omit `densityValueType`.
- Native `exportXCAF` and `importXCAF` preserve datums, linked geometric tolerances, and graphical presentations in binary and worker-generated XML.
- Worker-generated XML is standard XmlXCAF with a validated binary recovery marker. Memory import accepts only XML produced by `exportXCAF()`; convert third-party XmlXCAF to XCAF binary before import.
- XML export rejects node `color`, non-empty `subshapeStyles`, and SHUO `color`; use XCAF binary when those RGBA fields are required.
- `exportSTEPDocument` rejects datum and geometric-tolerance links that the pinned STEP transfer cannot preserve. GDT export requires exactly one document root.
- `importSTEPDocument` returns the occurrence tree, a scoped shape for each node, and one combined document shape. Native XCAF views also preserve named clipping-plane labels with origin, normal, and capping state.
- STEP exporters use the caller's `timestamp` or the protocol default; they do not read the clock. Exported bytes are outside the cross-host byte-determinism scope.

Native XCAF dimensions, datums, and geometric tolerances can include `presentation: { shape, name? }`. The shape is the graphical annotation topology stored on the OCAF presentation label. Imports return it as a scope-owned handle. Binary persistence retains that topology, including in the recovery marker used by worker-generated XML.

#### IGES

- IGES shape and XCAF document operations use the same explicit length units as STEP.
- `exportIGESDocument` and `importIGESDocument` preserve rooted `part`/`assembly` hierarchy, names, RGBA colors, layers, and occurrence transforms through a validated metadata marker.
- IGES cannot preserve `gdt`, `datums`, `geometricTolerances`, `views`, or `shuo`; document export rejects those fields.
- `exportIGES` accepts `mode: "faces"` (default) or `mode: "brep"`.
- STEP and IGES shape exporters accept one `shape` or multiple independent `shapes`. Import returns OCCT's combined `shape`, one scope-owned handle per transferred result in `shapes`, and `rootCount`.
- Shape-only IGES transfers geometry, not XCAF names, colors, layers, materials, or assembly relationships.

#### STL

- `exportSTL` accepts `encoding: "binary" | "ascii"`; binary is the default. ASCII output accepts a printable `solidName`, while binary output accepts an exact 80-byte `binaryHeader`.
- Shape export accepts `relative` to interpret `linearDeflection` relative to shape size. The indexed-mesh overload writes supplied triangles directly and ignores tessellation options.
- With `includeMesh: true`, import also returns per-facet normals and the matching name or header metadata. Imported STL becomes a triangulated TopoDS compound and does not recover analytic CAD surfaces.

#### OBJ

- OBJ accepts positive and negative position/UV/normal indices, independent slash-separated corner fields, seam expansion, and polygon fan triangulation.
- The default `importOBJ` result is shape-only and retains complete normals and UVs in face triangulations.
- With `includeDocument: true`, import also returns indexed buffers and a document. Primitive ranges preserve contiguous `o`, `g`, `usemtl`, and `s` state; `indexStart` and `indexCount` count scalar index entries. `materialLibraries` preserves every `mtllib` reference.
- Matching MTL `resources` populate ambient, diffuse, specular, and emissive colors; opacity; shininess; optical density; illumination model; and supported map references. A missing companion resource does not block geometry import or remove its library reference.
- Mesh `exportOBJ` accepts indexed buffers and document metadata, requires primitive ranges to partition the index buffer, and returns OBJ data plus named MTL resources. The shape-only overload remains available.

#### PLY

- PLY 1.0 import and export support ASCII, binary little-endian, and binary big-endian encodings.
- The shape path retains vertex normals and common UV properties: `s/t`, `u/v`, and `texture_u/texture_v`.
- With `includeMesh: true`, import also returns indexed positions, indices, normals, UVs, u8 RGBA vertex colors, and ordered `comment`/`obj_info` headers.
- `exportPLY` accepts ASCII, binary little-endian, and binary big-endian encoding. It writes the indexed mesh directly, rejects out-of-range or repeated indices and geometrically degenerate triangles, and produces output readable by the matching `importPLY` path.

#### glTF and GLB

- glTF operations support glTF 2.0 JSON and GLB; data-URI and URI-matched buffers; triangle lists, strips, and fans; node matrices or TRS; normalized or float `NORMAL`/`TEXCOORD_0`; sparse accessors; morph targets; four-influence skinning; animation sampling; and OCCT Z-up/glTF Y-up conversion.
- `animationIndex` and `animationTime` must be supplied together. Node `translation`, `rotation`, `scale`, and `weights` use `STEP`, `LINEAR`, or `CUBICSPLINE` interpolation before world transforms, morph evaluation, and skinning. Linear rotation uses normalized shortest-path quaternion interpolation; times outside the key range clamp to its endpoints.
- Invalid animation, sampler, accessor, channel, target, interpolation, count, and finite-value constraints are rejected.
- Explicit `morphWeights` override sampled node weights, which override static node weights and then mesh weights. Skinning uses `JOINTS_0`, `WEIGHTS_0`, joint world transforms, and optional inverse bind matrices after morph evaluation.
- The default `importGLTF` result is shape-only. With `includeDocument: true`, it also returns the unmodified source JSON, active scene and roots, evaluated Z-up buffers, primitive-instance ranges, resolved buffers and images, and an optional shape.
- Document export preserves standard metadata. JSON uses companion resources for external buffers and images; GLB folds buffers into the BIN chunk and keeps external images as resources.
- The TypeScript import boundary decodes `KHR_draco_mesh_compression` and `EXT_meshopt_compression` before kernel import. Document imports retain the original compressed JSON and source buffers.
- Unknown required geometry extensions are rejected. Mesh imports produce triangle-face TopoDS compounds; they do not recover analytic surfaces.

#### VRML

- VRML 1/2 import and export use memory streams and preserve the shape-only API by default.
- With `includeMesh: true`, `importVRML` also returns indexed positions, triangle indices, normals, optional UVs, and optional u8 RGBA vertex colors from the OCCT scene.
- `exportVRML` accepts the same indexed mesh attributes, but RGBA colors must be opaque because VRML stores RGB only. Shape export retains the tessellation options.

#### BREP cache

- BREP is a cache format bound to the exact running WASM identity, not a cross-version exchange format. The kernel emits bare BREP bytes and neither embeds nor validates that identity.
- For the official standalone runtime, store the SHA-256 from `docs/g0-build.json` with each cache entry.
- For an official isolated Profile, use its artifact SHA-256 from [`protocol/artifacts.json`](../protocol/artifacts.json).
- For the official shared runtime, the identity contains the SHA-256 values of both `shared-main.wasm` and `step-shape-exchange.side.wasm` from the same manifest.
- Reject an identity mismatch before calling `importBREP`.

### Operation-specific constraints

#### Fillet and chamfer

- Edge selection uses zero-based `edgeIndices` from the input shape's stable edge map. Both operations retain a scalar size for compatibility and accept aligned per-edge `radii` or `distances` arrays.
- Fillet `radius2` defines linear start/end evolution. `radiusLaw` or aligned `radiusLaws` define multipoint radius changes with strictly increasing relative parameters spanning 0 through 1.
- Chamfer two-distance mode accepts scalar `distance2` or aligned `distances2` together with aligned `referenceFaceIndices`. Each reference face must be adjacent to its edge and identifies the side for the first distance.
- Fillet supports analytic and BSpline faces. Algorithm failures return `FilletFailed`.

#### Operation history

- `history` is a local mapping for one operation. It is not persistent topological naming.
- Full mappings are available for booleans, cylindrical holes, defeaturing, extrusion, revolution, loft, pipe sweeps, pipe-shell spine/profile inputs, fillet/chamfer, sewing, shape repair, same-domain unification, and rigid, scale, mirror, and general affine transforms.
- An auxiliary pipe-shell spine has no OCCT history mapping, so `sweepPipeShell` is `partial`.
- Fillet/chamfer history reports retained subshapes, faces generated by OCCT from input edges, modified replacement faces, and deleted inputs. Generated and deleted classifications can both refer to one input edge.
- `capabilities` reports operations without a reliable history source as `unsupported`. Requesting `includeHistory: true` for one of those operations returns `InvalidArgs`.

#### Holes and defeaturing

`cylindricalHole` uses an explicit axis and supports `throughAll`, `throughNext`, `untilEnd`, `blind`, and `between` limits. Blind `length` and between `from`/`to` are axis parameters measured from `origin` in model units. `defeature` accepts zero-based face indices on a solid, compsolid, or compound of solids. Both operations provide full local face/edge history; this remains single-operation history rather than persistent naming.

#### BSpline and triangulation editing

- `editCurveBSpline` and `editSurfaceBSpline` expose native knot insertion/removal, degree elevation, periodicity, and control-net edits.
- `reduceCurveDegree` and `reduceSurfaceDegree` perform bounded OCCT BSpline approximation, return the new shape plus OCCT's `maxError`, and preserve the source parameter domain and surface boundary topology.
- `triangulationData` reads an existing face mesh without retessellation. `validateTriangulation` checks the documented finite-attribute, index, and degeneracy invariants.
- `replaceTriangulation` returns an independent face copy with caller-supplied positions, indices, normals, and UVs. `repairTriangulation` copies and remeshes a shape with OCCT BRepMesh and computes normals on the rebuilt faces.

#### Surface filling

`makeSurfaceFilling.constraints` uses zero-based boundary edge indices in wire order and face handles for `g1` or `g2` support-face constraints. Unspecified edges use C0, and the support face must contain an OCCT pcurve for the constrained edge.

#### Local features

- `localPrism` defaults to `length` mode. The `until`, `fromUntil`, `untilEnd`, `fromEnd`, and `thruAll` modes map directly to the corresponding `BRepFeat_MakePrism` limit operations; `until` and `from` are scoped limiting shape handles.
- `localRevolution` uses selected base faces and an explicit axis. Its `angle`, `until`, `fromUntil`, and `thruAll` modes map directly to `BRepFeat_MakeRevol`; positive or negative angle values are accepted within one full revolution.
- `linearForm` maps to `BRepFeat_MakeLinearForm` and creates a planar rib or slot from a profile wire, work plane, and one or two opposing thickness vectors. `direction1` is either zero or opposite to `direction`; `operation` selects fusion (`add`) or cutting (`cut`). The native linear form has no length/until limit mode.
- `revolutionForm` provides `BRepFeat_MakeRevolutionForm` rotational rib/slot construction. It takes a profile wire, plane, revolution axis, and non-negative `height1`/`height2`; at least one height must be non-zero. It has no limit-shape mode.
- `glue` provides explicit `BRepFeat_Gluer` imprinting. `faceBindings` pair zero-based face indices from `newShape` and `baseShape`; optional `edgeBindings` pair zero-based edge indices. At least one face binding is required. OCCT also matches bound-face edges when possible.

### Errors and cancellation

- VRML, STL, OBJ, PLY, and glTF import/export plus edge tessellation report normalized progress.
- Mesh exchange checks cancellation while parsing, serializing, and copying output buffers.
- Only operations listed by the host adapter as cooperatively cancellable may observe an `AbortSignal` after dispatch.
