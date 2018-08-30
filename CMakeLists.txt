set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")
cmake_minimum_required (VERSION 2.6)
project (Geoflow)

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++17> $<$<COMPILE_LANGUAGE:C>:-std=c99>)
set(CMAKE_CXX_FLAGS "-O2")

set(CMAKE_EXE_LINKER_FLAGS -Wl)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "/usr/local/opt/llvm/lib")
link_directories("/usr/local/opt/llvm/lib")

# find system-installed glfw3 and link dynamically
# find_package(PkgConfig REQUIRED)
# pkg_search_module(GLFW REQUIRED glfw3)

# GLFW3
if(EXISTS "${PROJECT_SOURCE_DIR}/.gitmodules")
    execute_process(
            COMMAND             git submodule update --init --recursive
            WORKING_DIRECTORY   ${PROJECT_SOURCE_DIR}
            OUTPUT_QUIET
            ERROR_QUIET
    )
endif()
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
add_subdirectory(thirdparty/glfw)
set(GLFW_LIBS glfw ${GLFW_LIBRARIES})
include_directories(SYSTEM "thirdparty/glfw/include")

include_directories(
  src/viewer
  src/flowchart
  thirdparty/glad/include 
  thirdparty/imgui
  thirdparty/imgui/examples
  /usr/local/include)

add_definitions(
  -DGLFW_INCLUDE_NONE 
  -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"
  -DIMGUI_IMPL_OPENGL_LOADER_GLAD
  )

add_library(viewer STATIC 
  src/viewer/app.cpp 
  src/viewer/app_povi.cpp 
  thirdparty/glad/src/glad.c 
  src/viewer/gloo.cpp
  thirdparty/imgui/imgui.cpp
  thirdparty/imgui/imgui_demo.cpp
  thirdparty/imgui/imgui_draw.cpp
  thirdparty/imgui/examples/imgui_impl_glfw.cpp
  thirdparty/imgui/examples/imgui_impl_opengl3.cpp
)
target_link_libraries(viewer ${GLFW_LIBRARIES})

add_executable(
  viewer-test 
  src/viewer/main.cpp 
)
target_link_libraries(viewer-test viewer)

add_executable(basic examples/basic.cpp src/flowchart/geoflow.cpp)
target_link_libraries(basic viewer)

add_executable(
  gui 
  examples/gui.cpp 
  src/flowchart/nodes.cpp 
  src/flowchart/geoflow.cpp
)
target_link_libraries(gui viewer)