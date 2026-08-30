# Generated from protocol/modules.json. Do not edit.

set(OCCT_PROFILE_IDS
  "core-modeling"
  "mesh"
  "exchange"
  "preview"
  "full-profile"
)

set(OCCT_SIDE_IDS
  "geometry-topology"
  "topology-query"
  "modeling"
  "algorithms"
  "tessellation"
  "mesh"
  "exchange-mesh"
  "step-shape-exchange"
  "cad-document-exchange"
)

set(OCCT_PROFILE_core_modeling_TARGET "profile-core-modeling")

set(OCCT_PROFILE_core_modeling_ARTIFACT "core-modeling.wasm")

set(OCCT_PROFILE_core_modeling_SOURCES
  "runtime/kernel.cpp"
  "runtime/dispatch.cpp"
  "runtime/shape_arena.cpp"
  "runtime/buffer_store.cpp"
  "runtime/protocol_helpers.cpp"
  "runtime/geometry_helpers.cpp"
  "runtime/history_helpers.cpp"
  "runtime/exchange_cad_helpers.cpp"
  "runtime/operation_registry.cpp"
  "runtime/protocol_dispatch.cpp"
  "runtime/protocol_bridge.cpp"
  "runtime/shape_transfer.cpp"
  "entry/full_entry.cpp"
  "operations/geometry_topology.cpp"
  "operations/geometry_curves.cpp"
  "operations/geometry_approximation.cpp"
  "operations/geometry_surfaces.cpp"
  "operations/geometry_surface_sweeps.cpp"
  "operations/geometry_surface_edits.cpp"
  "operations/topology.cpp"
  "operations/geometry_analysis.cpp"
  "operations/topology_query.cpp"
  "operations/modeling.cpp"
  "operations/modeling_fillet.cpp"
  "operations/modeling_transform.cpp"
  "operations/algorithms.cpp"
  "operations/analysis_algorithms.cpp"
  "operations/shape_upgrade.cpp"
)

set(OCCT_PROFILE_core_modeling_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKFeat"
  "TKOffset"
  "TKFillet"
  "TKBool"
  "TKPrim"
  "TKHLR"
)

set(OCCT_PROFILE_mesh_TARGET "profile-mesh")

set(OCCT_PROFILE_mesh_ARTIFACT "mesh.wasm")

set(OCCT_PROFILE_mesh_SOURCES
  "runtime/kernel.cpp"
  "runtime/dispatch.cpp"
  "runtime/shape_arena.cpp"
  "runtime/buffer_store.cpp"
  "runtime/protocol_helpers.cpp"
  "runtime/geometry_helpers.cpp"
  "runtime/history_helpers.cpp"
  "runtime/exchange_cad_helpers.cpp"
  "runtime/operation_registry.cpp"
  "runtime/protocol_dispatch.cpp"
  "runtime/protocol_bridge.cpp"
  "runtime/shape_transfer.cpp"
  "entry/full_entry.cpp"
  "operations/geometry_topology.cpp"
  "operations/geometry_curves.cpp"
  "operations/geometry_approximation.cpp"
  "operations/geometry_surfaces.cpp"
  "operations/geometry_surface_sweeps.cpp"
  "operations/geometry_surface_edits.cpp"
  "operations/topology.cpp"
  "operations/geometry_analysis.cpp"
  "operations/topology_query.cpp"
  "operations/modeling.cpp"
  "operations/modeling_fillet.cpp"
  "operations/modeling_transform.cpp"
  "operations/algorithms.cpp"
  "operations/analysis_algorithms.cpp"
  "operations/shape_upgrade.cpp"
  "operations/tessellation.cpp"
  "operations/mesh.cpp"
)

set(OCCT_PROFILE_mesh_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKFeat"
  "TKOffset"
  "TKFillet"
  "TKBool"
  "TKPrim"
  "TKHLR"
  "TKMesh"
)

set(OCCT_PROFILE_exchange_TARGET "profile-exchange")

set(OCCT_PROFILE_exchange_ARTIFACT "exchange.wasm")

set(OCCT_PROFILE_exchange_SOURCES
  "runtime/kernel.cpp"
  "runtime/dispatch.cpp"
  "runtime/shape_arena.cpp"
  "runtime/buffer_store.cpp"
  "runtime/protocol_helpers.cpp"
  "runtime/geometry_helpers.cpp"
  "runtime/history_helpers.cpp"
  "runtime/exchange_cad_helpers.cpp"
  "runtime/operation_registry.cpp"
  "runtime/protocol_dispatch.cpp"
  "runtime/protocol_bridge.cpp"
  "runtime/shape_transfer.cpp"
  "entry/full_entry.cpp"
  "operations/exchange_cad.cpp"
  "operations/exchange_step.cpp"
  "operations/exchange_probe.cpp"
  "operations/cad_document_exchange.cpp"
  "operations/exchange_document_export.cpp"
  "operations/exchange_document_writers.cpp"
  "operations/exchange_document_nodes.cpp"
  "operations/exchange_document_annotations.cpp"
  "operations/exchange_document_import.cpp"
  "operations/exchange_iges.cpp"
  "operations/exchange_mesh.cpp"
  "operations/exchange_obj_ply_gltf.cpp"
  "runtime/document_dispatch.cpp"
  "runtime/mesh_codecs.cpp"
  "runtime/obj_ply_codecs.cpp"
  "runtime/obj_ply_writers.cpp"
  "runtime/gltf_codecs.cpp"
  "runtime/gltf_animation.cpp"
  "runtime/gltf_import.cpp"
)

set(OCCT_PROFILE_exchange_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKDESTEP"
  "TKDE"
  "TKBinXCAF"
  "TKXmlXCAF"
  "TKBin"
  "TKBinL"
  "TKXml"
  "TKXmlL"
  "TKXCAF"
  "TKLCAF"
  "TKCAF"
  "TKCDF"
  "TKDEIGES"
  "TKBool"
  "TKPrim"
  "TKDEVRML"
  "TKRWMesh"
  "TKV3d"
  "TKService"
  "TKHLR"
  "TKDESTL"
  "TKMesh"
)

set(OCCT_PROFILE_preview_TARGET "profile-preview")

set(OCCT_PROFILE_preview_ARTIFACT "preview.wasm")

set(OCCT_PROFILE_preview_SOURCES
  "runtime/kernel.cpp"
  "runtime/dispatch.cpp"
  "runtime/shape_arena.cpp"
  "runtime/buffer_store.cpp"
  "runtime/protocol_helpers.cpp"
  "runtime/geometry_helpers.cpp"
  "runtime/history_helpers.cpp"
  "runtime/exchange_cad_helpers.cpp"
  "runtime/operation_registry.cpp"
  "runtime/protocol_dispatch.cpp"
  "runtime/protocol_bridge.cpp"
  "runtime/shape_transfer.cpp"
  "entry/full_entry.cpp"
  "operations/topology_query.cpp"
  "operations/tessellation.cpp"
  "operations/exchange_cad.cpp"
  "operations/exchange_step.cpp"
  "operations/exchange_probe.cpp"
)

set(OCCT_PROFILE_preview_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKMesh"
  "TKDESTEP"
  "TKDE"
)

set(OCCT_PROFILE_full_profile_TARGET "profile-full-profile")

set(OCCT_PROFILE_full_profile_ARTIFACT "full.wasm")

set(OCCT_PROFILE_full_profile_SOURCES
  "runtime/kernel.cpp"
  "runtime/dispatch.cpp"
  "runtime/shape_arena.cpp"
  "runtime/buffer_store.cpp"
  "runtime/protocol_helpers.cpp"
  "runtime/geometry_helpers.cpp"
  "runtime/history_helpers.cpp"
  "runtime/exchange_cad_helpers.cpp"
  "runtime/operation_registry.cpp"
  "runtime/protocol_dispatch.cpp"
  "runtime/protocol_bridge.cpp"
  "runtime/shape_transfer.cpp"
  "entry/full_entry.cpp"
  "operations/geometry_topology.cpp"
  "operations/geometry_curves.cpp"
  "operations/geometry_approximation.cpp"
  "operations/geometry_surfaces.cpp"
  "operations/geometry_surface_sweeps.cpp"
  "operations/geometry_surface_edits.cpp"
  "operations/topology.cpp"
  "operations/geometry_analysis.cpp"
  "operations/topology_query.cpp"
  "operations/modeling.cpp"
  "operations/modeling_fillet.cpp"
  "operations/modeling_transform.cpp"
  "operations/algorithms.cpp"
  "operations/analysis_algorithms.cpp"
  "operations/shape_upgrade.cpp"
  "operations/tessellation.cpp"
  "operations/mesh.cpp"
  "operations/exchange_mesh.cpp"
  "operations/exchange_obj_ply_gltf.cpp"
  "runtime/document_dispatch.cpp"
  "runtime/mesh_codecs.cpp"
  "runtime/obj_ply_codecs.cpp"
  "runtime/obj_ply_writers.cpp"
  "runtime/gltf_codecs.cpp"
  "runtime/gltf_animation.cpp"
  "runtime/gltf_import.cpp"
  "operations/exchange_cad.cpp"
  "operations/exchange_step.cpp"
  "operations/exchange_probe.cpp"
  "operations/cad_document_exchange.cpp"
  "operations/exchange_document_export.cpp"
  "operations/exchange_document_writers.cpp"
  "operations/exchange_document_nodes.cpp"
  "operations/exchange_document_annotations.cpp"
  "operations/exchange_document_import.cpp"
  "operations/exchange_iges.cpp"
)

set(OCCT_PROFILE_full_profile_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKFeat"
  "TKOffset"
  "TKFillet"
  "TKBool"
  "TKPrim"
  "TKHLR"
  "TKMesh"
  "TKDEVRML"
  "TKRWMesh"
  "TKV3d"
  "TKService"
  "TKXCAF"
  "TKLCAF"
  "TKDESTL"
  "TKDESTEP"
  "TKDE"
  "TKBinXCAF"
  "TKXmlXCAF"
  "TKBin"
  "TKBinL"
  "TKXml"
  "TKXmlL"
  "TKCAF"
  "TKCDF"
  "TKDEIGES"
)

set(OCCT_SIDE_geometry_topology_TARGET "shared-side-geometry-topology")

set(OCCT_SIDE_geometry_topology_ARTIFACT "geometry-topology.side.wasm")

set(OCCT_SIDE_geometry_topology_SOURCES
  "operations/geometry_topology.cpp"
  "operations/geometry_curves.cpp"
  "operations/geometry_approximation.cpp"
  "operations/geometry_surfaces.cpp"
  "operations/geometry_surface_sweeps.cpp"
  "operations/geometry_surface_edits.cpp"
  "operations/topology.cpp"
  "operations/geometry_analysis.cpp"
)

set(OCCT_SIDE_geometry_topology_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
)

set(OCCT_SIDE_topology_query_TARGET "shared-side-topology-query")

set(OCCT_SIDE_topology_query_ARTIFACT "topology-query.side.wasm")

set(OCCT_SIDE_topology_query_SOURCES
  "operations/topology_query.cpp"
)

set(OCCT_SIDE_topology_query_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
)

set(OCCT_SIDE_modeling_TARGET "shared-side-modeling")

set(OCCT_SIDE_modeling_ARTIFACT "modeling.side.wasm")

set(OCCT_SIDE_modeling_SOURCES
  "operations/modeling.cpp"
  "operations/modeling_fillet.cpp"
  "operations/modeling_transform.cpp"
)

set(OCCT_SIDE_modeling_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKFeat"
  "TKOffset"
  "TKFillet"
  "TKBool"
  "TKPrim"
)

set(OCCT_SIDE_algorithms_TARGET "shared-side-algorithms")

set(OCCT_SIDE_algorithms_ARTIFACT "algorithms.side.wasm")

set(OCCT_SIDE_algorithms_SOURCES
  "operations/algorithms.cpp"
  "operations/analysis_algorithms.cpp"
  "operations/shape_upgrade.cpp"
)

set(OCCT_SIDE_algorithms_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKHLR"
)

set(OCCT_SIDE_tessellation_TARGET "shared-side-tessellation")

set(OCCT_SIDE_tessellation_ARTIFACT "tessellation.side.wasm")

set(OCCT_SIDE_tessellation_SOURCES
  "operations/tessellation.cpp"
)

set(OCCT_SIDE_tessellation_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKMesh"
)

set(OCCT_SIDE_mesh_TARGET "shared-side-mesh")

set(OCCT_SIDE_mesh_ARTIFACT "mesh.side.wasm")

set(OCCT_SIDE_mesh_SOURCES
  "operations/mesh.cpp"
)

set(OCCT_SIDE_mesh_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
)

set(OCCT_SIDE_exchange_mesh_TARGET "shared-side-exchange-mesh")

set(OCCT_SIDE_exchange_mesh_ARTIFACT "exchange-mesh.side.wasm")

set(OCCT_SIDE_exchange_mesh_SOURCES
  "operations/exchange_mesh.cpp"
  "operations/exchange_obj_ply_gltf.cpp"
  "runtime/document_dispatch.cpp"
  "runtime/mesh_codecs.cpp"
  "runtime/obj_ply_codecs.cpp"
  "runtime/obj_ply_writers.cpp"
  "runtime/gltf_codecs.cpp"
  "runtime/gltf_animation.cpp"
  "runtime/gltf_import.cpp"
)

set(OCCT_SIDE_exchange_mesh_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKDEVRML"
  "TKRWMesh"
  "TKV3d"
  "TKService"
  "TKXCAF"
  "TKLCAF"
  "TKHLR"
  "TKDESTL"
  "TKBool"
  "TKPrim"
  "TKMesh"
)

set(OCCT_SIDE_step_shape_exchange_TARGET "shared-side-step-shape-exchange")

set(OCCT_SIDE_step_shape_exchange_ARTIFACT "step-shape-exchange.side.wasm")

set(OCCT_SIDE_step_shape_exchange_SOURCES
  "operations/exchange_cad.cpp"
  "operations/exchange_step.cpp"
  "operations/exchange_probe.cpp"
)

set(OCCT_SIDE_step_shape_exchange_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKDESTEP"
  "TKDE"
)

set(OCCT_SIDE_cad_document_exchange_TARGET "shared-side-cad-document-exchange")

set(OCCT_SIDE_cad_document_exchange_ARTIFACT "cad-document-exchange.side.wasm")

set(OCCT_SIDE_cad_document_exchange_SOURCES
  "operations/cad_document_exchange.cpp"
  "operations/exchange_document_export.cpp"
  "operations/exchange_document_writers.cpp"
  "operations/exchange_document_nodes.cpp"
  "operations/exchange_document_annotations.cpp"
  "operations/exchange_document_import.cpp"
  "operations/exchange_iges.cpp"
)

set(OCCT_SIDE_cad_document_exchange_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKBinXCAF"
  "TKXmlXCAF"
  "TKBin"
  "TKBinL"
  "TKXml"
  "TKXmlL"
  "TKXCAF"
  "TKLCAF"
  "TKCAF"
  "TKCDF"
  "TKDEIGES"
  "TKDESTEP"
  "TKDE"
  "TKBool"
  "TKPrim"
)

set(OCCT_SHARED_MAIN_SOURCES
  "runtime/kernel.cpp"
  "runtime/dispatch.cpp"
  "runtime/shape_arena.cpp"
  "runtime/buffer_store.cpp"
  "runtime/protocol_helpers.cpp"
  "runtime/geometry_helpers.cpp"
  "runtime/history_helpers.cpp"
  "runtime/exchange_cad_helpers.cpp"
  "runtime/operation_registry.cpp"
  "runtime/protocol_dispatch.cpp"
  "runtime/protocol_bridge.cpp"
  "runtime/shape_transfer.cpp"
  "entry/full_entry.cpp"
)

set(OCCT_SHARED_MAIN_TOOLKITS
  "TKXSBase"
  "TKBO"
  "TKShHealing"
  "TKTopAlgo"
  "TKGeomAlgo"
  "TKBRep"
  "TKGeomBase"
  "TKG3d"
  "TKG2d"
  "TKMath"
  "TKernel"
  "TKFeat"
  "TKOffset"
  "TKFillet"
  "TKBool"
  "TKPrim"
  "TKHLR"
  "TKMesh"
  "TKDEVRML"
  "TKRWMesh"
  "TKV3d"
  "TKService"
  "TKXCAF"
  "TKLCAF"
  "TKDESTL"
  "TKDESTEP"
  "TKDE"
  "TKBinXCAF"
  "TKXmlXCAF"
  "TKBin"
  "TKBinL"
  "TKXml"
  "TKXmlL"
  "TKCAF"
  "TKCDF"
  "TKDEIGES"
)
