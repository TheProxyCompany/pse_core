cmake_minimum_required(VERSION 3.10)
project(pse_core VERSION 1.0 LANGUAGES CXX)

# set c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

# set build type
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# set dev module
if (CMAKE_VERSION VERSION_LESS 3.18)
  set(DEV_MODULE Development)
else()
  set(DEV_MODULE Development.Module)
endif()

# get python
set(Python_VIRTUALENV FIRST)
find_package(Python COMPONENTS Interpreter ${DEV_MODULE} REQUIRED)

# add nanobind
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/nanobind)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/external/nanobind/include)

# add sources
file(GLOB_RECURSE SOURCES src/**.cpp)
add_library(pse_core SHARED ${SOURCES})
target_include_directories(pse_core PUBLIC ./include)

# add python module
add_subdirectory(pse_core)
