cmake_minimum_required(VERSION 3.24)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/protocol/artifacts.json" ARTIFACT_IDENTITY_JSON)
string(JSON PROJECT_PROTOCOL_VERSION GET "${ARTIFACT_IDENTITY_JSON}" protocolVersion)
project(occt-worker VERSION ${PROJECT_PROTOCOL_VERSION} LANGUAGES CXX)

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "occt-worker must be built with Emscripten")
endif()

add_subdirectory(kernel)
