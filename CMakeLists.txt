cmake_minimum_required (VERSION 2.6)
project (Povi)

add_executable(povi main.cpp)

# find system-installed glfw3 and link dynamically
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)

include_directories(${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
target_link_libraries(povi ${GLFW_LIBRARIES} ${OPENGL_gl_LIBRARY})