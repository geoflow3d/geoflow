
set (geoflowlib geoflow-core)
if(${GF_BUILD_GUI})
  set (geoflowlib geoflow-gui)
endif()

add_executable(gf gf.cpp)
target_link_libraries(gf ${geoflowlib})

# target_link_options(
#   gf
#   PUBLIC
#   -Wl,-export_dynamic
# )

set_target_properties(
  gf
  PROPERTIES CXX_STANDARD 17
)