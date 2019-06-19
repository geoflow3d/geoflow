#include <iostream>
#include <fstream>

#include "DLLoader.h"

#include <geoflow/core/geoflow.hpp>
#ifdef GF_BUILD_GUI
    #include <geoflow/gui/flowchart.hpp>
#endif


int main(int ac, const char * av[]) {

  const std::string config_path = "/Users/ravi/git/geoflow/apps/geoflow-config.json";
  const std::string flowchart_path = "/Users/ravi/git/geoflow/apps/add.json";

  typedef dlloader::DLLoader<geoflow::NodeRegister> DLLoader;
  std::unordered_map<std::string, std::unique_ptr<DLLoader>> dloaders;

  // load node registers from libraries
  {
    geoflow::NodeRegisterMap node_registers;
    #ifdef GF_BUILD_GUI
      auto R = geoflow::NodeRegister::create("Visualisation");
      R->register_node<geoflow::nodes::gui::ColorMapperNode>("ColorMapper");
      R->register_node<geoflow::nodes::gui::GradientMapperNode>("GradientMapper");
      R->register_node<geoflow::nodes::gui::PainterNode>("Painter");
      node_registers.emplace(R);
      
      ImGui::CreateContext();
    #endif
    
    json j;
    std::ifstream i(config_path);
    i >> j;
    auto node_paths = j.at("node-libs").get<std::vector<std::string>>();

    for (const auto& path : node_paths)  {
      dloaders.emplace(path, std::make_unique<DLLoader>(path));
      // auto lib = ;
      std::cout << "Loading " << path << std::endl;
      dloaders[path]->DLOpenLib();

      #ifdef GF_BUILD_GUI
        dloaders[path]->DLSetImGuiContext(ImGui::GetCurrentContext());
      #endif

      node_registers.emplace( dloaders[path]->DLGetInstance() );
    }

    // load flowchart from file
    geoflow::NodeManager node_manager;
    // node_manager.load_json(flowchart_path, node_registers);

    // launch gui or just run the flowchart in cli mode
    #ifdef GF_BUILD_GUI
      geoflow::launch_flowchart(node_manager, node_registers);
    #else
      //N.run(*las_loader);
    #endif
  }

  // unload node libraries
  for ( auto& [path, loader] : dloaders) {
    std::cout << "Unloading " << path << std::endl;
    loader->DLCloseLib();
  }
}