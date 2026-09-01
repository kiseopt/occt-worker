set(OCCT_INCLUDE_DIR "${OCCT_ROOT}/include/opencascade")
set(OCCT_LIB_DIR "${OCCT_ROOT}/lib")

include("${CMAKE_CURRENT_SOURCE_DIR}/generated/profile-topology.generated.cmake")

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
set(OCCT_WORKER_MAX_MEMORY "2147483648" CACHE STRING "wasm maximum memory in bytes")
set(OCCT_WORKER_THINLTO_CACHE "" CACHE PATH
  "Shared ThinLTO cache directory for Release links; empty disables caching")

# ThinLTO redoes codegen over the whole OCCT bitcode on every link, and this
# project links that bitcode 16 times (standalone, 5 profiles, Main, 9 Sides).
# A persistent cache lets a repeated link reuse the previous codegen. Cache keys
# fold in each module's import list, so links whose toolkit sets differ still
# miss: the reliable win is across runs, not across profiles.
function(kernel_thinlto_cache theTarget)
  if(NOT OCCT_WORKER_THINLTO_CACHE)
    return()
  endif()
  target_link_options(${theTarget} PRIVATE
    "-Wl,--thinlto-cache-dir=${OCCT_WORKER_THINLTO_CACHE}"
    "-Wl,--thinlto-cache-policy=cache_size_bytes=2g:prune_after=168h"
  )
endfunction()

function(standalone_kernel_target theTarget theSourcesVariable theToolkitsVariable)
  add_executable(${theTarget} ${${theSourcesVariable}})
  kernel_common_setup(${theTarget})
  target_link_directories(${theTarget} PRIVATE "${OCCT_LIB_DIR}")
  target_link_libraries(${theTarget} PRIVATE -Wl,--start-group ${${theToolkitsVariable}} -Wl,--end-group)
  set_target_properties(${theTarget} PROPERTIES SUFFIX ".wasm")
  target_link_options(${theTarget} PRIVATE
    --no-entry
    -fwasm-exceptions
    -ffp-contract=off
    "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${theTarget}.map"
    -sSTANDALONE_WASM
    -sALLOW_MEMORY_GROWTH=1
    -sMAXIMUM_MEMORY=${OCCT_WORKER_MAX_MEMORY}
    -sSTACK_SIZE=5242880
    -sFILESYSTEM=0
    -sENVIRONMENT=web,worker,node
    -sEXPORTED_FUNCTIONS=_k_alloc,_k_free,_k_handle,_k_response_ptr,_k_buffer_ptr,_k_buffer_len
  )
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${theTarget} PRIVATE -O3 -flto=thin)
    target_link_options(${theTarget} PRIVATE -O3 -flto=thin)
    kernel_thinlto_cache(${theTarget})
  else()
    target_compile_options(${theTarget} PRIVATE -O0 -g3)
    target_link_options(${theTarget} PRIVATE -O0 -g3 -sASSERTIONS=2)
  endif()
endfunction()

if(KERNEL_BUILD_PROFILES)
  foreach(PROFILE_ID IN LISTS OCCT_PROFILE_IDS)
    string(REPLACE "-" "_" PROFILE_KEY "${PROFILE_ID}")
    set(PROFILE_TARGET "${OCCT_PROFILE_${PROFILE_KEY}_TARGET}")
    standalone_kernel_target("${PROFILE_TARGET}" "OCCT_PROFILE_${PROFILE_KEY}_SOURCES" "OCCT_PROFILE_${PROFILE_KEY}_TOOLKITS")
  endforeach()
  return()
endif()

if(NOT KERNEL_BUILD_SHARED)
  standalone_kernel_target(occt-worker OCCT_PROFILE_full_profile_SOURCES OCCT_PROFILE_full_profile_TOOLKITS)
  return()
endif()

# ---------------------------------------------------------------------------
# Shared Main + Side targets.
# ---------------------------------------------------------------------------

# Side modules first: their import closure drives the Main export manifest.
foreach(SIDE_ID IN LISTS OCCT_SIDE_IDS)
  string(REPLACE "-" "_" SIDE_KEY "${SIDE_ID}")
  set(SIDE_TARGET "${OCCT_SIDE_${SIDE_KEY}_TARGET}")
  add_executable(${SIDE_TARGET} ${OCCT_SIDE_${SIDE_KEY}_SOURCES})
  kernel_common_setup(${SIDE_TARGET})
  target_compile_definitions(${SIDE_TARGET} PRIVATE OCCT_WORKER_SHARED_SIDE)
  target_compile_options(${SIDE_TARGET} PRIVATE
    -fPIC -fvisibility=hidden -fvisibility-inlines-hidden)
  set_target_properties(${SIDE_TARGET} PROPERTIES SUFFIX ".wasm")
  target_link_options(${SIDE_TARGET} PRIVATE
    --no-entry
    -fwasm-exceptions
    -ffp-contract=off
    -fPIC
    -Wl,-Bsymbolic
    -sSIDE_MODULE=2
    -sENVIRONMENT=web,worker,node
  )
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${SIDE_TARGET} PRIVATE -flto=thin)
    target_link_options(${SIDE_TARGET} PRIVATE -flto=thin)
    kernel_thinlto_cache(${SIDE_TARGET})
  endif()
endforeach()

find_program(NODE_BIN node REQUIRED)
set(MAIN_EXPORTS_FILE "${CMAKE_CURRENT_BINARY_DIR}/main-exports")
set(SIDE_ARTIFACTS "")
foreach(SIDE_ID IN LISTS OCCT_SIDE_IDS)
  string(REPLACE "-" "_" SIDE_KEY "${SIDE_ID}")
  list(APPEND SIDE_ARTIFACTS "${CMAKE_CURRENT_BINARY_DIR}/${OCCT_SIDE_${SIDE_KEY}_TARGET}.wasm")
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

add_executable(shared-main ${OCCT_SHARED_MAIN_SOURCES})
kernel_common_setup(shared-main)
target_compile_definitions(shared-main PRIVATE OCCT_WORKER_SHARED_MAIN)
# Keep every inline definition so the Side import closure can be exported.
target_compile_options(shared-main PRIVATE -fPIC -fkeep-inline-functions)
target_link_directories(shared-main PRIVATE "${OCCT_LIB_DIR}")
# Pull every OCCT archive member: Side import closure may reference any of
# them, and nothing else in Main's own objects forces these archives in.
target_link_options(shared-main PRIVATE -Wl,--whole-archive)
target_link_libraries(shared-main PRIVATE -Wl,--start-group ${OCCT_SHARED_MAIN_TOOLKITS} -Wl,--end-group)
target_link_libraries(shared-main PRIVATE -Wl,--no-whole-archive)
set_target_properties(shared-main PROPERTIES SUFFIX ".mjs")
target_link_options(shared-main PRIVATE
  --no-entry
  -fwasm-exceptions
  -ffp-contract=off
  -sMAIN_MODULE=2
  -sALLOW_MEMORY_GROWTH=1
  -sMAXIMUM_MEMORY=${OCCT_WORKER_MAX_MEMORY}
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
  target_compile_options(shared-main PRIVATE -O2 -flto=thin)
  target_link_options(shared-main PRIVATE -O2 -flto=thin)
  kernel_thinlto_cache(shared-main)
else()
  target_compile_options(shared-main PRIVATE -O0 -g3)
  target_link_options(shared-main PRIVATE -O0 -g3 -sASSERTIONS=2)
endif()
