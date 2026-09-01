# TypeScript API reference

[中文说明 / Chinese guide](api.zh-CN.md)

## API at a glance

| Use case | Public entry point | Reference |
| --- | --- | --- |
| In-process Node.js kernel | `DirectClient` and `ShapeScope` | This API reference |
| Worker-backed Node.js or browser kernel | `WorkerClient` | [Getting started](getting-started.md) and this API reference |
| Modeling, queries, mesh, and exchange operations | `ShapeScope` methods | [Capability matrix](capabilities.md) |
| Low-level messages, buffers, errors, and defaults | `request()` and protocol frames | [Protocol specification](protocol.md) |
| Wasmtime or custom host integration | Synchronous WebAssembly ABI | [Host support](hosts.md) |
| Parametric features and sketches | `ParametricModel` and feature definitions | This API reference |
| Shared Main/Side high-level compatibility | `SharedClient` + `EngineCompatClient` | This API reference |

For how these clients map to standalone full, shared Main/Side, and isolated Profile runtimes, including shape ownership and lifecycle, see the [runtime architecture](architecture.md).

## Clients and lifecycle

`DirectClient.create(wasm, imports?)` creates an in-thread kernel. `WorkerClient.create(factory, wasm)` creates a disposable worker-backed kernel with hard timeout recovery. Both initialize against the generated protocol version and expose query, tessellation, and export methods.

### Scopes and handles

- `beginScope()` returns a `ShapeScope`, which owns every shape created within it. End it explicitly with `await scope.end()` or via `await using`.
- `ShapeHandle` instances carry an epoch check and cannot be constructed from raw numbers or transferred to another/rebuilt kernel instance.
- `scope.batch()` is the typed batch entry point; it converts every top-level `shape` or `shapes` result into provenance-checked `ShapeHandle` values.

### Buffers and low-level requests

- `DirectClient.createBuffer()` exposes in-thread reusable input buffers; callers must reacquire `view()` after every WASM call because memory growth invalidates earlier views. Pass `{ bufferId }` to low-level protocol calls and release with `DirectClient.freeBuffer()`. `releaseAll()` invalidates all live direct buffers.
- The low-level `request()` method is explicitly unsafe: it accepts raw `u32` shape handles and buffer references, bypassing TypeScript ownership and epoch provenance. Normal application code should use the typed methods instead.

### Custom isolated kernels

To replace an isolated kernel, pass a custom `artifact` to `createWorkerProfileRuntime` and resolve it to the actual URL with `resolve` or `baseUrl`. `defaultBase` supplies a host-derived fallback when neither of those caller overrides is set. The path starts a Worker and performs the `capabilities` handshake, rejecting an incompatible protocol. When supplied, descriptor `sha256`, `protocolVersion`, `abiVersion`, and `buildFamily` are checked against the bytes or client build.

Generated profile client types apply only to the official profiles declared by `protocol/modules.json`. A custom artifact has no corresponding static capability narrowing; after loading it, read `runtime.request("capabilities", {})` and choose operations from the returned `ops`.

## Construction and modeling

Construction and modeling methods cover:

- primitives and vertex, polygon, wire, face, shell, solid, compound, and compsolid construction;
- bounded analytic faces and finite line, conic, offset, Bezier, BSpline, and helix edges;
- extrusion, revolution, ruled, offset, filling, trimmed, approximation, and extension surfaces;
- extrusion, revolution, loft, pipe sweeps, booleans, and General Fuse;
- draft, local prism/revolution, rib/slot, cylindrical-hole, and defeaturing operations; and
- offsets, repair, and transforms.

N-side filling uses C0 boundaries by default and accepts optional G1/G2 support-face constraints. Wire-trimmed faces preserve arbitrary outer and hole wires on an existing surface.

Method option types in `dist/client.d.ts` are the normative TypeScript signatures; operation names and history levels come from `protocol/operations.json`.

### Fillet and chamfer options

`fillet()` and `chamfer()` support scalar dimensions, per-edge arrays, and variable law profiles:

- **Scalar mode:** `scope.fillet(shape, edgeIndices, radius)` or `scope.chamfer(shape, edgeIndices, distance)`.
- **Per-edge arrays:** `radii` or `distances` aligned with `edgeIndices`, building contours with independent per-edge sizes.
- **Linear evolution (fillet):** `radius2` creates a linear transition from the first radius at the start of the contour to `radius2` at its end.
- **Multipoint evolution law (fillet):** `radiusLaw` applies one shared profile, while `radiusLaws` accepts one profile per selected edge. Every profile contains strictly increasing relative parameters from `0` to `1` with positive radii.
- **Two-distance mode (chamfer):** `distance2` or `distances2` accompanied by `referenceFaceIndices` aligned with `edgeIndices`. Each reference face identifies the side on which the first distance is measured and must be adjacent to the corresponding edge.

```ts
// Example: variable fillet and two-distance chamfer
const filleted = await scope.fillet(baseBox, [0, 1], 2.0, {
  radius2: 5.0,
  includeHistory: true,
});

const chamfered = await scope.chamfer(baseBox, [2], 3.0, {
  distance2: 6.0,
  referenceFaceIndices: [0],
});
```

### General Fuse cell selection

`selectGeneralFuseCells()` reruns the General Fuse algorithm from its input shapes and filters cells according to selection rules:

- `take`: zero-based array of input shape indices that the cell must originate from.
- `avoid`: zero-based array of input shape indices that the cell must not contain.
- `material`: non-zero integer material tag assigned to selected cells.
- `removeInternalBoundaries`: merges adjacent cells sharing identical non-zero material tags.
- `makeContainers`: creates wires, shells, or compsolids according to cell dimension.

### Local features and defeaturing

- `draftAngle()`: applies a direction, angle, and neutral plane to zero-based input face indices.
- `localPrism()`: performs a non-destructive `BRepFeat_MakePrism` add or cut using selected faces of the base shape. Numeric argument selects length mode; options select `until`, `fromUntil`, `untilEnd`, `fromEnd`, or `thruAll` with required limiting shapes.
- `localRevolution()`: corresponding `BRepFeat_MakeRevol` feature with axis, angle, until/from limits, or thru-all mode.
- `linearForm()`: native `BRepFeat_MakeLinearForm` planar rib/slot from a profile wire, plane, and primary/opposing thickness directions (`operation: "add"` or `"cut"`).
- `revolutionForm()`: native `BRepFeat_MakeRevolutionForm` rotational rib/slot with non-negative `height1`/`height2`.
- `cylindricalHole()`: cuts along an explicit axis with through-all, through-next, until-end, blind-length, or axis-interval limits.
- `defeature()`: removes selected zero-based faces from a solid, compsolid, or compound of solids and reconstructs surrounding geometry.
- `glue()`: exposes `BRepFeat_Gluer` for explicit local imprinting using paired face and edge bindings.

## Curves and surfaces

### Subshapes and curves

- `scope.getSubShape(shape, type, index)` returns a handle for the zero-based indexed subshape. The current scope owns the returned handle.
- `curveDomain(edge)` returns the native parameter interval as `first` and `last`, together with `periodic` and `period`.
- `curveGeometry(edge)` returns the analytic type and the fields available for that type.
- `curveContinuity(edge)` returns `c0`, `g1`, `c1`, `g2`, `c2`, `c3`, or `cn`.
- `evaluateCurve(edge, parameter)` returns `point`, `d1`, and `d2`. It returns `tangent` and `curvature` only when `tangentDefined` and `curvatureDefined` indicate that those values exist.
- `curveControlData(edge)` returns all Bezier or BSpline poles, weights, knots, multiplicities, and periodicity data.
- `updateCurvePole(edge, index, point)` copies the underlying curve, changes one zero-based pole, and returns a new edge without modifying the input. The new edge preserves the input parameter domain and topology orientation.
- `trimCurve()` creates a finite sub-range edge. `convertCurveToBSpline()` converts that finite edge rather than its potentially unbounded basis curve.

`makeEdgeHyperbola()` and `makeEdgeParabola()` require finite, increasing parameter limits, so the returned edge has a finite topology domain. `makeEdgeOffset()` preserves the basis edge domain and follows OCCT's tangent/reference-direction cross-product convention.

### Surfaces

- `surfaceDomain(face)` returns `uFirst`, `uLast`, `vFirst`, and `vLast`, plus the U/V periodicity flags and periods.
- `surfaceGeometry(face)` returns the analytic type and the fields available for that type.
- `surfaceContinuity(face)` returns the U and V continuity levels separately.
- `evaluateSurface(face, u, v)` returns the point, first and second U/V derivatives, and the mixed derivative. `normalDefined` and `curvatureDefined` indicate whether the oriented normal and curvature fields are present. Non-umbilic results can also include the principal directions.
- `makeSurfaceExtrusion()`, `makeSurfaceRevolution()`, and `makeSurfaceRuled()` construct finite faces from edge domains.
- `makeSurfaceOffset()` replaces the support surface while preserving the input face's outer and hole wires.
- `makeSurfaceBezier()` and `makeSurfaceBSpline()` use U-major `poles[u][v]` control grids with optional rational weights, knots, multiplicities, and periodicity.
- `makeSurfaceFilling()` constructs an N-side face from a closed wire. Boundaries are C0 by default; G1/G2 support faces and interior points are optional.
- `makeFaceOnSurface()` copies arbitrary outer and hole wires, projects missing pcurves, and trims an existing support surface without modifying the inputs.
- `surfaceControlData()` returns the complete control net. `updateSurfacePole()` copies and edits one pole, and `trimSurface()` creates a finite rectangular UV patch without modifying the input.
- `convertSurfaceToBSpline()` converts a finite analytic, Bezier, or BSpline face.
- `surfaceIsoCurve(face, "u" | "v", parameter)` creates scoped edges clipped by the face's outer boundary and holes. `"u"` fixes U; `"v"` fixes V.

### BSpline editing and boundary rules

- `editCurveBSpline()` applies native knot insertion/removal, degree elevation, periodicity conversion, or complete control-net replacement and returns a new BSpline edge.
- `editSurfaceBSpline()` provides the corresponding U/V knot, degree, periodicity, direction, and control-net operations while preserving face wires and pcurves.
- `reparameterizeCurve()` and `reparameterizeSurface()` apply exact affine knot-domain changes. Surface boundary pcurves are rebuilt for the new parameter domain.
- `reduceCurveDegree()` and `reduceSurfaceDegree()` approximate to requested lower degrees and return `{ shape, maxError }` while preserving the input domain and topology.
- The approximation methods accept `variationalSmoothing` length, curvature, and torsion weights for OCCT fairing. This mode uses `degreeMax`, continuity, and tolerance; it does not use parameterization or `degreeMin`.

`makeSurfaceFilling.constraints` uses zero-based boundary edge indices in wire order and a support face for `g1` or `g2` continuity. Unspecified edges remain C0. Duplicate or out-of-range indices and support faces without an edge pcurve are rejected. `trimSurface()` accepts one four-edge boundary and rejects arbitrary trimmed topology instead of discarding it.

## Queries and diagnostics

- `scope.middlePath()` extracts the OCCT center path of a pipe-like shape between two face or wire subshapes that belong to it.
- `inspectTolerances()` returns stable, zero-based face, edge, and vertex tolerance entries. `scope.setTolerance()` edits one or all topology levels on a deep copy and leaves the input unchanged.
- Projection, extrema, and intersection methods operate on finite topology and respect face holes and outer boundaries. Extrema set `parallel` for a continuous stationary solution and then return representative global-nearest pairs.
- `intersectCurveCurve()` and `intersectSurfaceSurface()` create scoped vertex or edge handles for isolated and one-dimensional results.
- `distance()` returns every minimum-distance solution, the inner-solid state, and each solution's vertex, edge, or face support. Edge parameters and face UV parameters are included when available. A support index can be absent when OCCT's inner solution refers to a shape that is not a subshape of that input.
- `scope.sectionAnalysis()` returns the scoped section shape, indexed section edges and lengths, optional ancestor-face indices on both inputs, and indexed isolated section vertices. It can also request curve approximation and input pcurves without changing `section()`.
- `scope.projectHLR()` runs exact B-Rep hidden-line removal without tessellation. It accepts a camera-to-model `direction`, an `up` direction, and parallel or perspective projection. Perspective projection also requires a positive `focus`. The reusable `visible` and `hidden` edge compounds use view coordinates: X points right, Y follows the projected up direction, and Z is zero.

## Mesh processing

### Tessellation and mesh repair

- `weldTessellation()` merges vertices within a position tolerance, preserves hard normal and UV seams by default, removes collapsed triangles, and rewrites `faceGroups`.
- `recalculateVertexNormals()` computes area-weighted normals.
- `generateTangents()` returns normalized `f32x4` tangent and handedness data for tessellations with UVs.
- `findBoundaryLoops()` returns deterministically ordered, consistently oriented indexed boundary loops.
- `fillPlanarHoles()` triangulates selected simple planar loops, appends a caller-selected face group, preserves positions and UVs, and rebuilds normals. It rejects non-manifold, branching, inconsistently oriented, non-planar, degenerate, and self-intersecting boundaries. It does not infer outer boundaries; pass `loopIndices` when an open surface contains both outer boundaries and holes.
- `FaceSelectionMap` maps rendered triangle indices to zero-based OCCT face indices and exposes the reverse triangle ranges. `EdgeSelectionMap` provides the equivalent mapping for edge-polyline vertices. Both validate range units, bounds, and overlap.
- `simplifyTessellation()` applies deterministic vertex-cluster simplification by `targetTriangles` or `ratio`, removes collapsed triangles, retains face-group ranges, and rebuilds area-weighted normals.
- `subdivideTessellation()` applies midpoint refinement for one or more levels, interpolates UVs, retains face-group ranges, and rebuilds normals.

The built-in weld, simplify, and subdivision operations rebuild `faceGroups` with output indices, so selection maps remain usable.

### Mesh exchange

- STL, OBJ, PLY, and VRML retain their shape-only import/export paths. Mesh or document options additionally return indexed buffers and format metadata.
- OBJ primitive ranges address scalar entries in `indices`.
- PLY output in `ascii`, `binary_little_endian`, and `binary_big_endian` encoding is accepted by the matching import path.
- The glTF document path retains source JSON, resolved resources, and primitive-instance ranges. A valid document without triangles can omit `shape`.

See [Exchange formats](#exchange-formats) and the [protocol specification](protocol.md) for each format's metadata boundaries and rejection rules.

## Modeling history

- `fillet()` and `chamfer()` return full local face/edge history with `includeHistory: true`: retained subshapes, generated faces, modified replacements, and deleted inputs. Generation and deletion are independent, so a deleted edge can still generate a blend or chamfer face.
- `extrude()`, `revolve()`, `loft()`, and `sweepPipe()` return the full local history provided by their OCCT builders.
- `sweepPipeShell()` maps its spine and profiles, but not an auxiliary spine; its overall history support is therefore partial.
- `sew()`, `fixShape()`, and `unifySameDomain()` return full history for healing workflows. `fixShape()` composes copy and repair history so mappings still refer to the caller's original shape.

## Parametric CAD

### Feature graph and serialization

`ParametricModel` adds a serializable feature layer over any `BaseClient`:

- expressions support named parameters and arithmetic without `eval`;
- feature dependencies are topologically ordered, and cycles are rejected;
- `recompute()` builds in a fresh scope and publishes results atomically;
- concurrent recomputes and disposal are serialized;
- serialized output uses `schemaVersion: 1`; `fromJSON()` migrates the original unversioned representation and rejects unknown future versions; and
- features can be marked `suppressed`; `getFeatureDiagnostics()` reports `ok`, `suppressed`, or `failed` for the last recompute.

### Persistent subshape references

Optional serializable `subshapeReferences` name a feature face or edge:

- `initialIndex` captures a persistent signature on the first successful recompute. Later recomputes ignore that index and uniquely match the geometry family plus parent-normalized subshape bounds.
- `source: "id"` follows an earlier reference through the unique feature-history path to the target feature. Booleans, cylindrical holes, defeaturing, extrusion, revolution, loft, pipe and pipe-shell sweeps, fillet, chamfer, sewing, shape repair, same-domain unification, and transforms contribute their available OCCT histories automatically.
- Pipe-shell auxiliary-spine topology is explicitly unmapped because OCCT does not expose its history. Deleted or unmapped sources become `missing`; one-to-many mappings become `ambiguous`.
- `disambiguation: "initialIndex"` selects by the original zero-based topology order when a symmetric equal-signature candidate set is intentional. Strict ambiguity remains the default.
- `allowGeometryReplacement: true` permits a deliberate curve- or surface-family replacement when the normalized boundary remains unique. Without this opt-in, a family change remains `missing`.
- Fillet, chamfer, hollow, draft, local prism/revolution, and defeature selection fields accept `{ reference: "id" }`. The resolved reference must belong to the feature input and have the required topology type.
- A missing or ambiguous reference fails the downstream feature without publishing a partial recompute.
- `getSubshapeReference()` returns the current scope-owned handle. `getSubshapeReferenceDiagnostics()` reports `resolved`, `missing`, or `ambiguous` plus candidate indices. `resolvePersistentSubshape(source, historyChain)` remains available for explicit history composition.

Signature matching is stable for dimension and uniform topology changes that preserve normalized placement. It is not a universal B-Rep naming algorithm for operations without OCCT history; initial-index disambiguation assumes stable topology order, and geometry replacement requires unique bounds.

### Feature definitions and assemblies

- Primitive feature definitions include box, cylinder, sphere, cone, torus, and wedge. Profile features include polygon and face.
- Modeling features include extrusion, revolution, loft, pipe and pipe-shell sweeps, fillet, chamfer, hollow, offsets, draft, local prism, cylindrical holes, defeaturing, translation, rotation, scale, mirror, general affine transforms, and boolean cut/fuse/common.
- An optional serializable `document` defines indexed part and assembly nodes. Repeated part nodes reference one feature as component instances and carry parameter-driven transforms, product names, RGBA colors, layers, visibility, physical and PBR materials, and validation properties.
- After `recompute()`, `toSTEPDocumentDefinition()` resolves feature IDs and transform expressions into the document definition accepted by `exportSTEPDocument()` and `exportXCAF()`.

### Freeform geometry and sketches

Parametric `bezierCurve`, `bsplineCurve`, `bezierSurface`, and `bsplineSurface` features serialize freeform control geometry and resolve expressions in every pole coordinate. Rational weights, BSpline degrees, knots, multiplicities, periodic flags, and construction tolerances are supported where the corresponding `ShapeScope` constructor supports them.

- `solveSketch()` accepts serializable 2D line, circle, arc, and BSpline entities. Geometric constraints are `coincident`, `fixed`, `horizontal`, `vertical`, `parallel`, `perpendicular`, `tangent`, `concentric`, `symmetry`, `equalLength`, and `equalRadius`; dimensional constraints are `distance`, `length`, and `radius`.
- Results include convergence, residual, iterations, degrees of freedom, and per-constraint residual diagnostics.
- Tangency applies to line, circle, and arc pairs. BSpline endpoints can participate in point-reference constraints, but BSpline tangency is outside this bounded solver. Unknown constraints and unsupported entity combinations are rejected.
- Construction entities participate in solving but are omitted from output. A parametric `sketch` resolves dimensional expressions and places the result on an arbitrary 3D plane.
- Without `profiles`, a sketch publishes one wire. Ordered profiles publish a compound and stable `sketchId.profileId` wire references suitable for `face.outer` and `face.holes`.

The parametric feature graph also supports sewing, shape repair, same-domain unification, section, and split operations.

## Shape upgrade

`ShapeScope.shapeUpgrade()` exposes these shape-level OCCT ShapeUpgrade modes:

- `continuity`: divide low-continuity geometry;
- `angle`: limit angular spans on revolution-like surfaces;
- `area`: divide faces by maximum area, approximate part count, or fixed U/V counts;
- `closedFaces` and `closedEdges`: open periodic topology;
- `convertToBezier`: convert selected 2D/3D curve and surface families;
- `removeInternalWires`: remove holes below a caller-specified area; and
- `removeLocations`: bake selected topology locations into geometry.

Divide and conversion modes accept precision bounds, surface segmentation, and the applicable edge mode (`0` free, `1` shared, `2` all). These operations may change topology and do not provide persistent topology history.

## Buffers and mesh boundaries

- Large BSpline pole arrays can use `Float64Array` in packed `f64x3` order.
- `tessellate()` returns face-isolated `Float32Array` positions and normals, `Uint32Array` indices and groups, and optional UVs.
- `triangulationData()` reads an existing face mesh without retessellating. `validateTriangulation()` reports attribute and triangle issues.
- `replaceTriangulation()` copies the face, applies caller-supplied positions, indices, normals, and UVs, and returns the new face without modifying the input.
- `repairTriangulation()` copies the shape and rebuilds its face meshes with OCCT BRepMesh and generated normals.
- Binary imports accept `ArrayBuffer` and `SharedArrayBuffer`. `WorkerClient` transfers and detaches ordinary input buffers; shared inputs remain caller-owned and cross the worker boundary without cloning.
- `WorkerClient.requestShared()` creates a `SharedArrayBuffer` for every kernel output buffer and copies data from WASM linear memory into it. The shared buffers are not transferred. This still performs one copy; it is not zero-copy access to WASM memory.
- Ordinary export and tessellation buffers are copied out and released before their promises resolve.

## Exchange formats

All exchange methods use memory streams.

### STEP and IGES shape exchange

- STEP and IGES accept `mm`, `cm`, `m`, `inch`, or `foot` as the model unit.
- STEP shape and document export accepts `schema: "AP203" | "AP214" | "AP242"`; the default is `AP242`. Export uses the caller's timestamp or the protocol default instead of reading the clock.
- `exportSTEP()` and `exportIGES()` accept one shape or an array of independent roots. Import returns `{ shape, shapes, rootCount }`: `shape` is OCCT's combined result, and `shapes` contains one scope-owned handle per transferred root.
- Shape-only IGES accepts `mode: "faces"` (the default) or `mode: "brep"`.
- `Matrix12` is the public transform type and uses the protocol's row-major 3x4 affine layout.

### XCAF documents

- `exportSTEPDocument()` and `ShapeScope.importSTEPDocument()` round-trip an indexed part/assembly tree. Nodes can carry local transforms, names, RGBA colors, layers, visibility, physical material metadata, validation properties, and scope-owned shapes.
- Material density must be non-negative. Omitted density metadata uses `kg/m^3` and `mass density`. Native XCAF retains every material field. STEP retains material name, description, density, and density name, but can omit `densityValueType`; it represents PBR visual material through presentation color only.
- Native `exportXCAF()` and `importXCAF()` preserve the complete public PBR field set in binary and Worker-generated XML. XCAF binary is required for node RGBA colors, subshape RGBA styles, colored SHUO paths, and third-party XmlXCAF input; XML export rejects those fields instead of silently reducing their color data.
- `subshapeStyles` selects a zero-based `face` or `edge` index using the same ordering as `getSubShape()`. Face styles map to XCAF surface colors, and edge styles map to curve colors. These are definition-level styles; per-occurrence overrides for repeated STEP instances require SHUO.
- Native XCAF `views` preserve named camera projection, directions, zoom/window, optional view-volume clipping, associated node indices, and clipping-plane labels with origin, normal, and capping state.
- Native XCAF `shuo` preserves component paths as node-index sequences from upper usage to next usage. Optional per-occurrence RGBA `color` requires binary persistence.
- Dimensions, datums, and geometric tolerances can include `presentation: { shape, name? }`. Import returns the annotation topology as a scope-owned `ShapeHandle`.
- Datum records preserve name, description, identification, optional semantic name, and the referenced part node. Geometric-tolerance records preserve type, value, optional value type, semantic name, linked `datumIndices`, `materialRequirement`, `zoneModifier`/`zoneModifierValue`, all public OCCT `modifiers`, and `maxValueModifier`.
- STEP transfer rejects links that it cannot preserve, and a STEP document containing GDT must have exactly one root. This API is a bounded metadata surface, not a general XCAF authoring API; the XML boundary is defined in the [protocol specification](protocol.md).
- `exportIGESDocument()` and `ShapeScope.importIGESDocument()` preserve rooted hierarchy, names, RGBA colors, layers, and occurrence transforms through the OCCT IGESCAF bridge. IGES document export rejects `gdt`, `datums`, `geometricTolerances`, `views`, and `shuo` because the format path cannot preserve them.

### STL, OBJ, PLY, and VRML

- `ShapeScope.importSTL(data)` returns a shape by default. With `{ includeMesh: true }`, it also returns indexed geometry, duplicated per-facet normals, and the ASCII `solidName` or exact binary 80-byte `binaryHeader`. `exportSTL()` accepts matching metadata and `encoding: "binary" | "ascii"`; shape export also accepts `relative`. STL does not preserve vertex sharing, UVs, materials, or hierarchy, and exported facet normals follow triangle winding.
- `ShapeScope.importOBJ(data)` returns a shape by default. With `{ includeDocument: true, resources }`, it also returns indexed geometry, corner normals and UVs, object/group/material/smoothing ranges, material-library references, and parsed MTL data. `exportOBJ(mesh, { materialLibrary })` returns OBJ data plus URI-to-`ArrayBuffer` companion resources. The shape-only overload remains available.
- `importPLY()` and `exportPLY()` support PLY 1.0 ASCII and both binary byte orders. The shape path retains vertex normals and common UV properties; `{ includeMesh: true }` also returns indexed geometry, u8 RGBA vertex colors, and ordered `comments`/`objectInfo`. Mesh export rejects invalid indices and degenerate triangles.
- `importVRML()` and `exportVRML()` support VRML 1/2. `{ includeMesh: true }` returns indexed geometry, normals, optional UVs, and opaque u8 RGBA colors. VRML stores RGB only, so mesh export rejects non-opaque alpha.

### glTF and GLB

- `importGLTF()` auto-detects glTF 2.0 JSON and GLB. It preserves float or normalized integer `NORMAL` and `TEXCOORD_0` data and resolves sparse accessors for positions, normals, UVs, indices, and morph targets.
- `{ includeDocument: true, resources }` preserves the source document, resolves buffers and images by exact URI, and returns active-scene mesh buffers plus primitive-instance ranges. `shape` is optional for a valid document without triangles. The document remains in glTF Y-up coordinates; returned mesh buffers and shapes use platform Z-up coordinates.
- `exportGLTF()` returns GLB by default. With `format: "gltf"`, shape export returns glTF JSON with an embedded buffer; tessellation export includes positions, normals, and UVs. The mesh-document overload accepts the imported `document`, resolved `buffers`, and `resources`; resolved source buffers are authoritative, and evaluated Z-up arrays are not re-encoded.
- JSON export preserves external buffer URIs as companion resources and regenerates data URIs. GLB export combines buffers into its BIN chunk and returns external images as companion resources. Standard scene nodes, instances, materials, textures, animations, skins, cameras, extensions, and extras remain in the source document.
- `animationIndex` and `animationTime` must be supplied together. Translation, rotation, scale, and weight channels support `STEP`, `LINEAR`, and `CUBICSPLINE`; out-of-range times clamp to an endpoint. Sampling validates references, layouts, counts, target paths, interpolation modes, and finite values before evaluation.
- Explicit `morphWeights` must match every imported primitive's target count and override sampled node weights, then static node weights, then mesh weights. Morph accessors can be sparse.
- Skinning supports four influences through `JOINTS_0`, `WEIGHTS_0`, joint world transforms, and optional inverse bind matrices. Joints can use unsigned-byte or unsigned-short components; weights can use float or normalized unsigned-byte/unsigned-short components. Imported normals are transformed with the skinned geometry.
- Sampling does not mutate the returned source `document`.
- The TypeScript import boundary decodes `KHR_draco_mesh_compression` and `EXT_meshopt_compression` with the pinned Draco and meshoptimizer runtimes before kernel import. Document import retains the original compressed JSON and buffers for round-trip export.

### BREP and format probing

- BREP is cache data bound to the exact running WASM identity. Store that identity with the bytes and reject mismatches before `importBREP()`; the standalone, isolated, and shared identity rules are defined by the [protocol specification](protocol.md#exchange-and-cache).
- `probeFormat()` recognizes BREP, STEP, IGES, STL, OBJ, PLY, glTF/GLB, VRML, and native XCAF binary/XML. It reports text or binary encoding and whether STEP assembly or presentation metadata is present.

## Errors, cancellation, and limits

- Errors are `KernelError` instances with a stable `code`; `details` is optional.
- A `KernelFailure` for a parsed operation includes `details.operation`. STEP, XCAF, IGES, STL, OBJ, PLY, glTF, VRML, and BREP failures also include the stable lowercase format in `details.format`.
- Malformed protocol frames and runtime failures can omit `details`.
- The low-level `request()` method accepts either a numeric timeout or `{ timeoutMs, signal, onProgress }`. The progress callback receives `{ operation, fraction }`, where `fraction` ranges from 0 to 1.
- Tessellation, edge tessellation, VRML, STL, OBJ, PLY, glTF, and STEP/IGES shape or XCAF document transfer connect progress to OCCT's `Message_ProgressIndicator`.
- Mesh exchange checks `AbortSignal` through a shared `WorkerClient` flag while parsing, serializing, and copying output buffers. OCCT-backed operations stop at their next progress checkpoint and keep the Worker and its handles alive.
- Aborting a queued request removes only that request. Other active synchronous Worker operations use hard cancellation: the client terminates and rebuilds the Worker, rejects the old queue, and expires prior handles. A timeout uses the same recovery path.

`DirectClient` calls are synchronous and do not promise hard timeout recovery. For untrusted CAD input, use `WorkerClient` so a timeout or non-cooperative cancellation can rebuild the Worker and expire its handles.
