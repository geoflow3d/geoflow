// This file is part of Geoflow
// Copyright (C) 2018-2022 Ravi Peters

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

#include <geoflow/geoflow.hpp>
#include <geoflow/plugin_manager.hpp>
#include "version.h"

#ifdef GF_BUILD_WITH_GUI
  #include <geoflow/gui/gfImNodes.hpp>
  #include <geoflow/core_nodes.hpp>
#else 
  #include <geoflow/core_nodes.hpp>
#endif

#include "argh.h"
#include "toml.hpp"

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
  R_core->register_node<nodes::core::ProjTesterNode>("ProjTester");
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

void print_help(std::string program_name) {
  // see http://docopt.org/
  std::cout << "Usage: \n";
  std::cout << "   " << program_name;
  std::cout << " [-v|-p|-n|-h]\n";
  std::cout << "   " << program_name;
  std::cout << " <flowchart_file> [-V] [-g] [-w] [-c <file>] [--GLOBAL1=A --GLOBAL2=B ...]\n";
  std::cout << "\n";
  std::cout << "Options:\n";
  std::cout << "   -v, --version                Print version information\n";
  std::cout << "   -p, --list-plugins           List available plugins\n";
  std::cout << "   -n, --list-nodes             List available nodes for plugins that are loaded\n";
  std::cout << "   -h, --help                   Print this help message\n";
  std::cout << "\n";
  std::cout << "   <flowchart_file>             JSON flowchart file\n";
  std::cout << "   -V, --verbose                Print verbose messages during flowchart execution\n";
  std::cout << "   -g, --list-globals           List available flowchart globals. Cancels flowchart execution\n";
  std::cout << "   -w, --workdir                Set working directory to folder containing flowchart file\n";
  std::cout << "   -c <file>, --config <file>   Read globals from TOML config file\n";
  std::cout << "   --GLOBAL1=A --GLOBAL2=B ...  Specify globals for flowchart (list availale globals with -g)\n";
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

  auto cmdl = argh::parser({ "-c", "--config" });
  cmdl.parse(argc, argv);
  std::string program_name = cmdl[0];

  if(cmdl[{ "-v", "--version" }]) {
    print_version();
    return EXIT_SUCCESS;
  }

  if(cmdl[{ "-h", "--help" }]) {
    print_help(program_name);
    return EXIT_SUCCESS;
  }

  PluginManager plugin_manager;
  bool verbose{false};
  {
    NodeRegisterMap node_registers;
    NodeManager flowchart(node_registers);
    
    verbose = cmdl[{ "-V", "--verbose" }];
    load_plugins(plugin_manager, node_registers, plugin_folder, verbose);

    if(cmdl[{ "-p", "--list-plugins" }]) {
      std::cout << "GF_PLUGIN_FOLDER = " << plugin_folder << "\n";
      print_plugins(node_registers);
      return EXIT_SUCCESS;
    } else if(cmdl[{ "-n", "--list-nodes" }]) {
      print_nodes(node_registers);
      return EXIT_SUCCESS;
    }

    // check for flowchart
    std::map<std::string, std::vector<std::string>> globals_from_cli;
    bool list_globals{false};
    if(cmdl[1].empty()) {
      #ifndef GF_BUILD_WITH_GUI
        std::cerr << "ERROR: no flowchart_file provided!\n";
        print_help(program_name);
        return EXIT_FAILURE;
      #endif
    } else {

      flowchart_path = cmdl[1];
      if (!fs::exists(flowchart_path)) {
        std::cerr << "ERROR: no such flowchart_file: " << flowchart_path << "\n";
        print_help(program_name);
        return EXIT_FAILURE;
      }

      // set current work directory to folder containing flowchart file
      auto abs_path = fs::absolute(fs::path(flowchart_path));
      flowchart_folder = abs_path.parent_path();
      flowchart_path = abs_path.string();
      // fs::current_path(flowchart_folder);
      flowchart.load_json(flowchart_path);
      // fs::current_path(launch_path);

      // handle globals overrides provided by user
      list_globals = cmdl[{"-g", "--list-globals"}];
      if (list_globals) std::cout << "Available globals:\n";
      for (auto&[key,val] : flowchart.global_flowchart_params) {
        auto [it, inserted] = globals_from_cli.emplace(std::make_pair(key, vec1s{}));
        // config_subcommand->add_option("--"+key, (it->second), "");
        if (list_globals) {
          std::cout << " > " << key;
          if (!val->get_help().empty()) std::cout << " [" << val->get_help() << "]";
          std::cout << "\n   default value: " << val->as_json() << "\n";
        }
      }
      if (list_globals) return EXIT_SUCCESS;  

      // process global values from cli/config file
      std::string config_path;
      // config is seen as flag because there is no value provided
      if (cmdl[{"-c", "--config"}]) {
        std::cerr << "ERROR: no config file provided\n";
        print_help(program_name);
        return EXIT_FAILURE;
      }
      // config is seen as parameter and there is a value provided
      if (cmdl({"-c", "--config"}) >> config_path) {
        if (!fs::exists(config_path)) {
          std::cerr << "ERROR: no such config file: " << config_path << "\n";
          print_help(program_name);
          return EXIT_FAILURE;
        }
        std::cout << "Reading configuration from file " << config_path << std::endl;
        toml::table config;
        try {
          config = toml::parse_file( config_path );
        } catch (const std::exception& e) {
          std::cerr << "ERROR: unable to parse config file " << config_path << "\n";
          std::cerr << "  " << e.what() << std::endl;
          return EXIT_FAILURE;
        }

        for (auto&& [key, value] : config)
        {
          if (flowchart.global_flowchart_params.find(key.data()) == flowchart.global_flowchart_params.end()) {
            std::cerr << "WARNING: no such global parameter (in config): " << key.str() << " (use -g to list available globals)\n";
            continue;
            // print_help(program_name);
            // return EXIT_FAILURE;
          }
          auto& g = flowchart.global_flowchart_params[key.data()];
          try{
            if (g->is_type(typeid(std::string))) {
              if (!value.is_string()) throw gfFlowchartError("Unable to set string from provided value");
              auto& s = value.ref<std::string>();
              auto* gptr = static_cast<ParameterByValue<std::string>*>(g.get());
              gptr->set(s);
            } else if (g->is_type(typeid(float))) {
              if (!value.is_floating_point()) throw gfFlowchartError("Unable to set float from provided value");
              auto& f = value.ref<double>();
              auto* gptr = static_cast<ParameterByValue<float>*>(g.get());
              gptr->set(float(f));
            } else if(g->is_type(typeid(int))) {
              if (!value.is_integer()) throw gfFlowchartError("Unable to set integer from provided value");
              auto& i = value.ref<int64_t>();
              auto* gptr = static_cast<ParameterByValue<int>*>(g.get());
              gptr->set(int(i));
            } else if(g->is_type(typeid(bool))) {
              if (!value.is_boolean()) throw gfFlowchartError("Unable set boolean global from provided value");
              auto& b = value.ref<bool>();
              auto* gptr = static_cast<ParameterByValue<int>*>(g.get());
              gptr->set(b);
            }
            std::cout << "set global " << key.data() << " = " << config[key.data()] << " (from config file)\n";
          } catch (const std::exception& e) {
            std::cerr << "ERROR in parsing global parameter '" << key << "':\n";
            std::cerr << e.what() << std::endl;
          }
        }
      }
      for (auto& [key, value] : cmdl.params()) {
        if (key == "c" || key == "config") continue;
        
        if (flowchart.global_flowchart_params.find(key) == flowchart.global_flowchart_params.end()) {
          std::cerr << "WARNING: no such global parameter: " << key << " (use -g to view available globals)\n";
          continue;
          // print_help(program_name);
          // return EXIT_FAILURE;
        }
        auto& g = flowchart.global_flowchart_params[key];
        try{
          if (g->is_type(typeid(std::string))) {
            std::string s;
            if (!(cmdl(key) >> s)) throw gfFlowchartError("Unable to set string from provided value (" + value + ")");
            auto* gptr = static_cast<ParameterByValue<std::string>*>(g.get());
            gptr->set(s);
          } else if (g->is_type(typeid(float))) {
            float f;
            if (!(cmdl(key) >> f)) throw gfFlowchartError("Unable to set float from provided value (" + value + ")");
            auto* gptr = static_cast<ParameterByValue<float>*>(g.get());
            gptr->set(f);
          } else if(g->is_type(typeid(int))) {
            int i;
            if (!(cmdl(key) >> i)) throw gfFlowchartError("Unable to set integer from provided value (" + value + ")");
            auto* gptr = static_cast<ParameterByValue<int>*>(g.get());
            gptr->set(i);
          } else if(g->is_type(typeid(bool))) {
            auto* gptr = static_cast<ParameterByValue<bool>*>(g.get());
            if(value == "true")
              gptr->set(true);
            else if(value == "false")
              gptr->set(false);
            else throw gfFlowchartError("Unable set boolean global from provided value (" + value + "). Please use 'true' or 'false'.");
          }
          std::cout << "set global " << key << " = " << value << " (from command line)\n";
        } catch (const std::exception& e) {
          std::cerr << "Error in parsing global parameter '" << key << "':\n";
          std::cerr << e.what() << std::endl;
        }
      }
    }

    if( ! list_globals ) {
      if( ! verbose ) {
        std::clog.setstate(std::ios_base::failbit);
        std::cout.setstate(std::ios_base::failbit);
        // std::cerr.setstate(std::ios_base::failbit);
      }

      // launch gui or just run the flowchart in cli mode
      if(cmdl[{"-w", "--workdir"}]) fs::current_path(flowchart_folder);
      #ifdef GF_BUILD_WITH_GUI
        try {
        launch_gui(flowchart, flowchart_path);
        }
        catch (const gfException& e) {
          // std::cerr.clear();
          std::cerr << "ERROR: " << e.what() << "\n";
          return EXIT_FAILURE;
        }
      #else
        try {
          flowchart.run_all();
        }
        catch (const gfException& e) {
          // std::cerr.clear();
          std::cerr << "ERROR: " << e.what() << "\n";
          return EXIT_FAILURE;
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
