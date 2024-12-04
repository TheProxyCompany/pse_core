cmake_minimum_required(VERSION 3.10)
project(pse_core VERSION 1.0 LANGUAGES CXX)

# set c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

set(PYBIND11_FINDPYTHON ON)
add_subdirectory(external/pybind11)
include_directories(SYSTEM external/pybind11/include)

file(GLOB_RECURSE SOURCES src/**.cpp)
add_library(pse_core SHARED ${SOURCES})
target_include_directories(pse_core PUBLIC ./include)

add_subdirectory(pse_core)
