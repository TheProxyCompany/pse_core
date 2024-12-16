cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0087 NEW)

# Project settings
project(${PY_BUILD_CMAKE_MODULE_NAME} LANGUAGES CXX)
set(EXTENSION_NAME _core)

# C++ settings
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS NO)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

# Build type configuration
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Python configuration
if(CMAKE_VERSION VERSION_LESS 3.18)
    set(DEV_MODULE Development)
else()
    set(DEV_MODULE Development.Module)
endif()

set(Python_VIRTUALENV FIRST)
find_package(Python COMPONENTS Interpreter ${DEV_MODULE} REQUIRED)

# External dependencies
add_subdirectory(${CMAKE_SOURCE_DIR}/external/nanobind)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/hat-trie)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/tokenizers-cpp)

# Include directories
include_directories(
    SYSTEM
    ${CMAKE_SOURCE_DIR}/external/nanobind/include
    ${CMAKE_SOURCE_DIR}/external/hat-trie/include
)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE pse_core_src ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Main module configuration
nanobind_add_module(
    ${EXTENSION_NAME}
    STABLE_ABI NB_STATIC
    ${pse_core_src}
)

target_compile_definitions(
    ${EXTENSION_NAME}
    PRIVATE
        MODULE_NAME=$<TARGET_FILE_BASE_NAME:${EXTENSION_NAME}>
        VERSION_INFO="${PY_FULL_VERSION}"
)

target_include_directories(
    ${EXTENSION_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
)

# Installation
install(
    TARGETS ${EXTENSION_NAME}
    DESTINATION ${PROJECT_NAME}
    COMPONENT python_modules
)
