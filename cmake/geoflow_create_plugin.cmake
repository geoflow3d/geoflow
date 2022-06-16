function(geoflow_create_plugin)
  configure_file(${GF_PLUGIN_FILE} ${PROJECT_BINARY_DIR}/gf_plugin.cpp)

  add_library(${GF_PLUGIN_TARGET_NAME} MODULE
    ${PROJECT_BINARY_DIR}/gf_plugin.cpp
    ${ARGN}
  )
  set_target_properties(
    ${GF_PLUGIN_TARGET_NAME} PROPERTIES 
    CXX_STANDARD 17
    CXX_VISIBILITY_PRESET hidden
    PREFIX ""
  )

  install(
    TARGETS ${GF_PLUGIN_TARGET_NAME} 
    LIBRARY DESTINATION lib/geoflow-plugins
  )
endfunction()