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
 
include_directories( src )

set(CMAKE_EXE_LINKER_FLAGS -Wl)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "/usr/local/opt/llvm/lib")
link_directories("/usr/local/opt/llvm/lib")

# add_library(libgeoflow STATIC src/geoflow.cpp )
# target_link_libraries(libgeoflow )
# Creating entries for target

# add_subdirectory(examples)

add_executable(math src/math_example.cpp src/geoflow.cpp)