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

  # this is an unfortunate requirement due to how parameters currently work. Should get rid of it.
  find_package(nlohmann_json 3.10.5 CONFIG REQUIRED)
  target_link_libraries( ${GF_PLUGIN_TARGET_NAME} PRIVATE nlohmann_json::nlohmann_json )

  install(
    TARGETS ${GF_PLUGIN_TARGET_NAME} 
    LIBRARY DESTINATION ${GF_PLUGIN_FOLDER}
  )
endfunction()