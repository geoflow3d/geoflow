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
#include <cstdlib>

#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include)
  #if __has_include(<filesystem>)
    #define GHC_USE_STD_FS
    #include <filesystem>
    namespace fs = std::filesystem;
  #endif
#endif
#ifndef GHC_USE_STD_FS
  #include <ghc/filesystem.hpp>
  namespace fs = ghc::filesystem;
#endif

#include <geoflow/geoflow.hpp>
#include <geoflow/plugin_manager.hpp>

#ifdef GF_BUILD_WITH_GUI
  #include <geoflow/gui/gfImNodes.hpp>
  #include <geoflow/core_nodes.hpp>
#else 
  #include <geoflow/core_nodes.hpp>
#endif

#include "CLI11.hpp"

using namespace geoflow;

int main(int argc, const char * argv[]) {

  std::string flowchart_path = "flowchart.json";
  std::string plugin_folder = GF_PLUGIN_FOLDER;
  std::string log_filename = "";
  
  if(const char* env_p = std::getenv("GF_PLUGIN_FOLDER")) {
    plugin_folder = env_p;
    std::cout << "Detected environment variable GF_PLUGIN_FOLDER = " << plugin_folder << "\n";
  }

  CLI::App cli{"Geoflow"};

  CLI::Option* opt_flowchart_path = cli.add_option("-f,--flowchart", flowchart_path, "Flowchart file");
  CLI::Option* opt_plugin_folder = cli.add_option("-p,--plugin-folder", plugin_folder, "Plugin folder");
  CLI::Option* opt_log = cli.add_option("-l,--log", log_filename, "Write log to file");
  
  try {
    cli.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return cli.exit(e);
  }

  std::ofstream logfile; 
  auto cout_rdbuf = std::cout.rdbuf();
  auto cerr_rdbuf = std::cerr.rdbuf(); 
  if(*opt_log) {
    logfile.open(log_filename);
    std::cout.rdbuf(logfile.rdbuf());
    std::cerr.rdbuf(logfile.rdbuf());
  }

  // load node registers from libraries
  PluginManager plugin_manager;
  {
    NodeRegisterMap node_registers;
    auto R_core = NodeRegister::create("Core");
    R_core->register_node<nodes::core::NestNode>("NestedFlowchart");
    node_registers.emplace(R_core);

    #ifdef GF_BUILD_WITH_GUI
      auto R_gui = NodeRegister::create("Visualisation");
      R_gui->register_node<nodes::gui::ColorMapperNode>("ColorMapper");
      R_gui->register_node<nodes::gui::GradientMapperNode>("GradientMapper");
      R_gui->register_node<nodes::gui::PainterNode>("Painter");
      R_gui->register_node<nodes::gui::VectorPainterNode>("VectorPainter");
      R_gui->register_node<nodes::gui::CubeNode>("Cube");
      R_gui->register_node<nodes::gui::TriangleNode>("Triangle");
      node_registers.emplace(R_gui);

      ImGui::CreateContext();
    #endif

    if(fs::exists(plugin_folder)) {
      plugin_manager.load(plugin_folder, node_registers);
    } else {
      std::cout << "Notice that this plugin folder does not exist: " << plugin_folder << "\n";
    }
    // load flowchart from file
    NodeManager node_manager(node_registers);
    if(*opt_flowchart_path) {
      node_manager.load_json(flowchart_path);
    }
    // launch gui or just run the flowchart in cli mode
    #ifdef GF_BUILD_WITH_GUI
      launch_flowchart(node_manager);
    #else
      node_manager.run();
    #endif
  }
  // NOTICE that we first must destroy any related node_registers before we can unload the plugin_manager!
  plugin_manager.unload();

  std::cout.rdbuf(cout_rdbuf);
  std::cerr.rdbuf(cerr_rdbuf);
  
  return 0;
}
