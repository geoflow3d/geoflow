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

add_executable(povi main.cpp app.cpp)

# find system-installed glfw3 and link dynamically
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_package(OpenGL REQUIRED)

include_directories(${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} /usr/local/include)
target_link_libraries(povi ${GLFW_LIBRARIES} ${OPENGL_gl_LIBRARY})