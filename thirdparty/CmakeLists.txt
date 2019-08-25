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
    OUTPUT_QUIET
    ERROR_QUIET
    )
endif()  
  
if(GF_USE_EXTERNAL_JSON)
  find_package(nlohmann_json 3.2.0 REQUIRED)
else()
  set(JSON_BuildTests OFF CACHE INTERNAL "")
  set(JSON_Install OFF CACHE INTERNAL "")
  add_subdirectory(json)
endif()