
set (geoflowlib geoflow-core)
if(${GF_BUILD_GUI})
  set (geoflowlib geoflow-gui)
endif()

add_executable(gf gf.cpp)
target_link_libraries(gf PRIVATE ${geoflowlib} nlohmann_json::nlohmann_json)


set_target_properties(
  gf
  PROPERTIES CXX_STANDARD 17
)

if(${GF_BUILD_GUI})
  install(FILES ${GF_SHADER_FILES} DESTINATION ${GF_SHADER_PATH})
endif()

install(TARGETS gf RUNTIME)