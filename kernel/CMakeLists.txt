set(OCCT_INCLUDE_DIR "${OCCT_ROOT}/include/opencascade")
set(OCCT_LIB_DIR "${OCCT_ROOT}/lib")

file(READ "${PROJECT_SOURCE_DIR}/protocol/modules.json" MODULE_TOPOLOGY_JSON)
string(JSON SIDE_COUNT LENGTH "${MODULE_TOPOLOGY_JSON}" artifactModuleCandidates)
math(EXPR SIDE_LAST "${SIDE_COUNT} - 1")
set(GROUP_KEYS "")
set(SIDE_ARTIFACT_KEYS "")
foreach(SIDE_INDEX RANGE ${SIDE_LAST})
  string(JSON SIDE_ARTIFACT MEMBER "${MODULE_TOPOLOGY_JSON}" artifactModuleCandidates ${SIDE_INDEX})
  string(JSON SIDE_MODULE_COUNT LENGTH "${MODULE_TOPOLOGY_JSON}" artifactModuleCandidates "${SIDE_ARTIFACT}")
  if(SIDE_MODULE_COUNT LESS 1)
    message(FATAL_ERROR "${SIDE_ARTIFACT} must own at least one semantic module")
  endif()
  string(REGEX REPLACE "\\.side\\.wasm$" "" SIDE_TARGET "${SIDE_ARTIFACT}")
  string(REPLACE "-" "_" SIDE_TARGET_KEY "${SIDE_TARGET}")
  list(APPEND SIDE_ARTIFACT_KEYS "${SIDE_TARGET}")
  set(SIDE_GROUPS_${SIDE_TARGET_KEY} "")
  foreach(SIDE_MODULE_INDEX RANGE ${SIDE_MODULE_COUNT})
    if(SIDE_MODULE_INDEX LESS SIDE_MODULE_COUNT)
      string(JSON SIDE_MODULE GET "${MODULE_TOPOLOGY_JSON}" artifactModuleCandidates "${SIDE_ARTIFACT}" ${SIDE_MODULE_INDEX})
      string(REPLACE "-" "_" SIDE_GROUP "${SIDE_MODULE}")
      list(APPEND GROUP_KEYS "${SIDE_GROUP}")
      list(APPEND SIDE_GROUPS_${SIDE_TARGET_KEY} "${SIDE_GROUP}")
    endif()
  endforeach()
endforeach()
list(REMOVE_DUPLICATES GROUP_KEYS)

set(GEOMETRY_TOPOLOGY_SOURCES
  operations/geometry_topology.cpp
  operations/geometry_curves.cpp
  operations/geometry_approximation.cpp
  operations/geometry_surfaces.cpp
  operations/geometry_surface_sweeps.cpp
  operations/geometry_surface_edits.cpp
  operations/topology.cpp
  operations/geometry_analysis.cpp
)
set(GROUP_SOURCES_geometry_topology ${GEOMETRY_TOPOLOGY_SOURCES})
set(MODELING_SOURCES operations/modeling.cpp operations/modeling_fillet.cpp operations/modeling_transform.cpp)
set(GROUP_SOURCES_modeling ${MODELING_SOURCES})
set(ALGORITHMS_SOURCES
  operations/algorithms.cpp
  operations/analysis_algorithms.cpp
  operations/shape_upgrade.cpp
)
set(GROUP_SOURCES_algorithms ${ALGORITHMS_SOURCES})
set(GROUP_SOURCES_mesh operations/mesh.cpp)
set(EXCHANGE_MESH_SOURCES
  operations/exchange_mesh.cpp
  operations/exchange_obj_ply_gltf.cpp
)
set(GROUP_SOURCES_exchange_mesh ${EXCHANGE_MESH_SOURCES})
set(EXCHANGE_CAD_SOURCES
  operations/exchange_cad.cpp
  operations/exchange_document_export.cpp
  operations/exchange_document_writers.cpp
  operations/exchange_document_nodes.cpp
  operations/exchange_document_annotations.cpp
  operations/exchange_document_import.cpp
  operations/exchange_probe.cpp
  operations/exchange_iges.cpp
  operations/exchange_step.cpp
)
set(GROUP_SOURCES_exchange_cad ${EXCHANGE_CAD_SOURCES})

set(OCCT_TOOLKITS
  TKBinXCAF TKXmlXCAF TKBin TKBinL TKXml TKXmlL
  TKDEVRML TKRWMesh TKV3d TKService TKXCAF TKLCAF TKCAF TKCDF TKHLR
  TKDESTL TKDEIGES TKDESTEP TKXSBase TKDE TKFeat TKOffset TKFillet TKBO TKBool TKPrim TKMesh TKShHealing TKTopAlgo TKGeomAlgo TKBRep TKGeomBase TKG3d TKG2d TKMath TKernel
)

function(kernel_common_setup theTarget)
  target_compile_features(${theTarget} PRIVATE cxx_std_17)
  target_include_directories(${theTarget} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/vendor/json/single_include"
    "${OCCT_INCLUDE_DIR}"
  )
  target_compile_options(${theTarget} PRIVATE
    -fwasm-exceptions
    -ffp-contract=off
    "-ffile-prefix-map=${PROJECT_SOURCE_DIR}=."
  )
endfunction()

option(KERNEL_BUILD_SHARED "Build shared Main + Side targets instead of standalone full" OFF)
option(KERNEL_BUILD_PROFILES "Build isolated standalone Profile targets" OFF)

set(CORE_RUNTIME_SOURCES
  runtime/kernel.cpp
  runtime/dispatch.cpp
  runtime/shape_arena.cpp
  runtime/buffer_store.cpp
  runtime/protocol_helpers.cpp
  runtime/geometry_helpers.cpp
  runtime/history_helpers.cpp
  runtime/exchange_cad_helpers.cpp
  runtime/operation_registry.cpp
  runtime/protocol_dispatch.cpp
  runtime/protocol_bridge.cpp
  runtime/shape_transfer.cpp
)
set(EXCHANGE_MESH_RUNTIME_SOURCES
  runtime/document_dispatch.cpp
  runtime/mesh_codecs.cpp
  runtime/obj_ply_codecs.cpp
  runtime/obj_ply_writers.cpp
  runtime/gltf_codecs.cpp
  runtime/gltf_animation.cpp
  runtime/gltf_import.cpp
)
list(APPEND GROUP_SOURCES_exchange_mesh ${EXCHANGE_MESH_RUNTIME_SOURCES})

function(standalone_kernel_target theTarget)
  add_executable(${theTarget} ${ARGN} ${CORE_RUNTIME_SOURCES} entry/full_entry.cpp)
  kernel_common_setup(${theTarget})
  target_link_directories(${theTarget} PRIVATE "${OCCT_LIB_DIR}")
  target_link_libraries(${theTarget} PRIVATE ${OCCT_TOOLKITS})
  set_target_properties(${theTarget} PROPERTIES SUFFIX ".wasm")
  target_link_options(${theTarget} PRIVATE
    --no-entry
    -fwasm-exceptions
    -ffp-contract=off
    -sSTANDALONE_WASM
    -sALLOW_MEMORY_GROWTH=1
    -sMAXIMUM_MEMORY=2147483648
    -sSTACK_SIZE=5242880
    -sFILESYSTEM=0
    -sENVIRONMENT=web,worker,node
    -sEXPORTED_FUNCTIONS=_k_alloc,_k_free,_k_handle,_k_response_ptr,_k_buffer_ptr,_k_buffer_len
  )
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${theTarget} PRIVATE -O3)
    target_link_options(${theTarget} PRIVATE -O3 -flto)
  else()
    target_compile_options(${theTarget} PRIVATE -O0 -g3)
    target_link_options(${theTarget} PRIVATE -O0 -g3 -sASSERTIONS=2)
  endif()
endfunction()

if(KERNEL_BUILD_PROFILES)
  string(JSON PROFILE_COUNT LENGTH "${MODULE_TOPOLOGY_JSON}" profiles)
  math(EXPR PROFILE_LAST "${PROFILE_COUNT} - 1")
  foreach(PROFILE_INDEX RANGE ${PROFILE_LAST})
    string(JSON PROFILE_ID MEMBER "${MODULE_TOPOLOGY_JSON}" profiles ${PROFILE_INDEX})
    string(JSON PROFILE_ARTIFACT_COUNT LENGTH "${MODULE_TOPOLOGY_JSON}" profiles "${PROFILE_ID}" artifacts)
    math(EXPR PROFILE_ARTIFACT_LAST "${PROFILE_ARTIFACT_COUNT} - 1")
    set(PROFILE_SOURCES "")
    foreach(PROFILE_ARTIFACT_INDEX RANGE ${PROFILE_ARTIFACT_LAST})
      string(JSON PROFILE_SIDE GET "${MODULE_TOPOLOGY_JSON}" profiles "${PROFILE_ID}" artifacts ${PROFILE_ARTIFACT_INDEX})
      string(JSON PROFILE_MODULE_COUNT LENGTH "${MODULE_TOPOLOGY_JSON}" artifactModuleCandidates "${PROFILE_SIDE}")
      foreach(PROFILE_MODULE_INDEX RANGE ${PROFILE_MODULE_COUNT})
        if(PROFILE_MODULE_INDEX LESS PROFILE_MODULE_COUNT)
          string(JSON PROFILE_MODULE GET "${MODULE_TOPOLOGY_JSON}" artifactModuleCandidates "${PROFILE_SIDE}" ${PROFILE_MODULE_INDEX})
          string(REPLACE "-" "_" PROFILE_GROUP "${PROFILE_MODULE}")
          list(APPEND PROFILE_SOURCES ${GROUP_SOURCES_${PROFILE_GROUP}})
        endif()
      endforeach()
    endforeach()
    standalone_kernel_target(profile-${PROFILE_ID} ${PROFILE_SOURCES})
  endforeach()
  return()
endif()

if(NOT KERNEL_BUILD_SHARED)
  set(FULL_SOURCES "")
  foreach(GROUP ${GROUP_KEYS})
    list(APPEND FULL_SOURCES ${GROUP_SOURCES_${GROUP}})
  endforeach()
  standalone_kernel_target(occt-worker ${FULL_SOURCES})
  return()
endif()

# ---------------------------------------------------------------------------
# Shared Main + Side targets (stage 4).
# ---------------------------------------------------------------------------

# Side modules first: their import closure drives the Main export manifest.
foreach(SIDE_TARGET ${SIDE_ARTIFACT_KEYS})
  string(REPLACE "-" "_" SIDE_TARGET_KEY "${SIDE_TARGET}")
  set(SIDE_SOURCES "")
  foreach(GROUP ${SIDE_GROUPS_${SIDE_TARGET_KEY}})
    list(APPEND SIDE_SOURCES ${GROUP_SOURCES_${GROUP}})
  endforeach()
  add_executable(shared-side-${SIDE_TARGET} ${SIDE_SOURCES})
  kernel_common_setup(shared-side-${SIDE_TARGET})
  target_compile_definitions(shared-side-${SIDE_TARGET} PRIVATE OCCT_WORKER_SHARED_SIDE)
  target_compile_options(shared-side-${SIDE_TARGET} PRIVATE
    -fPIC -fvisibility=hidden -fvisibility-inlines-hidden)
  set_target_properties(shared-side-${SIDE_TARGET} PROPERTIES SUFFIX ".wasm")
  target_link_options(shared-side-${SIDE_TARGET} PRIVATE
    --no-entry
    -fwasm-exceptions
    -ffp-contract=off
    -fPIC
    -Wl,-Bsymbolic
    -sSIDE_MODULE=2
    -sENVIRONMENT=web,worker,node
  )
endforeach()

find_program(NODE_BIN node REQUIRED)
set(MAIN_EXPORTS_FILE "${CMAKE_CURRENT_BINARY_DIR}/main-exports")
set(SIDE_ARTIFACTS "")
foreach(SIDE_TARGET ${SIDE_ARTIFACT_KEYS})
  list(APPEND SIDE_ARTIFACTS "${CMAKE_CURRENT_BINARY_DIR}/shared-side-${SIDE_TARGET}.wasm")
 endforeach()
add_custom_command(
  OUTPUT "${MAIN_EXPORTS_FILE}.txt" "${MAIN_EXPORTS_FILE}.lld.rsp"
  COMMAND "${NODE_BIN}" "${PROJECT_SOURCE_DIR}/scripts/generate-main-exports.mjs"
          "${MAIN_EXPORTS_FILE}" ${SIDE_ARTIFACTS}
  DEPENDS ${SIDE_ARTIFACTS} "${PROJECT_SOURCE_DIR}/scripts/generate-main-exports.mjs"
  COMMENT "Generating Main export manifest from Side import closure"
  VERBATIM
)
add_custom_target(generate_main_exports DEPENDS "${MAIN_EXPORTS_FILE}.txt" "${MAIN_EXPORTS_FILE}.lld.rsp")

add_executable(shared-main
  ${CORE_RUNTIME_SOURCES}
  entry/full_entry.cpp
)
kernel_common_setup(shared-main)
target_compile_definitions(shared-main PRIVATE OCCT_WORKER_SHARED_MAIN)
# Keep every inline definition so the Side import closure can be exported.
target_compile_options(shared-main PRIVATE -fPIC -fkeep-inline-functions)
target_link_directories(shared-main PRIVATE "${OCCT_LIB_DIR}")
# Pull every OCCT archive member: Side import closure may reference any of
# them, and nothing else in Main's own objects forces these archives in.
target_link_options(shared-main PRIVATE -Wl,--whole-archive)
target_link_libraries(shared-main PRIVATE ${OCCT_TOOLKITS})
target_link_libraries(shared-main PRIVATE -Wl,--no-whole-archive)
set_target_properties(shared-main PROPERTIES SUFFIX ".mjs")
target_link_options(shared-main PRIVATE
  --no-entry
  -fwasm-exceptions
  -ffp-contract=off
  -sMAIN_MODULE=2
  -sALLOW_MEMORY_GROWTH=1
  -sMAXIMUM_MEMORY=2147483648
  -sSTACK_SIZE=5242880
  -sFILESYSTEM=0
  -sENVIRONMENT=web,worker,node
  -sMODULARIZE=1
  -sEXPORT_ES6=1
  -sASSERTIONS=2
  "--js-library=${CMAKE_CURRENT_SOURCE_DIR}/runtime/shared_hooks.js"
  -sEXPORTED_RUNTIME_METHODS=loadDynamicLibrary,UTF8ToString,HEAPU8
  # The two env hooks (progress/cancellation) stay genuine wasm imports,
  # supplied by the glue exactly like the standalone artifact.
  -sERROR_ON_UNDEFINED_SYMBOLS=0
  # Symbols defined by one Side but imported by another resolve through the
  # dylink global namespace at load time; they must not fail the Main link.
  -Wno-error=undefined
  "-sEXPORTED_FUNCTIONS=@${MAIN_EXPORTS_FILE}.txt"
  "-Wl,@${MAIN_EXPORTS_FILE}.lld.rsp"
)
add_dependencies(shared-main generate_main_exports)
set_property(TARGET shared-main APPEND PROPERTY LINK_DEPENDS
  "${MAIN_EXPORTS_FILE}.txt"
  "${MAIN_EXPORTS_FILE}.lld.rsp"
  "${CMAKE_CURRENT_SOURCE_DIR}/runtime/shared_hooks.js"
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(shared-main PRIVATE -O2)
  target_link_options(shared-main PRIVATE -O2)
else()
  target_compile_options(shared-main PRIVATE -O0 -g3)
  target_link_options(shared-main PRIVATE -O0 -g3 -sASSERTIONS=2)
endif()
