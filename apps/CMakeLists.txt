# gui application
if(${GF_BUILD_GUI})
  if(APPLE)
    set(RESOURCE_FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/resources/AppIcon.icns
    )
    add_executable(geoflow geoflow-gui.cpp ${RESOURCE_FILES})
    set_target_properties(geoflow PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist
      RESOURCE ${RESOURCE_FILES}
    )
  elseif(WIN32)
    set(RESOURCE_FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon.rc
    )
    add_executable(geoflow geoflow-gui.cpp ${RESOURCE_FILES}) # add WIN32 as second parameter to make it a GUI app and suppress console window on execution
  else()
    add_executable(geoflow geoflow-gui.cpp)
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR LINUX)
    target_link_libraries(geoflow PRIVATE -ldl)
  endif()
  target_link_libraries(geoflow PRIVATE geoflow-gui nlohmann_json::nlohmann_json PROJ::proj glfw)
  message("${CMAKE_BINARY_DIR}/include")
  # last include dir is for the shared header hash|
  target_include_directories(geoflow PRIVATE ${GLM_INCLUDE_DIRECTORIES} ${CMAKE_BINARY_DIR}/include)
  target_link_directories(geoflow PRIVATE ${CMAKE_SOURCE_DIR}/src)
  set_target_properties( geoflow PROPERTIES CXX_STANDARD 17 )
  if (LINUX)
    target_compile_options( geoflow PRIVATE -Wl,-E ) # https://gcc.gnu.org/faq.html#dso
  endif()
  install(FILES ${GF_SHADER_FILES} DESTINATION ${GF_SHADER_PATH})
  install(TARGETS geoflow
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION /Applications)
endif()

# cli application
add_executable(geof geoflow-app.cpp)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR LINUX)
  target_link_libraries(geof PRIVATE -ldl)
endif()
target_link_libraries(geof PRIVATE geoflow-core nlohmann_json::nlohmann_json PROJ::proj)
target_link_directories(geof PRIVATE ${CMAKE_SOURCE_DIR}/src)
# last include dir is for the shared header hash|
target_include_directories(geof PRIVATE ${CMAKE_BINARY_DIR}/include)
set_target_properties( geof PROPERTIES CXX_STANDARD 17 )
if (LINUX)
  target_compile_options( geof PRIVATE -Wl,-E ) # https://gcc.gnu.org/faq.html#dso
endif()

install(TARGETS geof
  RUNTIME DESTINATION bin)
