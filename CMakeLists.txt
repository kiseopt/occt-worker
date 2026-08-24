cmake_minimum_required(VERSION 3.24)
project(occt-worker VERSION 1.0.0 LANGUAGES CXX)

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "occt-worker must be built with Emscripten")
endif()

add_subdirectory(kernel)
