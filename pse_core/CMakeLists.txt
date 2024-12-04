cmake_policy(SET CMP0087 NEW)

file(GLOB_RECURSE SOURCES
        _core/**.cpp
        _core/**.h
)
nanobind_add_module(_core STABLE_ABI NB_STATIC ${SOURCES})
target_include_directories(_core PRIVATE ${CMAKE_CURRENT_LIST_DIR}/_core)
target_link_libraries(_core PRIVATE pse_core)
install(TARGETS _core LIBRARY DESTINATION pse_core)

nanobind_add_stub(
        _core_stub
        INSTALL_TIME
        MARKER_FILE pse_core/py.typed
        MODULE _core
        OUTPUT pse_core/_core.pyi
        PYTHON_PATH $<TARGET_FILE_DIR:_core>
        DEPENDS _core
        COMPONENT python_modules
)
