cmake_minimum_required(VERSION 3.10)
project(pse_core VERSION 1.0 LANGUAGES CXX)

# set c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

set(Python_VIRTUALENV FIRST)
set(PYBIND11_FINDPYTHON ON)
add_subdirectory(external/pybind11)


pybind11_add_module(pse_core src/acceptor.cpp)
target_compile_definitions(pse_core PRIVATE
    MODULE_NAME=$<TARGET_FILE_BASE_NAME:pse_core>
    VERSION_INFO="${PY_FULL_VERSION}"
)

set_target_properties(pse_core PROPERTIES
    CXX_VISIBILITY_PRESET "hidden"
    VISIBILITY_INLINES_HIDDEN true
)

# Install the Python module
install(TARGETS pse_core
        EXCLUDE_FROM_ALL
        COMPONENT python_modules
        DESTINATION ${PY_BUILD_CMAKE_MODULE_NAME}
)

include(cmake/stubgen.cmake)
pybind11_stubgen(pse_core)
