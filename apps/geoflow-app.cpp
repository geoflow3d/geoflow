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
#include "version.h"

#ifdef GF_BUILD_WITH_GUI
  #include <geoflow/gui/gfImNodes.hpp>
  #include <geoflow/core_nodes.hpp>
#else 
  #include <geoflow/core_nodes.hpp>
#endif

#include "CLI11.hpp"

using namespace geoflow;

// load node registers from libraries
void load_plugins(PluginManager& plugin_manager, NodeRegisterMap& node_registers, std::string& plugin_dir, bool verbose=false) {
  auto R_core = NodeRegister::create("Core");
  R_core->register_node<nodes::core::NestNode>("NestedFlowchart");
  R_core->register_node<nodes::core::IntNode>("Int");
  R_core->register_node<nodes::core::FloatNode>("Float");
  R_core->register_node<nodes::core::BoolNode>("Bool");
  R_core->register_node<nodes::core::TextNode>("Text");
  R_core->register_node<nodes::core::TextWriterNode>("TextWriter");
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
    plugin_manager.load(plugin_dir, node_registers, verbose);
  } else {
    std::cout << "Plugin folder does not exist: " << plugin_dir << "\n";
  }
}
void print_version() {
  std::cout << "Geoflow " << PROJECT_VERSION_MAJOR;
  std::cout << "." << PROJECT_VERSION_MINOR;
  std::cout << "." << PROJECT_VERSION_PATCH; std::cout << std::endl;
  // std::cout << "host_system : " << CMAKE_HOST_SYSTEM; std::cout << std::endl;
  std::cout << "compiled with " << CMAKE_CXX_COMPILER_ID;
  std::cout << " v" << CMAKE_CXX_COMPILER_VERSION;
  std::cout << " (" << CMAKE_BUILD_TYPE;
  std::cout << ")" << std::endl;
}
void print_plugins(NodeRegisterMap& node_registers) {
  if (node_registers.size()) {
    // std::cout << "Available plugins:\n";
    for (auto& [rname, reg] : node_registers) {
      auto& rinfo = reg->get_plugin_info();
      std::cout << " > " << rname;
      if (rinfo.size()) {
        std::cout << " " << rinfo["version_major"]
        << "." << rinfo["version_minor"] << "." << rinfo["version_patch"]
        << std::endl;
        std::cout << "   compiled with " << rinfo["compiler_id"]
        << " v" << rinfo["compiler_version"];
        if (rinfo["build_type"] != "") {
          std::cout << " (" << rinfo["build_type"];
          std::cout << ")";
        }
        std::cout << std::endl;
      } else {
        std::cout << " [built-in]\n";
      }
    }
  }
}
void print_nodes(NodeRegisterMap& node_registers) {
  if (node_registers.size()) {
    for (auto& [rname, reg] : node_registers) {
      std::cout << "Available nodes for plugin " << rname << ":\n";
      for (auto& [nname, val] : reg->node_types) {
        std::cout << " > " << nname << "\n";
      }
    }
  }
}

int main(int argc, const char * argv[]) {
  std::string flowchart_path = "flowchart.json";
  std::string plugin_folder = GF_PLUGIN_FOLDER;
  std::string log_filename = "";
  fs::path launch_path{fs::current_path()};
  fs::path flowchart_folder = launch_path;
  
  if(const char* env_p = std::getenv("GF_PLUGIN_FOLDER")) {
    plugin_folder = env_p;
    std::cout << "Detected environment variable GF_PLUGIN_FOLDER = " << plugin_folder << "\n";
  }

  PluginManager plugin_manager;
  bool verbose{false};

  {
    NodeRegisterMap node_registers;
    NodeManager flowchart(node_registers);
    load_plugins(plugin_manager, node_registers, plugin_folder, verbose);

    CLI::App cli{"Geoflow"};

    // CLI::Option* opt_plugin_folder = cli.add_option("-p,--plugin-folder", plugin_folder, "Plugin folder");
    // if(*opt_plugin_folder) {
    //   std::cout << "Setting plugin folder to " << plugin_folder << "\n";
    // }
    cli.add_flag("--verbose", verbose, "Print verbose messages");
    auto info_options = cli.add_option_group("Info", "Debug information");

    auto version_flag = info_options->add_flag("--version,-v", "Print version information");
    auto plugins_flag = info_options->add_flag("--plugins,-p", "List available plugins");
    auto nodes_flag = info_options->add_flag("--nodes,-n", "List available nodes from plugins that are loaded");

    // CLI::Option* opt_log = cli.add_option("-l,--log", log_filename, "Write log to file");
    // opt_log->check([](const std::string& s)->std::string {
    //   if(!fs::exists(fs::absolute(fs::path(s)).parent_path())) {
    //     return std::string("Path to log file does not exist");
    //   } else return std::string();
    // });

    auto run_subcommand = cli.add_subcommand("run", "Load and run flowchart");
    auto opt_flowchart_path = run_subcommand->add_option("flowchart", flowchart_path, "Flowchart file");
    auto opt_list_globals = run_subcommand->add_flag("--globals,-g", "List flowchart globals. Skips flowchart execution");
    auto opt_workdir = run_subcommand->add_flag("--workdir,-w", "Set working directory to folder containing flowchart file");
    opt_flowchart_path->check(CLI::ExistingFile);

    opt_flowchart_path->required();
    run_subcommand->fallthrough();

    auto config_subcommand = cli.add_subcommand("set", "Set flowchart globals (comes after run)");
    config_subcommand->needs(run_subcommand);
    config_subcommand->set_config("--config,-c", "", "Read globals from config file");
    config_subcommand->allow_extras(); // for globals

    // CLI11 Callbacks
    info_options->parse_complete_callback([&]() -> void {
      // std::cout << "info_options->parse_complete_callback\n";

      if(*version_flag) {
        print_version();
      }

      if(*plugins_flag) {
        print_plugins(node_registers);
      }

      if(*nodes_flag) {
        print_nodes(node_registers);
      }
    });
    std::map<std::string, std::vector<std::string>> globals_from_cli;
    run_subcommand->parse_complete_callback([&]() -> void {
      // std::cout << "run_subcommand->parse_complete_callback\n";
      // load flowchart from file
      if(*opt_flowchart_path) {
        // set current work directory to folder containing flowchart file
        auto abs_path = fs::absolute(fs::path(flowchart_path));
        flowchart_folder = abs_path.parent_path();
        flowchart_path = abs_path.string();
        // fs::current_path(flowchart_folder);
        flowchart.load_json(flowchart_path);
        // fs::current_path(launch_path);
      }

      // handle globals overrides provided by user
      if (*opt_list_globals) std::cout << "Available globals:\n";
      for (auto&[key,val] : flowchart.global_flowchart_params) {
        auto [it, inserted] = globals_from_cli.emplace(std::make_pair(key, vec1s{}));
        config_subcommand->add_option("--"+key, (it->second), "");
        if (*opt_list_globals) {
          std::cout << " > " << key;
          if (!val->get_help().empty()) std::cout << " [" << val->get_help() << "]";
          std::cout << "\n   default value: " << val->as_json() << "\n";
        }
      }
      // sc_globals.parse(run_subcommand->remaining_for_passthrough());
    });
    config_subcommand->preparse_callback([&](size_t n) -> void {
      // std::cout << "config_subcommand->preparse_callback\n";
    });
    config_subcommand->parse_complete_callback([&]() -> void {
      // std::cout << "config_subcommand->parse_complete_callback\n";
      // process global values from cli/config file
      if(config_subcommand->count()) {
        for (auto& [key, values] : globals_from_cli) {
          if (values.size()) {
            std::string concat_values{};
            for (auto& value : values) {
              concat_values += value + " ";
            }
            concat_values.pop_back();
            std::cout << "set global " << key << " = " << concat_values << "\n";
            
            auto& g = flowchart.global_flowchart_params[key];
            try{
              if (g->is_type(typeid(std::string))) {
                auto* gptr = static_cast<ParameterByValue<std::string>*>(g.get());
                gptr->set(concat_values);
              } else if (g->is_type(typeid(float))) {
                auto* gptr = static_cast<ParameterByValue<float>*>(g.get());
                gptr->set(std::stof(concat_values));
              } else if(g->is_type(typeid(int))) {
                auto* gptr = static_cast<ParameterByValue<int>*>(g.get());
                gptr->set(std::stoi(concat_values));
              } else if(g->is_type(typeid(bool))) {
                auto* gptr = static_cast<ParameterByValue<bool>*>(g.get());
                if(concat_values == "true")
                  gptr->set(true);
                else if(concat_values == "false")
                  gptr->set(false);
                else throw gfFlowchartError("failed to get boolean from string");
              }
            } catch (const std::exception& e) {
              std::cerr << "Error in parsing global parameters\n";
              std::cerr << e.what();
              throw;
            }
          }
        }
      }
    });

    try {
      cli.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
      return cli.exit(e);
    }
    
    #ifndef GF_BUILD_WITH_GUI
      bool no_arguments = !(*run_subcommand) && !(*version_flag) && !(*nodes_flag) && !(*plugins_flag) && !(verbose);
      if(no_arguments) {
        std::cout << cli.help() << std::flush;
        return 1;
      }
    #endif

    if( ! *opt_list_globals ) {
      if( ! verbose ) {
        std::clog.setstate(std::ios_base::failbit);
        std::cout.setstate(std::ios_base::failbit);
        // std::cerr.setstate(std::ios_base::failbit);
      }

      // launch gui or just run the flowchart in cli mode
      if(*opt_workdir) fs::current_path(flowchart_folder);
      #ifdef GF_BUILD_WITH_GUI
        try {
        launch_gui(flowchart, flowchart_path);
        }
        catch (const gfException& e) {
          // std::cerr.clear();
          std::cerr << e.what() << "\n";
          throw;
        }
      #else
        try {
          if (*run_subcommand) flowchart.run_all();
        }
        catch (const gfException& e) {
          // std::cerr.clear();
          std::cerr << e.what() << "\n";
          throw;
        }
      #endif
      fs::current_path(launch_path);

      if( ! verbose ) {
        std::clog.clear();
        std::cout.clear();
        // std::cerr.clear();
      }
    }
    // auto cout_rdbuf = std::cout.rdbuf();
    // auto cerr_rdbuf = std::cerr.rdbuf();
    // std::ofstream logfile;
    // if(*opt_log) {
    //   logfile.open(log_filename);
    //   std::cout.rdbuf(logfile.rdbuf());
    //   std::cerr.rdbuf(logfile.rdbuf());
    // }
  }

  // NOTICE that we first must destroy any related node_registers before we can unload the plugin_manager!
  plugin_manager.unload(verbose);

  // std::cout.rdbuf(cout_rdbuf);
  // std::cerr.rdbuf(cerr_rdbuf);
  
  return 0;
}
