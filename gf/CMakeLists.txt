set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")

project( geoflow )

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++17> $<$<COMPILE_LANGUAGE:C>:-std=c99>)

cmake_minimum_required(VERSION 2.6.2)
if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" VERSION_GREATER 2.6)
  if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_GREATER 2.8.3)
    cmake_policy(VERSION 2.8.4)
  else()
    cmake_policy(VERSION 2.6)
  endif()
endif()

set(CMAKE_CXX_FLAGS "-O2")

 
include_directories(/usr/local/include src thirdparty/povi/src thirdparty/povi/imgui thirdparty/povi/glad/include)

set(CMAKE_EXE_LINKER_FLAGS -Wl)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "/usr/local/opt/llvm/lib")
link_directories("/usr/local/opt/llvm/lib")

# add_subdirectory(examples)
add_subdirectory(thirdparty/povi)

add_executable(basic examples/basic.cpp src/geoflow.cpp)
target_link_libraries(basic povi_lib)

add_executable(
  gui 
  examples/gui.cpp 
  src/nodes.cpp 
  src/geoflow.cpp
)
target_link_libraries(gui povi_lib)