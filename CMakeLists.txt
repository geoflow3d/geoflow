cmake_minimum_required (VERSION 3.13)

project (Geoflow VERSION 0.0.1)

option(GF_BUILD_GUI "Build the GUI components of geoflow" TRUE)
option(GF_USE_EXTERNAL_JSON "Use an external JSON library" OFF)

# dependencies
if(GF_BUILD_GUI)
  find_package(glfw3 3.3 REQUIRED)
  #message(STATUS "GLFW libs: ${GLFW_LIBRARIES}")
  find_package(glm)
endif()
if(GF_USE_EXTERNAL_JSON)
  find_package(nlohmann_json 3.2.0 REQUIRED)
endif()

add_subdirectory(thirdparty)
  
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

# definitions
set(GF_SHADER_PATH ${CMAKE_INSTALL_PREFIX}/share/geoflow)
if(WIN32)
  set(GF_SHADER_PATH "bin\\share")
  add_definitions(
    -DGF_SHADER_PATH=\"share\"
  )
else()
  add_definitions(
    -DGF_SHADER_PATH=\"${GF_SHADER_PATH}\"
    )
endif()
add_definitions(
  -DGLFW_INCLUDE_NONE
  -DIMGUI_IMPL_OPENGL_LOADER_GLAD
  -DGLM_FORCE_CTOR_INIT
)
if(MSVC)
  add_definitions(-DNOMINMAX)
endif()
if(GF_BUILD_GUI)
  add_definitions(-DGF_BUILD_GUI)
endif()

# includes
include_directories(
  src
  thirdparty/filesystem/include
  ${CMAKE_BINARY_DIR}
)

# targets
add_library(geoflow-core SHARED
  src/geoflow/geoflow.cpp
)
target_link_libraries(geoflow-core PRIVATE nlohmann_json::nlohmann_json)
set_target_properties(geoflow-core PROPERTIES 
  CXX_STANDARD 17
  WINDOWS_EXPORT_ALL_SYMBOLS TRUE
)
if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  target_compile_options(geoflow-core PUBLIC "/Zc:__cplusplus")
endif()

if(${GF_BUILD_GUI})
  include_directories(
    thirdparty/glad/include
    thirdparty/imgui
    thirdparty/imgui/examples
    thirdparty/osdialog
  )
  set(GF_SHADER_FILES 
    ${CMAKE_SOURCE_DIR}/src/viewer/shaders/crosshair.frag 
    ${CMAKE_SOURCE_DIR}/src/viewer/shaders/crosshair.vert 
    ${CMAKE_SOURCE_DIR}/src/viewer/shaders/basic.frag 
    ${CMAKE_SOURCE_DIR}/src/viewer/shaders/basic.vert)
  SET(GF_GUI_SOURCES
    src/viewer/app.cpp 
    src/viewer/app_povi.cpp 
    thirdparty/glad/src/glad.c 
    src/viewer/gloo.cpp
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/misc/cpp/imgui_stdlib.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/examples/imgui_impl_glfw.cpp
    thirdparty/imgui/examples/imgui_impl_opengl3.cpp
    src/geoflow/gui/ImNodes.cpp 
    src/geoflow/gui/ImNodesEz.cpp 
    src/geoflow/gui/imgui_color_gradient.cpp
    thirdparty/osdialog/osdialog.c
    src/geoflow/gui/osdialog.cpp)
  SET(GF_FILEDIALOG_LIBS "")
  if(APPLE)
    SET(GF_GUI_SOURCES ${GF_GUI_SOURCES}
      thirdparty/osdialog/osdialog_mac.m)
    SET(GF_FILEDIALOG_LIBS ${GF_FILEDIALOG_LIBS}
      "-framework AppKit")
  elseif(MSVC)
    SET(GF_GUI_SOURCES ${GF_GUI_SOURCES}
      thirdparty/osdialog/osdialog_win.c)
    SET(GF_FILEDIALOG_LIBS ${GF_FILEDIALOG_LIBS}
      comdlg32)
  else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK2 REQUIRED gtk+-2.0)

    SET(GF_GUI_SOURCES ${GF_GUI_SOURCES}
      thirdparty/osdialog/osdialog_gtk2.c)
    SET(GF_FILEDIALOG_LIBS ${GF_FILEDIALOG_LIBS}
      ${GTK2_LIBRARIES})
    include_directories(${GTK2_INCLUDE_DIRS})
  endif()

  add_library(geoflow-gui STATIC ${GF_GUI_SOURCES})
  # target_include_directories(geoflow-gui nlohmann_json::nlohmann_json)
  target_link_libraries(geoflow-gui PRIVATE geoflow-core glm glfw ${GF_FILEDIALOG_LIBS})
  set_target_properties(geoflow-gui PROPERTIES CXX_STANDARD 17)
  if (WIN32)
    get_target_property(GLFW_DLL_LOCATION glfw IMPORTED_LOCATION_RELEASE)
    message("GLFW DLL: ${GLFW_DLL_LOCATION}")
    install(FILES ${GLFW_DLL_LOCATION} DESTINATION bin)
  endif (WIN32)
endif()

# installation

# ensure rpath is set up properly (https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling)
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(GF_PLUGIN_FOLDER "$ENV{HOME}/.geoflow/plugins")
set(GF_PLUGIN_EXTENSION ".so")
if (WIN32)
  set(GF_PLUGIN_FOLDER "$ENV{USERPROFILE}\\.geoflow\\plugins")
  string(REPLACE "\\" "\\\\" GF_PLUGIN_FOLDER ${GF_PLUGIN_FOLDER})
  set(GF_PLUGIN_EXTENSION ".dll")
endif()
message(STATUS "Setting Geoflow plugin folder to ${GF_PLUGIN_FOLDER}")
message(STATUS "Setting Geoflow plugin extension to ${GF_PLUGIN_EXTENSION}")
add_definitions(-DGF_PLUGIN_FOLDER=\"${GF_PLUGIN_FOLDER}\")
add_definitions(-DGF_PLUGIN_EXTENSION=\"${GF_PLUGIN_EXTENSION}\")

# write the hash of the concatenated shared header files to a new header file, so that we can check compatibility between main geof executable and the plugins
set(GF_SHH_FILE ${CMAKE_BINARY_DIR}/gfSharedHeadersHash.h)
add_custom_command(
  TARGET geoflow-core
  COMMAND ${CMAKE_COMMAND} -DOUTPUT_FILE=${GF_SHH_FILE} -P "${CMAKE_CURRENT_LIST_DIR}/cmake/ComputeSharedHeadersHash.cmake"
)
  
set_property(TARGET geoflow-core PROPERTY PUBLIC_HEADER
  src/geoflow/common.hpp
  src/geoflow/parameters.hpp
  src/geoflow/geoflow.hpp
  ${GF_SHH_FILE}
)

install(TARGETS geoflow-core EXPORT geoflow-targets 
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/geoflow)
install(EXPORT geoflow-targets DESTINATION lib/cmake/geoflow)

include ( CMakePackageConfigHelpers )
configure_package_config_file(cmake/geoflow-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/geoflow-config.cmake
  INSTALL_DESTINATION lib/cmake/geoflow
  PATH_VARS GF_PLUGIN_FOLDER)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/geoflow-config.cmake DESTINATION lib/cmake/geoflow)

add_subdirectory(apps)
# add_subdirectory(examples)

if (WIN32)
  set(CPACK_GENERATOR NSIS)
  set(CPACK_NSIS_MODIFY_PATH ON)
endif (WIN32)
set(CPACK_PACKAGE_NAME "Geoflow")
set(CPACK_PACKAGE_VENDOR "Ravi Peters")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Geoflow - Software")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Geoflow")

INCLUDE(CPack)
