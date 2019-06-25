cmake_minimum_required (VERSION 3.10)

project (Geoflow VERSION 0.0.1)

option(GF_BUILD_GUI "Build the GUI components of geoflow" TRUE)

# configure GLFW
find_package(glfw3 3.3 REQUIRED)

if(EXISTS "${PROJECT_SOURCE_DIR}/.gitmodules")
    execute_process(
            COMMAND             git submodule update --init --recursive thirdparty/glm thirdparty/imgui
            WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}
            OUTPUT_QUIET
            ERROR_QUIET
    )
endif()

# DLLoader
if (WIN32)
	include_directories(
		src/DLLoader/Windows/
	)
endif(WIN32)

if(UNIX)
	include_directories(
		src/DLLoader/Unix/
	)
    # set (CMAKE_CXX_FLAGS "-W -Wall -Wextra")
endif(UNIX)

# nlohmann's json
find_package(nlohmann_json)

include_directories(
  src
  thirdparty/glad/include
  thirdparty/imgui
  thirdparty/imgui/examples
  thirdparty/glm
  thirdparty/osdialog
)

add_definitions(
  -DGLFW_INCLUDE_NONE 
  -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"
  -DCMAKE_INSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"
  -DIMGUI_IMPL_OPENGL_LOADER_GLAD
  -DGLM_FORCE_CTOR_INIT
)
if(MSVC)
  add_definitions(-DNOMINMAX)
endif()
if(GF_BUILD_GUI)
  add_definitions(-DGF_BUILD_GUI)
endif()

add_library(geoflow-core SHARED
  src/geoflow/geoflow.cpp
  thirdparty/imgui/imgui.cpp
  thirdparty/imgui/misc/cpp/imgui_stdlib.cpp
  thirdparty/imgui/imgui_draw.cpp
  thirdparty/imgui/imgui_widgets.cpp
)
target_link_libraries(geoflow-core PRIVATE nlohmann_json::nlohmann_json)

set_target_properties(
  geoflow-core
  PROPERTIES CXX_STANDARD 17
)

if(${GF_BUILD_GUI})
  SET(GEOFLOW_GUI_SOURCES
    src/viewer/app.cpp 
    src/viewer/app_povi.cpp 
    thirdparty/glad/src/glad.c 
    src/viewer/gloo.cpp
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/examples/imgui_impl_glfw.cpp
    thirdparty/imgui/examples/imgui_impl_opengl3.cpp
    src/geoflow/gui/flowchart.cpp 
    src/geoflow/gui/imgui_color_gradient.cpp
    thirdparty/osdialog/osdialog.c
    src/geoflow/gui/osdialog.cpp)
  SET(GEOFLOW_GUI_LIBS geoflow-core )
  if(APPLE)
    SET(GEOFLOW_GUI_SOURCES ${GEOFLOW_GUI_SOURCES}
      thirdparty/osdialog/osdialog_mac.m)
  elseif(MSVC)
    SET(GEOFLOW_GUI_SOURCES ${GEOFLOW_GUI_SOURCES}
      thirdparty/osdialog/osdialog_win.c)
    SET(GEOFLOW_GUI_LIBS ${GEOFLOW_GUI_LIBS}
      comdlg32)
  else()
    execute_process (
      COMMAND "pkg-config --libs gtk+-2.0"
      OUTPUT_VARIABLE gtklib
    )
    SET(GEOFLOW_GUI_SOURCES ${GEOFLOW_GUI_SOURCES}
      thirdparty/osdialog/osdialog_gtk2.c)
    SET(GEOFLOW_GUI_LIBS ${GEOFLOW_GUI_LIBS}
      gtklib)
  endif()

  add_library(geoflow-gui SHARED ${GEOFLOW_GUI_SOURCES})
  # target_include_directories(geoflow-gui nlohmann_json::nlohmann_json)
  target_link_libraries(geoflow-gui PUBLIC ${GEOFLOW_GUI_LIBS} PRIVATE glfw)
  set_target_properties(
    geoflow-gui
    PROPERTIES CXX_STANDARD 17
  )
endif()

# ensure rpath is set up properly (https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling)
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/geoflow")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


install(FILES 
  src/geoflow/common.hpp
  src/geoflow/parameters.hpp
  src/geoflow/geoflow.hpp 
  DESTINATION include/geoflow)

install(TARGETS geoflow-gui geoflow-core EXPORT geoflow-targets DESTINATION lib/geoflow)
install(EXPORT geoflow-targets DESTINATION lib/geoflow)
install(FILES geoflow-config.cmake DESTINATION lib/geoflow)

install(FILES 
  src/viewer/shaders/crosshair.frag 
  src/viewer/shaders/crosshair.vert 
  src/viewer/shaders/basic.frag 
  src/viewer/shaders/basic.vert 
  DESTINATION share/geoflow)

add_subdirectory(apps)
add_subdirectory(examples)