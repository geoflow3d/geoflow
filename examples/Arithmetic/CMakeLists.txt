
add_definitions(-DGF_PLUGIN_NAME=\"Arithmetic\")

add_library(gf_arithmetic SHARED
  plugin.cpp
)
target_link_libraries(gf_arithmetic geoflow-core)

# add_executable(demo_dynamic demo_dynamic.cpp)
# target_link_libraries(demo_dynamic geoflow-core)

# add_executable(demo_simple demo_simple.cpp)
# target_link_libraries(demo_simple geoflow-core)

set_target_properties(
gf_arithmetic
# demo_simple
# demo_dynamic
PROPERTIES CXX_STANDARD 17
)