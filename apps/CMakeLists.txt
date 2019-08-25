
set (geoflowlib geoflow-core)
if(${GF_BUILD_GUI})
  set (geoflowlib geoflow-gui)
endif()

add_executable(geof geof.cpp)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(geof PRIVATE -ldl)
endif()
target_link_libraries(geof PRIVATE ${geoflowlib} nlohmann_json::nlohmann_json glm)
target_link_directories(geof PRIVATE ${CMAKE_SOURCE_DIR}/src)
set_target_properties( geof PROPERTIES CXX_STANDARD 17 )

if(${GF_BUILD_GUI})
  install(FILES ${GF_SHADER_FILES} DESTINATION ${GF_SHADER_PATH})
endif()
install(TARGETS geof RUNTIME)