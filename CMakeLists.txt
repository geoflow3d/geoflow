cmake_minimum_required (VERSION 3.21)

project (Geoflow VERSION 0.2.0)

include(GNUInstallDirs)

set(GF_PLUGIN_FOLDER "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/geoflow-plugins" CACHE STRING "Geoflow plugin folder")
set(GF_PLUGIN_FOLDER_INSTALL "${GF_PLUGIN_FOLDER}")
if(PROJECT_IS_TOP_LEVEL)
  if (WIN32)
    string(REPLACE "\\" "\\\\" GF_PLUGIN_FOLDER ${GF_PLUGIN_FOLDER})
    string(REPLACE "\\" "\\\\" GF_PLUGIN_FOLDER_INSTALL ${GF_PLUGIN_FOLDER})
  endif()
else()
  set(GF_PLUGIN_FOLDER_INSTALL "${GF_PLUGIN_FOLDER}" PARENT_SCOPE)
  if (WIN32)
    set(GF_PLUGIN_FOLDER_INSTALL "..\\lib\\geoflow-plugins") # this needs to be a relative to the installed geof.exe
    string(REPLACE "\\" "\\\\" GF_PLUGIN_FOLDER ${GF_PLUGIN_FOLDER})
    string(REPLACE "\\" "\\\\" GF_PLUGIN_FOLDER_INSTALL ${GF_PLUGIN_FOLDER})
  endif()
endif()

option(GF_BUILD_GUI "Build the GUI components of geoflow" TRUE)
option(GF_BUILD_GUI_FILE_DIALOGS "Build GUI with OS native file dialogs" TRUE)

# dependencies
add_subdirectory(thirdparty)

find_package(nlohmann_json 3.10.5 CONFIG REQUIRED)
find_package(PROJ REQUIRED CONFIG)

if(GF_BUILD_GUI)
find_package(glfw3 3.3 REQUIRED)
#message(STATUS "GLFW libs: ${GLFW_LIBRARIES}")
endif()

# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads)

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
if(GF_BUILD_GUI_FILE_DIALOGS)
  add_definitions(-DGF_BUILD_GUI_FILE_DIALOGS)
endif()

# version
configure_file(cmake/version.h ${PROJECT_BINARY_DIR}/version.h)

# includes
include_directories(
  src
  thirdparty/filesystem/include
  ${PROJECT_BINARY_DIR}
)

# targets
add_library(geoflow-core SHARED
  src/geoflow/geoflow.cpp
  src/geoflow/common.cpp
  src/geoflow/parameters.cpp
)
target_link_libraries(geoflow-core PRIVATE nlohmann_json::nlohmann_json PROJ::proj)
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
  )
  set(GF_SHADER_FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/viewer/shaders/crosshair.frag 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/viewer/shaders/crosshair.vert 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/viewer/shaders/basic.frag 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/viewer/shaders/basic.vert)
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
    src/geoflow/gui/parameter_widgets.cpp
  )
  
  SET(GF_FILEDIALOG_LIBS "")
  if(GF_BUILD_GUI_FILE_DIALOGS)
    SET(GF_GUI_SOURCES
      ${GF_GUI_SOURCES}
      thirdparty/osdialog/osdialog.c
      src/geoflow/gui/osdialog.cpp
    )
    include_directories(
      thirdparty/osdialog
    )
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
  endif(GF_BUILD_GUI_FILE_DIALOGS)

  add_library(geoflow-gui STATIC ${GF_GUI_SOURCES})
  # target_include_directories(geoflow-gui nlohmann_json::nlohmann_json)
  target_link_libraries(geoflow-gui PRIVATE geoflow-core glfw ${GF_FILEDIALOG_LIBS})
  target_include_directories(geoflow-gui PRIVATE ${GLM_INCLUDE_DIRECTORIES})
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


add_definitions(-DGF_PLUGIN_FOLDER=\"${GF_PLUGIN_FOLDER}\")
message(STATUS "Geoflow plugin folder set to ${GF_PLUGIN_FOLDER}")
message(STATUS "Geoflow plugin folder install set to ${GF_PLUGIN_FOLDER_INSTALL}")

set(GF_PLUGIN_EXTENSION ".so")
if (WIN32)
  set(GF_PLUGIN_EXTENSION ".dll")
endif()
message(STATUS "Setting Geoflow plugin extension to ${GF_PLUGIN_EXTENSION}")
add_definitions(-DGF_PLUGIN_EXTENSION=\"${GF_PLUGIN_EXTENSION}\")

# write the hash of the concatenated shared header files to a new header file, so that we can check compatibility between main geof executable and the plugins
set(GF_SHH_FILE ${CMAKE_BINARY_DIR}/include/geoflow/gfSharedHeadersHash.h)
if(PROJECT_IS_TOP_LEVEL)
  add_custom_command(
    TARGET geoflow-core
    COMMAND ${CMAKE_COMMAND} -DOUTPUT_FILE=${GF_SHH_FILE} -DPROJECT_SOURCE_DIR=${CMAKE_CURRENT_LIST_DIR} -P "${CMAKE_CURRENT_LIST_DIR}/cmake/ComputeSharedHeadersHash.cmake"
  )
else()
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/geoflow/common.hpp s1)
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/geoflow/parameters.hpp s2)
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/geoflow/geoflow.hpp s3)
  string(CONCAT GF_SHARED_HEADERS ${s1} ${s2} ${s3})
  string(MD5 GF_SHARED_HEADERS_HASH ${GF_SHARED_HEADERS})
  message(STATUS "Setting Geoflow shared header hash to ${GF_SHARED_HEADERS_HASH}")
  file(WRITE ${GF_SHH_FILE} "#define GF_SHARED_HEADERS_HASH \"${GF_SHARED_HEADERS_HASH}\"\n")
endif()

  
set_property(TARGET geoflow-core PROPERTY PUBLIC_HEADER
  src/geoflow/common.hpp
  src/geoflow/parameters.hpp
  src/geoflow/geoflow.hpp
  ${GF_SHH_FILE}
)

# target_include_directories(geoflow-core PUBLIC
#   ${CMAKE_BINARY_DIR}/include
# )

# target_sources(geoflow-core PUBLIC
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/geoflow/common.hpp>
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/geoflow/parameters.hpp>
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/geoflow/geoflow.hpp>
#   $<BUILD_INTERFACE:${GF_SHH_FILE}>
#   $<INSTALL_INTERFACE:include/geoflow/common.hpp>
#   $<INSTALL_INTERFACE:include/geoflow/parameters.hpp>
#   $<INSTALL_INTERFACE:include/geoflow/geoflow.hpp>
#   $<INSTALL_INTERFACE:include/geoflow/gfSharedHeadersHash.h>
# )

install(TARGETS geoflow-core EXPORT geoflow-targets 
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/geoflow)
install(EXPORT geoflow-targets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/geoflow)

include ( CMakePackageConfigHelpers )
configure_package_config_file(cmake/geoflow-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/geoflow-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/geoflow
)

install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/geoflow-config.cmake 
  cmake/gf_plugin.cpp.in
  cmake/geoflow_create_plugin.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/geoflow)

install(FILES LICENSE DESTINATION share/geoflow)

add_subdirectory(apps)

include(cmake/geoflow_create_plugin.cmake)
