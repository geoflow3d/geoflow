

add_executable(
  gui 
  gui.cpp 
)
target_link_libraries(gui geoflow-gui)
# add_executable(
#   viewer-test 
#   src/viewer/main.cpp 
# )
# target_link_libraries(viewer-test viewer)

set_target_properties(
  viewer
  geoflow-gui
  gui 
  PROPERTIES CXX_STANDARD 17
)

add_library(basic_plugin SHARED
  basic_plugin.cpp
)
target_link_libraries(basic_plugin geoflow-core)

add_executable(basic_dynamic basic_dynamic.cpp)
target_link_libraries(basic_dynamic geoflow-core)

add_executable(basic basic.cpp)
target_link_libraries(basic geoflow-core)

set_target_properties(
geoflow-core
basic
basic_plugin
basic_dynamic
PROPERTIES CXX_STANDARD 17
)