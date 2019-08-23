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

#include <DLLoader.h>
#include "CLI11.hpp"

#include <geoflow/geoflow.hpp>
#include <geoflow/core_nodes.hpp>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#ifdef GF_BUILD_GUI
    #include <geoflow/gui/gfImNodes.hpp>
#endif

namespace fs = std::filesystem;

int main(int argc, const char * argv[]) {

  const std::string config_path = "/Users/ravi/git/geoflow/apps/geoflow-config.json";
  std::string flowchart_path = "/Users/ravi/git/geoflow/apps/add.json";


  CLI::App cli{"Geoflow"};

  CLI::Option* opt_flowchart_path = cli.add_option("-f,--flowchart", flowchart_path, "Flowchart file");

  try {
    cli.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return cli.exit(e);
  }

  typedef dlloader::DLLoader<geoflow::NodeRegister> DLLoader;
  std::unordered_map<std::string, std::unique_ptr<DLLoader>> dloaders;

  // load node registers from libraries
  {
    geoflow::NodeRegisterMap node_registers;
    auto R_core = geoflow::NodeRegister::create("Core");
    R_core->register_node<geoflow::nodes::core::NestNode>("NestedFlowchart");
    node_registers.emplace(R_core);
    #ifdef GF_BUILD_GUI
      auto R_gui = geoflow::NodeRegister::create("Visualisation");
      R_gui->register_node<geoflow::nodes::gui::ColorMapperNode>("ColorMapper");
      R_gui->register_node<geoflow::nodes::gui::GradientMapperNode>("GradientMapper");
      R_gui->register_node<geoflow::nodes::gui::PainterNode>("Painter");
      R_gui->register_node<geoflow::nodes::gui::VectorPainterNode>("VectorPainter");
      R_gui->register_node<geoflow::nodes::gui::CubeNode>("Cube");
      R_gui->register_node<geoflow::nodes::gui::TriangleNode>("Triangle");
      node_registers.emplace(R_gui);
      
      ImGui::CreateContext();
    #endif
    
    for(auto& p: fs::directory_iterator(GF_PLUGIN_FOLDER)) {
        if (p.path().extension() == GF_PLUGIN_EXTENSION) {
          std::string path = p.path().string();
          std::cout << "Loading " << path << " ...\n";
          dloaders.emplace(path, std::make_unique<DLLoader>(path));
          if (dloaders[path]->DLOpenLib()) {
            node_registers.emplace( dloaders[path]->DLGetInstance() );
            std::cout << "... success :)\n";
          } else {
            dloaders.erase(path);
            std::cout << "... failed :(\n";
          }
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
    geoflow::NodeManager node_manager(node_registers);
    if(*opt_flowchart_path)
      node_manager.load_json(flowchart_path);

    // launch gui or just run the flowchart in cli mode
    #ifdef GF_BUILD_GUI
      geoflow::launch_flowchart(node_manager);
    #else
      //N.run();
    #endif
  }

  // unload node libraries
  for ( auto& [path, loader] : dloaders) {
    std::cout << "Unloading " << path << std::endl;
    loader->DLCloseLib();
  }

  return 0;
}