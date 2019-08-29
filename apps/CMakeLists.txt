
set (geoflowlib geoflow-core)
if(${GF_BUILD_GUI})
  set (geoflowlib geoflow-gui)
endif()


if(APPLE AND ${GF_BUILD_GUI})
  set(RESOURCE_FILES
    ${CMAKE_SOURCE_DIR}/resources/AppIcon.icns
  )
  add_executable(geoflow geof.cpp ${RESOURCE_FILES})
  set_target_properties(geoflow PROPERTIES
  MACOSX_BUNDLE TRUE
  MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/Info.plist
  RESOURCE ${RESOURCE_FILES}
  )
else()
  add_executable(geoflow geof.cpp)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(geoflow PRIVATE -ldl)
endif()

target_link_libraries(geoflow PRIVATE ${geoflowlib} nlohmann_json::nlohmann_json glm)
target_link_directories(geoflow PRIVATE ${CMAKE_SOURCE_DIR}/src)
set_target_properties( geoflow PROPERTIES CXX_STANDARD 17 )

if(${GF_BUILD_GUI})
  install(FILES ${GF_SHADER_FILES} DESTINATION ${GF_SHADER_PATH})
endif()
install(TARGETS geoflow 
  RUNTIME
  BUNDLE DESTINATION /Applications)