# gui application
if(${GF_BUILD_GUI})
  if(APPLE)
    set(RESOURCE_FILES
      ${CMAKE_SOURCE_DIR}/resources/AppIcon.icns
    )
    add_executable(geoflow geoflow-gui.cpp ${RESOURCE_FILES})
    set_target_properties(geoflow PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/Info.plist
    RESOURCE ${RESOURCE_FILES}
    )
  else()
    add_executable(geoflow geoflow-gui.cpp)
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(geoflow PRIVATE -ldl)
  endif()
  target_link_libraries(geoflow PRIVATE geoflow-gui nlohmann_json::nlohmann_json glm)
  target_link_directories(geoflow PRIVATE ${CMAKE_SOURCE_DIR}/src)
  set_target_properties( geoflow PROPERTIES CXX_STANDARD 17 )
  install(FILES ${GF_SHADER_FILES} DESTINATION ${GF_SHADER_PATH})
  install(TARGETS geoflow
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION /Applications)
endif()

# cli application
add_executable(geof geoflow-app.cpp)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(geof PRIVATE -ldl)
endif()
target_link_libraries(geof PRIVATE geoflow-core nlohmann_json::nlohmann_json)
target_link_directories(geof PRIVATE ${CMAKE_SOURCE_DIR}/src)
set_target_properties( geof PROPERTIES CXX_STANDARD 17 )

install(TARGETS geof
  RUNTIME DESTINATION bin)