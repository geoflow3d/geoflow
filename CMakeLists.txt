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

include_directories(${GLFW_INCLUDE_DIRS} include /usr/local/include)
link_directories(${GLFW_LIBRARY_DIRS})

add_definitions(-DGLFW_INCLUDE_NONE -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

add_executable(povi main.cpp app.cpp src/glad.c shader.cpp)
target_link_libraries(povi ${GLFW_LIBRARIES})