cmake_minimum_required (VERSION 2.6)
project (Povi)

# C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# find system-installed glfw3 and link dynamically
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(
  ${GLFW_INCLUDE_DIRS} 
  glad/include 
  imgui
  /usr/local/include)
link_directories(${GLFW_LIBRARY_DIRS})

add_definitions(-DGLFW_INCLUDE_NONE -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

add_executable(
  povi 
  src/main.cpp 
  src/app.cpp 
  src/app_povi.cpp 
  glad/src/glad.c 
  src/gloo.cpp
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_impl_glfw.cpp
  imgui/imgui_impl_opengl3.cpp
)
target_link_libraries(povi ${GLFW_LIBRARIES})