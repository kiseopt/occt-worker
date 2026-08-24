set(OCCT_INCLUDE_DIR "${OCCT_ROOT}/include/opencascade")
set(OCCT_LIB_DIR "${OCCT_ROOT}/lib")

add_executable(occt-worker src/kernel.cpp)
target_compile_features(occt-worker PRIVATE cxx_std_17)
target_include_directories(occt-worker PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${PROJECT_SOURCE_DIR}/vendor/json/single_include"
  "${OCCT_INCLUDE_DIR}"
)
target_compile_options(occt-worker PRIVATE -fwasm-exceptions -ffp-contract=off)

target_link_directories(occt-worker PRIVATE "${OCCT_LIB_DIR}")
target_link_libraries(occt-worker PRIVATE
  TKBinXCAF TKXmlXCAF TKBin TKBinL TKXml TKXmlL
  TKDEVRML TKRWMesh TKV3d TKService TKXCAF TKLCAF TKCAF TKCDF TKHLR
  TKDESTL TKDEIGES TKDESTEP TKXSBase TKDE TKFeat TKOffset TKFillet TKBO TKBool TKPrim TKMesh TKShHealing TKTopAlgo TKGeomAlgo TKBRep TKGeomBase TKG3d TKG2d TKMath TKernel
)

set_target_properties(occt-worker PROPERTIES SUFFIX ".wasm")
target_link_options(occt-worker PRIVATE
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
  target_compile_options(occt-worker PRIVATE -O3)
  target_link_options(occt-worker PRIVATE -O3 -flto)
else()
  target_compile_options(occt-worker PRIVATE -O0 -g3)
  target_link_options(occt-worker PRIVATE -O0 -g3 -sASSERTIONS=2)
endif()
