// This file is part of Geoflow
// Copyright (C) 2018-2019  Ravi Peters, 3D geoinformation TU Delft

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <fstream>
#include <filesystem>

#include "DLLoader.h"

#include <geoflow/geoflow.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#ifdef GF_BUILD_GUI
    #include <geoflow/gui/flowchart.hpp>
#endif

namespace fs = std::filesystem;

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
    
    for(auto& p: fs::directory_iterator(GF_PLUGIN_FOLDER)) {
        if (p.path().extension() == ".so") {
          std::string path = p.path().string();
          std::cout << "Loading " << path << "\n";
          dloaders.emplace(path, std::make_unique<DLLoader>(path));
          dloaders[path]->DLOpenLib();
          node_registers.emplace( dloaders[path]->DLGetInstance() );
        }
    }

    // {   
    //   json j;
    //   std::ifstream i(config_path);
    //   i >> j;
    //   auto node_paths = j.at("node-libs").get<std::vector<std::string>>();

    //   for (const auto& path : node_paths)  {
    //     dloaders.emplace(path, std::make_unique<DLLoader>(path));
    //     // auto lib = ;
    //     std::cout << "Loading " << path << std::endl;
    //     dloaders[path]->DLOpenLib();

    //     node_registers.emplace( dloaders[path]->DLGetInstance() );
    //   }
    // }
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