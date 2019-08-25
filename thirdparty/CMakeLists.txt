set(GF_GIT_SUBMODULES "filesystem")
if(${GF_BUILD_GUI})
  set(GF_GIT_SUBMODULES ${GF_GIT_SUBMODULES} imgui osdialog)
endif()
if(NOT GF_USE_EXTERNAL_JSON)
  set(GF_GIT_SUBMODULES ${GF_GIT_SUBMODULES} json)
endif()


if(EXISTS "${PROJECT_SOURCE_DIR}/.gitmodules")
  execute_process(
    COMMAND             git submodule update --init ${GF_GIT_SUBMODULES}
    WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}/thirdparty
    )
endif()  
  
if(NOT GF_USE_EXTERNAL_JSON)
  set(JSON_BuildTests OFF CACHE INTERNAL "")
  set(JSON_Install OFF CACHE INTERNAL "")
  add_subdirectory(json)
endif()