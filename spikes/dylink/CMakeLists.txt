cmake_minimum_required(VERSION 3.24)
project(occt-dylink-spike VERSION 1.0.0 LANGUAGES CXX)

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "dylink spike must be built with Emscripten")
endif()

set(SPIKE_COMMON_FLAGS -fwasm-exceptions -ffp-contract=off)

find_program(NODE_BIN node REQUIRED)

# Base exports: host API + JS entry points, independent of any side module.
set(BASE_EXPORTS
  _malloc,_free,_plugin_register_v1,_plugin_registration_count,_plugin_descriptor_name,_plugin_descriptor_abi_version,_host_last_error,_host_add_one,_host_alloc,_host_free,_host_memory_pages,_host_grow_pages,_host_dispatch,_host_build_family,_occt_host_build_family,_occt_host_plugin_count,_occt_host_plugin_error
)

# Side module first: its import surface drives Main's export manifest.
add_executable(dylink-side side.cpp)
target_compile_features(dylink-side PRIVATE cxx_std_17)
target_include_directories(dylink-side PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(dylink-side PRIVATE ${SPIKE_COMMON_FLAGS} -fPIC)
set_target_properties(dylink-side PROPERTIES SUFFIX ".wasm")
target_link_options(dylink-side PRIVATE
  ${SPIKE_COMMON_FLAGS}
  --no-entry
  -sSIDE_MODULE=2
  -sASSERTIONS=2
)

# Symbol closure step: read dylink-side.wasm imports, emit Main exports.
# This mirrors the release flow where the Main keepalive/export list is
# generated from side-module undefined-import closure instead of hand lists.
set(MAIN_EXPORTS_FILE "${CMAKE_CURRENT_BINARY_DIR}/main-exports.txt")
add_custom_command(
  OUTPUT "${MAIN_EXPORTS_FILE}"
  COMMAND "${NODE_BIN}" "${CMAKE_CURRENT_SOURCE_DIR}/extract-exports.mjs"
          "$<TARGET_FILE:dylink-side>" "${MAIN_EXPORTS_FILE}" ${BASE_EXPORTS}
  DEPENDS dylink-side "${CMAKE_CURRENT_SOURCE_DIR}/extract-exports.mjs"
  COMMENT "Generating Main export manifest from dylink-side imports"
  VERBATIM
)
add_custom_target(generate_main_exports DEPENDS "${MAIN_EXPORTS_FILE}")

# Main module: relocatable host with JS glue (ES6, modularized).
add_executable(dylink-main main.cpp)
target_compile_features(dylink-main PRIVATE cxx_std_17)
target_include_directories(dylink-main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(dylink-main PRIVATE ${SPIKE_COMMON_FLAGS})
set_target_properties(dylink-main PROPERTIES SUFFIX ".mjs")
target_link_options(dylink-main PRIVATE
  ${SPIKE_COMMON_FLAGS}
  --no-entry
  -sMAIN_MODULE=2
  -sALLOW_MEMORY_GROWTH=1
  -sMAXIMUM_MEMORY=2147483648
  -sSTACK_SIZE=1048576
  -sFILESYSTEM=0
  -sENVIRONMENT=web,worker,node
  -sMODULARIZE=1
  -sEXPORT_ES6=1
  -sASSERTIONS=2
  -sEXPORTED_RUNTIME_METHODS=loadDynamicLibrary,UTF8ToString
  "-sEXPORTED_FUNCTIONS=@${MAIN_EXPORTS_FILE}"
)
add_dependencies(dylink-main generate_main_exports)

# Side modules resolve any libc++/libc++abi symbol from Main at runtime, so
# Main links those system archives with whole-archive via EMCC_FORCE_STDLIBS.
set_property(TARGET dylink-main PROPERTY RULE_LAUNCH_LINK
  "${CMAKE_COMMAND} -E env EMCC_FORCE_STDLIBS=libc++abi,libc++"
)
