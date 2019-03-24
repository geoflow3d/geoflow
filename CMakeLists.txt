cmake_minimum_required (VERSION 3.10)

project (Geoflow VERSION 0.0.1)

option(GF_BUILD_GUI "Build the GUI components of geoflow" TRUE)
# set_property(CXX_STANDARD_REQUIRED ON)

# configure GLFW
if(${GF_BUILD_GUI})
  execute_process(
    COMMAND             git submodule update --init --recursive thirdparty/glfw
    WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}
    OUTPUT_QUIET
    ERROR_QUIET
  )
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
  add_subdirectory(thirdparty/glfw)
  set(GLFW_LIBS glfw ${GLFW_LIBRARIES})
  include_directories(SYSTEM "thirdparty/glfw/include")
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/.gitmodules")
    execute_process(
            COMMAND             git submodule update --init --recursive thirdparty/glm thirdparty/imgui thirdparty/json
            WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}
            OUTPUT_QUIET
            ERROR_QUIET
    )
endif()

# nlohmann's json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(thirdparty/json)

include_directories(
  src
  thirdparty/glad/include 
  thirdparty/imgui
  thirdparty/imgui/examples
  thirdparty/glm
  thirdparty/json/include
  thirdparty/osdialog
)

add_definitions(
  -DGLFW_INCLUDE_NONE 
  -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"
  -DIMGUI_IMPL_OPENGL_LOADER_GLAD
  -DGLM_FORCE_CTOR_INIT
)
if(MSVC)
  add_definitions(-DNOMINMAX)
endif()

add_library(geoflow-core STATIC
  src/geoflow/core/geoflow.cpp
  thirdparty/imgui/imgui.cpp
  thirdparty/imgui/misc/cpp/imgui_stdlib.cpp
  thirdparty/imgui/imgui_draw.cpp
  thirdparty/imgui/imgui_widgets.cpp
)
target_link_libraries(geoflow-core nlohmann_json::nlohmann_json)

if(${GF_BUILD_GUI})
  add_library(viewer STATIC 
    src/viewer/app.cpp 
    src/viewer/app_povi.cpp 
    thirdparty/glad/src/glad.c 
    src/viewer/gloo.cpp
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/misc/cpp/imgui_stdlib.cpp
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/examples/imgui_impl_glfw.cpp
    thirdparty/imgui/examples/imgui_impl_opengl3.cpp
  )
  target_link_libraries(viewer ${GLFW_LIBS})
  add_library(geoflow-gui STATIC
    src/geoflow/core/geoflow.cpp
    src/geoflow/gui/flowchart.cpp 
    src/geoflow/gui/imgui_color_gradient.cpp
    thirdparty/osdialog/osdialog.c
    thirdparty/osdialog/osdialog_mac.m
  )
  target_link_libraries(geoflow-gui viewer)

  add_executable(
    gui 
    examples/gui.cpp 
  )
  target_link_libraries(gui geoflow-gui)
  # add_executable(
  #   viewer-test 
  #   src/viewer/main.cpp 
  # )
  # target_link_libraries(viewer-test viewer)

  set_target_properties(
    viewer
    geoflow-gui
    # viewer-test
    gui 
    PROPERTIES CXX_STANDARD 17
  )
endif()

add_executable(basic examples/basic.cpp)
target_link_libraries(basic geoflow-core)

set_target_properties(
  geoflow-core
  basic
  PROPERTIES CXX_STANDARD 17
)