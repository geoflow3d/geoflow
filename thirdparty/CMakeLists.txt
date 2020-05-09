set(GF_GIT_SUBMODULES filesystem json)
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

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(json)

if(${GF_BUILD_GUI})
  set(GLM_TEST_ENABLE OFF CACHE INTERNAL "")
  include_directories(glm)
endif()