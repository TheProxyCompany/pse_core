file(GLOB_RECURSE SOURCES
        _core/**.cpp
        _core/**.h
)
nanobind_add_module(_core ${SOURCES})
target_include_directories(_core PRIVATE ${CMAKE_CURRENT_LIST_DIR}/_core)
target_link_libraries(_core PRIVATE pse_core)
