cmake_minimum_required(VERSION 3.10)
project(pse_core VERSION 1.0 LANGUAGES CXX)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# set c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/nanobind)
add_subdirectory(external/pybind11)
include_directories(SYSTEM external/pybind11/include)

file(GLOB_RECURSE SOURCES src/**.cpp)
add_library(pse_core SHARED ${SOURCES})
target_include_directories(pse_core PUBLIC ./include)

add_subdirectory(pse_core)
