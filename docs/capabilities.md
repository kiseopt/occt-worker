# Public CAD platform capability matrix

[中文说明 / Chinese guide](capabilities.zh-CN.md)

> For exact compile-time Profile-to-operation mappings and operation counts, see the generated [Profile capability matrix](profile-capabilities.generated.md).

This matrix describes the implemented public surface backed by the unmodified upstream OCCT commit pinned in `protocol/artifacts.json`. It does not claim to bind every non-visual OCCT class or toolkit. Rendering/UI toolkits (`TKV3d`, `AIS`, `TKOpenGl`, `TKMeshVS`), generic OCAF authoring, broader TKXMesh/TKExpress APIs, and pthread/TBB execution are outside the v1 public surface. Renderer-neutral tessellation remains public.

An item is listed as supported only when it:

1. is available through both the protocol and typed TypeScript API;
2. runs in the release WASM;
3. is documented; and
4. has integration coverage when it crosses a host boundary.

## Kernel and topology

- scoped generation-checked `TopoDS_Shape` handles, buffers, batch execution, validation, topology counts, indexed subshape catalogs, adjacency, scope-owned handles for indexed subshapes, and direct compound/shell/compsolid assembly.
- primitives including wedges and half-spaces, plus wire, face, shell, solid, compound, and compsolid construction.
- core booleans, section, non-destructive splitting, and General Fuse cell selection with source mapping, take/avoid/material rules, internal-boundary removal, and container construction.
- extrusion, revolution, loft, pipe sweeps, draft, local prism and revolution features, cylindrical holes, defeaturing, fillet, chamfer, hollow, offsets, sewing, healing, same-domain unification, transforms, and transform arrays. Local features expose their documented length, shape-limit, semi-infinite, thru-all, angle, and native hole modes.
- N-side surface filling from closed boundary wires with optional interior point constraints and G1/G2 support-face constraints, plus arbitrary outer/hole wire trimming on existing surfaces with generated pcurves.
- planar rib/slot features through native `BRepFeat_MakeLinearForm`, including one-sided or opposing thickness vectors and add/cut semantics.
- shape-level ShapeUpgrade operations for continuity, angular, area, closed-face, and closed-edge division; selective Bezier conversion; internal-wire removal; and location removal, with the native precision, segmentation, conversion, and edge-selection controls. Native planar and revolution rib/slot features are exposed through `linearForm` and `revolutionForm`, with explicit face/edge-bound `glue`/imprint workflows through `BRepFeat_Gluer`.
- chamfer exposes scalar and per-edge first/second distances with explicit reference-face control.
- fillet exposes scalar and per-edge radii, linear start/end evolution, and shared or per-edge multipoint radius laws.
- extrusion, revolution, loft, pipe sweeps, fillet/chamfer, sewing, shape repair, same-domain unification, and rigid, scale, mirror, and general affine transforms expose full local face/edge history. Results distinguish retained subshapes, generated or modified replacements, and deleted inputs.
- Pipe-shell sweep history is partial because OCCT does not map an auxiliary spine; its primary spine and profiles remain mapped.
- The parametric SDK composes available history across booleans, holes, defeaturing, sweeps, blends, repair, same-domain unification, and transforms. Other feature paths use deterministic signature matching and explicitly diagnose missing or ambiguous results.

## Curves and surfaces

- line, arc, circle, ellipse, hyperbola, parabola, offset, Bezier, BSpline, and cylindrical helix edge construction, with finite topology domains for unbounded conics.
- edge parameter domains; point, first/second derivative, tangent, and curvature evaluation.
- analytic curve type and parameter extraction, Bezier/BSpline metadata, and complete pole, knot, multiplicity, and weight data.
- non-mutating pole editing; native BSpline knot insertion/removal, degree elevation, bounded degree reduction, periodicity conversion, and batch control-net editing; global curve and U/V surface continuity; finite curve trimming; and exact curve-to-BSpline conversion.
- restricted face UV domains; point and first/second derivative evaluation; oriented normal and principal/mean/Gaussian curvatures.
- directly bounded analytic plane, cylinder, cone, sphere, and torus faces; finite linear-extrusion, revolution, ruled, and offset surface faces; analytic surface type/parameter extraction; topology-clipped U/V iso-curves.
- topology-bounded point projection onto curves/surfaces, curve/curve-surface/surface extrema, and topology-clipped curve-curve/curve-surface/surface-surface intersections. Parallel extrema are explicitly identified and return representative global-nearest solutions because the stationary set is continuous.
- finite rectangular surface trimming with explicit topology limits, topology-preserving conversion to BSpline, and non-mutating pole editing that returns a new face.
- direct U/V BSpline knot insertion/removal, degree elevation, bounded degree reduction, periodicity conversion, direction exchange/reversal, and batch control-net editing.
- exact affine curve/surface reparameterization with rebuilt boundary pcurves, plus arbitrary wire/pcurve trimming through `makeFaceOnSurface`.
- Bezier/BSpline surface construction; point-set BSpline approximation with optional OCCT variational fairing; bounded curve/surface extension; N-side filling with C0/G1/G2 constraints; and complete control data extraction.

## Parametric CAD

### Sketch coverage summary

| Area | Boundary |
| --- | --- |
| Line, circle, and arc entities | Geometric and dimensional constraints are supported; see the feature contract for the exact combinations. |
| BSpline entities | Endpoints participate in point-reference constraints; BSpline tangency is outside the bounded v1 solver. |
| Construction geometry | Participates in solving and is omitted from the published sketch output. |
| Solver diagnostics | Convergence, residual, iterations, degrees of freedom, and per-constraint residuals are returned. |

| Constraint group | Supported values |
| --- | --- |
| Geometric | `coincident`, `fixed`, `horizontal`, `vertical`, `parallel`, `perpendicular`, `tangent`, `concentric`, `symmetry`, `equalLength`, `equalRadius` |
| Dimensional | `distance`, `length`, `radius` |

- Serializable feature graphs support named arithmetic parameters, topological dependency ordering, cycle rejection, deterministic recompute, atomic result replacement, rollback on failure, and scoped cleanup.
- Features cover primitives, profiles, sweeps, blends, shelling, offsets, draft, local features, holes, defeaturing, repair, section, split, sewing, transforms, and booleans.
- Bezier and BSpline curve/surface control geometry accepts expressions, rational weights, and the supported degree, knot, multiplicity, and periodic options.
- Per-feature diagnostics report recompute and suppression state. A suppressed or failed dependency does not publish a partial result scope.
- Serialization writes schema v1, migrates the original unversioned form, and rejects unknown future versions.
- Serializable face and edge references persist geometry-family and parent-normalized-bound signatures. They can follow available OCCT history across the unique feature path, including multi-input operations, and explicitly report deleted, unmapped, or one-to-many results.
- Pipe-shell auxiliary-spine topology is explicitly unmapped because OCCT exposes no history for it. Strict geometry matching and ambiguity rejection remain the defaults; documented cases can opt into `disambiguation: "initialIndex"` or `allowGeometryReplacement`.
- References can select topology for downstream fillet, chamfer, hollow, draft, local prism/revolution, and defeature features. Operations without usable OCCT history use deterministic signatures and report ambiguity instead of claiming universal B-Rep naming.
- Sketches support serializable line, circle, arc, and BSpline entities; construction geometry; the geometric and dimensional constraints listed above; arbitrary planes; solver diagnostics; and sketch-to-feature references. BSpline endpoint constraints are supported, while BSpline tangency is outside the bounded solver.
- Ordered multi-profile sketches return a compound and stable `sketchId.profileId` wire references for outer boundaries, holes, and downstream solid features.
- Serializable assemblies support reusable feature instances, parameter-driven transforms, product hierarchy, names, colors, layers, visibility, physical and PBR materials, and validation properties through the STEP/XCAF document model.

## Data exchange

### Format summary

| Format | Shape path | Mesh or document path | Key boundary |
| --- | --- | --- | --- |
| BREP | Import/export | Cache only | Bind every entry to the exact WASM artifact hash. |
| STEP | Shape import/export | XCAF document import/export | Explicit model units; AP203/AP214/AP242; document links that STEP cannot preserve are rejected. |
| IGES | Shape import/export | XCAF document import/export | Root hierarchy and presentation metadata use the documented marker; unsupported semantic annotations are rejected. |
| STL | Triangulated shape import/export | Indexed mesh import/export | No UV, material, or hierarchy preservation. |
| OBJ | Shape import/export | Indexed mesh/document import/export | Corner attributes and optional MTL companion resources are preserved. |
| PLY | Triangulated shape import/export | Indexed mesh/document import/export | ASCII and both binary endian modes; colors and header metadata are opt-in. |
| glTF/GLB | Triangulated shape import/export | Scene/document import/export | JSON/GLB packaging, resources, and standard scene metadata are preserved within the documented limits. |
| VRML | Shape import/export | Indexed mesh import/export | VRML stores RGB only; alpha input must be opaque. |

#### BREP

- Import and export are available as memory operations, but BREP is a cache format bound to the exact running WASM identity.

#### STEP, IGES, and XCAF

- Shape-only STEP and IGES support explicit model units, multiple independent roots, one scope-owned handle per imported root, and the combined OCCT result. STEP export uses a caller-supplied or protocol-default timestamp.
- STEP XCAF supports rooted assembly trees, repeated shape instances, local transforms, names, RGBA colors, layers, visibility, physical material metadata, validation properties, zero-based face/edge presentation styles, and geometry handles. STEP maps PBR material to presentation color and requires one document root for GDT.
- Native XCAF binary additionally preserves named-camera `views`, clipping planes, component-path `shuo` records, datum links, graphical annotation topology, geometric tolerances, and the complete public PBR fields.
- Worker-generated XmlXCAF includes a validated binary recovery marker. Node colors, subshape styles, colored SHUO, and third-party XmlXCAF input require binary persistence. STEP export rejects datum and tolerance links that its transfer path cannot preserve.
- IGES XCAF preserves rooted hierarchy, names, RGBA colors, layers, and occurrence transforms through a validated metadata marker. It rejects `gdt`, `datums`, `geometricTolerances`, `views`, and `shuo`.

#### Mesh formats

- STL supports binary and ASCII shape or indexed-mesh exchange. Optional mesh import returns indexed geometry, per-facet normals, ASCII solid names, and exact binary headers. STL does not preserve UVs, materials, hierarchy, or analytic surfaces.
- OBJ supports negative indices, independent corner attributes, seam expansion, polygon triangulation, object/group/material/smoothing ranges, material-library references, and supported MTL companion data. Shape-only and symmetric mesh-document paths are both available.
- PLY 1.0 supports ASCII and both binary byte orders. The shape path retains normals and common UV names; the indexed mesh-document path also preserves u8 RGBA vertex colors and ordered `comment`/`obj_info` metadata.
- glTF 2.0 JSON and GLB support embedded or companion buffers, scene hierarchy and instances, normals, UVs, sparse accessors, morph targets, four-influence skinning, selected-animation sampling, and Z-up/Y-up conversion. Document import/export preserves standard JSON metadata, resolved resources, and primitive-instance ranges; valid documents can omit triangles. The TypeScript layer decodes Draco and meshoptimizer compression before kernel import.
- VRML 1/2 supports shape and indexed-mesh exchange. Mesh results include indexed geometry, normals, UVs, and opaque u8 RGBA colors within VRML's format limits.

#### Shared exchange behavior

- `probeFormat()` recognizes BREP, STEP, IGES, STL, OBJ, PLY, glTF/GLB, VRML, and native XCAF binary/XML, including encoding and STEP document-metadata detection.
- Every format has named TypeScript options and generated protocol contracts for its encodings, units, document/resource behavior, tessellation controls, and defaults.
- Shape and document STEP/IGES, VRML, edge tessellation, and shape/mesh STL, OBJ, PLY, and glTF report normalized progress and cooperatively observe `AbortSignal` in `WorkerClient`. Mesh exchange checks cancellation while parsing, serializing, and copying output buffers.
- Exchange `KernelFailure` responses provide stable `details.operation` and `details.format`; malformed requests and runtime failures can omit `details`.

## Mesh processing

- face-isolated solid tessellation with normals, indices, face groups, and optional UVs; edge polyline tessellation with topology groups.
- STL, OBJ, PLY, glTF, and GLB mesh import as valid triangulated TopoDS shapes.
- direct access to an existing face triangulation and non-mutating replacement of face positions, indices, normals, and UVs.
- triangulation validation for missing/empty meshes, non-finite attributes, invalid indices, and degenerate triangles; OCCT BRepMesh rebuild on a copied shape with generated normals.
- typed-array mesh welding with position tolerance and optional normal/UV seam preservation, area-weighted vertex-normal regeneration, and UV tangent generation while retaining face-group index ranges.
- deterministic grid-cluster mesh simplification by target triangle count or ratio, with rebuilt normals and retained face-group ranges.
- midpoint triangle subdivision/refinement for up to eight levels, with retained face-group ranges, interpolated UVs, and rebuilt normals.
- deterministic indexed boundary-loop extraction and bounded planar-hole filling with explicit rejection of non-manifold, branching, inconsistently oriented, non-planar, degenerate, and self-intersecting boundaries.
- topology-aware selection maps from rendered triangle and polyline-vertex indices back to zero-based OCCT face/edge indices, including reverse ranges and preservation through built-in weld, simplify, and subdivision operations.
- OBJ and glTF provide symmetric mesh-document import/export with format metadata and companion resources.
- PLY preserves positions, normals, UVs, u8 RGBA colors, and ordered global header metadata. VRML preserves indexed positions, normals, UVs, and opaque u8 RGBA colors. STL preserves facet normals, ASCII solid names, and exact binary headers.
- These guarantees apply within each format's inherent limits. STL and PLY indexed exports reject out-of-range or repeated indices, non-finite data, and geometrically degenerate triangles.

## Queries and runtime

- bounding boxes, mass properties, distance, point classification, validity, shape identity, topology type, resource statistics, workers, timeout recovery, and Node/browser hosts.
- oriented bounding boxes, topology-aware geometric projections/intersections, stationary extrema with parallel-state reporting, and detailed `BRepCheck` shape diagnostics.
- indexed face/edge/vertex tolerance inspection and non-mutating tolerance editing by topology level; edits return a copied shape and leave the input unchanged.
- all minimum-distance solutions with inner-solid state, stable topology support indices, edge parameters, and face UV parameters.
- pipe-like shape center-path extraction between face or wire sections.
- section edge lengths, two-sided ancestor-face mapping, isolated contact vertices, intersection approximation, and optional input pcurves.
- exact B-Rep hidden-line removal through `TKHLR`, with parallel or perspective camera projection and reusable visible/hidden edge compounds in view coordinates.
- a separate artifact translates the pinned Emscripten Phase 3 wasm-EH instructions to standardized exception references, and Wasmtime 47 runs scope creation, primitive construction, boolean modeling, bounding-box queries, tessellation, structured errors, and resource cleanup through the frozen ABI. The Node release-wasm suite remains the exhaustive operation test; host tests validate their host boundary and representative kernel paths.
- tessellation, edge tessellation, VRML, mesh exchange, and shape/XCAF STEP and IGES transfers expose progress and cooperative cancellation through a one-word `SharedArrayBuffer`.
- `WorkerClient.requestShared()` allocates shared output buffers and copies data from WASM linear memory. Shared binary and mesh inputs cross the Worker boundary without detachment.
- pthread/TBB builds remain outside the single-thread v1 target; browser parallelism uses multiple Workers and arenas.
