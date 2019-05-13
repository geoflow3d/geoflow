
set (geoflowlib geoflow-core)
if(${GF_BUILD_GUI})
  set (geoflowlib geoflow-gui)
endif()

add_executable(gf gf.cpp)
target_link_libraries(gf ${geoflowlib})

set_target_properties(
  gf
  PROPERTIES CXX_STANDARD 17
)