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

# DLLoader
if (WIN32)
	# set(DLLOADER_SRC
	# 	Core/DLLoader/Windows/DLLoader.h
	# )
	
	include_directories(
		src/DLLoader/Windows/
	)

endif(WIN32)

if(UNIX)
	# set(DLLOADER_SRC
	# 	Core/DLLoader/Unix/DLLoader.h
	# )

	include_directories(
		src/DLLoader/Unix/
	)

    # set (CMAKE_CXX_FLAGS "-W -Wall -Wextra")
endif(UNIX)

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
if(GF_BUILD_GUI)
  add_definitions(-DGF_BUILD_GUI)
endif()

add_library(geoflow-core STATIC
  src/geoflow/core/geoflow.cpp
  thirdparty/imgui/imgui.cpp
  thirdparty/imgui/misc/cpp/imgui_stdlib.cpp
  thirdparty/imgui/imgui_draw.cpp
  thirdparty/imgui/imgui_widgets.cpp
)
target_link_libraries(geoflow-core nlohmann_json::nlohmann_json)

set_target_properties(
geoflow-core
PROPERTIES CXX_STANDARD 17
)

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
  
  SET(GEOFLOW_GUI_SOURCES 
    src/geoflow/core/geoflow.cpp
    src/geoflow/gui/flowchart.cpp 
    src/geoflow/gui/imgui_color_gradient.cpp
    thirdparty/osdialog/osdialog.c
    src/geoflow/gui/osdialog.cpp)
  SET(GEOFLOW_GUI_LIBS viewer)
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

  add_library(geoflow-gui STATIC ${GEOFLOW_GUI_SOURCES})
  target_link_libraries(geoflow-gui ${GEOFLOW_GUI_LIBS})


  set_target_properties(
    viewer
    geoflow-gui
    PROPERTIES CXX_STANDARD 17
  )
endif()

add_subdirectory(apps)
add_subdirectory(examples)