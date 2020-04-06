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
#include <utility>

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

// load node registers from libraries
void load_plugins(PluginManager& plugin_manager, NodeRegisterMap& node_registers, std::string& plugin_dir) {
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

  if(fs::exists(plugin_dir)) {
    plugin_manager.load(plugin_dir, node_registers);
  } else {
    std::cout << "Notice that this plugin folder does not exist: " << plugin_dir << "\n";
  }
}

int main(int argc, const char * argv[]) {

  std::string flowchart_path = "flowchart.json";
  std::string plugin_folder = GF_PLUGIN_FOLDER;
  std::string log_filename = "";
  
  if(const char* env_p = std::getenv("GF_PLUGIN_FOLDER")) {
    plugin_folder = env_p;
    std::cout << "Detected environment variable GF_PLUGIN_FOLDER = " << plugin_folder << "\n";
  }

  PluginManager plugin_manager;
  auto cout_rdbuf = std::cout.rdbuf();
  auto cerr_rdbuf = std::cerr.rdbuf(); 
  {
    NodeRegisterMap node_registers;
    NodeManager flowchart(node_registers);

    CLI::App cli{"Geoflow"};

    // CLI::Option* opt_plugin_folder = cli.add_option("-p,--plugin-folder", plugin_folder, "Plugin folder");
    CLI::Option* opt_log = cli.add_option("-l,--log", log_filename, "Write log to file");

    auto sc_flowchart = cli.add_subcommand("", "Load flowchart");
    CLI::Option* opt_flowchart_path = sc_flowchart->add_option("flowchart", flowchart_path, "Flowchart file");
    #ifndef GF_BUILD_WITH_GUI
      opt_flowchart_path->required();
    #endif
    auto sc_info = cli.add_subcommand("info", "Print info")->excludes(sc_flowchart);

    sc_info->parse_complete_callback([&plugin_manager, &node_registers, &plugin_folder](){
      load_plugins(plugin_manager, node_registers, plugin_folder);
    });
    
    std::map<std::string, std::vector<std::string>> globals_from_cli;
    sc_flowchart->parse_complete_callback([&](){
      load_plugins(plugin_manager, node_registers, plugin_folder);

      // load flowchart from file
      if(*opt_flowchart_path) {
        // set current work directory to folder containing flowchart file
        auto abs_path = fs::absolute(fs::path(flowchart_path));
        fs::current_path(abs_path.parent_path());
        flowchart_path = abs_path.string();
        if(fs::exists(abs_path)) {
          flowchart.load_json(flowchart_path);
        }
      }
    });

    // handle cli globals
    CLI::App sc_globals{"globals", "Set flowchart globals"};
    cli.allow_extras();
    cli.callback([&](){
      // allow user to pass globals through a toml file. 
      sc_globals.set_config("--config,-c", "", "Read flowchart globals from a config file");
      for (auto&[key,val] : flowchart.global_flowchart_params) {
        auto [it, inserted] = globals_from_cli.emplace(std::make_pair(key, vec1s{val}));
        sc_globals.add_option("--"+key, (it->second), "");
        std::cout << "add global option " << key << "\n";
      }
      sc_globals.parse(cli.remaining_for_passthrough());
    });

    try {
      cli.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
      return cli.exit(e);
    }
    // if(*opt_plugin_folder) {
    //   std::cout << "Setting plugin folder to " << plugin_folder << "\n";
    // }
    // read global values from cli
    if(sc_globals.count()) {
      for (auto& [key, values] : globals_from_cli) {
        std::string concat_values{};
        for (auto& value : values) {
          concat_values += value + " ";
        }
        concat_values.pop_back();
        std::cout << "global " << key << " = " << concat_values << "\n";
        flowchart.global_flowchart_params[key]=concat_values;
      }
    }

    std::ofstream logfile;
    if(*opt_log) {
      logfile.open(log_filename);
      std::cout.rdbuf(logfile.rdbuf());
      std::cerr.rdbuf(logfile.rdbuf());
    }

    // launch gui or just run the flowchart in cli mode
    #ifdef GF_BUILD_WITH_GUI
      if(!*opt_flowchart_path) {
        load_plugins(plugin_manager, node_registers, plugin_folder);
        // set current work directory to folder containing flowchart file
        auto abs_path = fs::absolute(fs::path(flowchart_path));
        fs::current_path(abs_path.parent_path());
        flowchart_path = abs_path.string();
        if(fs::exists(abs_path)) {
          flowchart.load_json(flowchart_path);
        }
      }
      launch_flowchart(flowchart, flowchart_path);
    #else
      flowchart.run_all();
    #endif
  }
  // NOTICE that we first must destroy any related node_registers before we can unload the plugin_manager!
  plugin_manager.unload();

  std::cout.rdbuf(cout_rdbuf);
  std::cerr.rdbuf(cerr_rdbuf);
  
  return 0;
}
