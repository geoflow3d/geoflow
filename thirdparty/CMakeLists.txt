set(GF_GIT_SUBMODULES filesystem)
# if(NOT ${GF_USE_EXTERNAL_JSON})
#   set(GF_GIT_SUBMODULES ${GF_GIT_SUBMODULES} json)
# endif()
if(${GF_BUILD_GUI})
  set(GF_GIT_SUBMODULES ${GF_GIT_SUBMODULES} imgui glm)
  if(${GF_BUILD_GUI_FILE_DIALOGS})
    set(GF_GIT_SUBMODULES ${GF_GIT_SUBMODULES} osdialog)
  endif()
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/.gitmodules")
  execute_process(
    COMMAND             git submodule update --init ${GF_GIT_SUBMODULES}
    WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}/thirdparty
    )
endif()  

# if(${GF_USE_EXTERNAL_JSON})
#   find_package(nlohmann_json CONFIG REQUIRED)
# else()
#   set(JSON_BuildTests OFF CACHE INTERNAL "")
#   set(JSON_Install OFF CACHE INTERNAL "")
#   add_subdirectory(json)
# endif()

if(${GF_BUILD_GUI})
  set(GLM_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/thirdparty/glm PARENT_SCOPE)
endif()