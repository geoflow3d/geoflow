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

#include "api.hpp"
#include <geoflow/plugin_manager.hpp>
#include <geoflow/core_nodes.hpp>

namespace geoflow {

  GeoflowRunner::GeoflowRunner(std::string flowchart_json, bool verbose) 
  : verbose(verbose), flowchart(node_registers), plugin_manager(createPluginManager()) 
  {
    // load node registers from libraries
    auto R_core = NodeRegister::create("Core");
    R_core->register_node<nodes::core::NestNode>("NestedFlowchart");
    R_core->register_node<nodes::core::IntNode>("Int");
    R_core->register_node<nodes::core::FloatNode>("Float");
    R_core->register_node<nodes::core::BoolNode>("Bool");
    R_core->register_node<nodes::core::TextNode>("Text");
    R_core->register_node<nodes::core::TextWriterNode>("TextWriter");
    node_registers.emplace(R_core);


    std::string plugin_folder = GF_PLUGIN_FOLDER;
    if(const char* env_p = std::getenv("GF_PLUGIN_FOLDER")) {
      plugin_folder = env_p;
    }
    
    if(fs::exists(plugin_folder)) {
      plugin_manager->load(plugin_folder, node_registers, verbose);
    } else {
      throw gfException("Plugin folder does not exist: " + plugin_folder);
    };


    // check for flowchart
    // flowchart = NodeManager(node_registers);
    if (!fs::exists(flowchart_json)) {
      throw gfException("no such flowchart_file: " + flowchart_json);
    }
    flowchart.load_json(flowchart_json);
  };
  GeoflowRunner::~GeoflowRunner() {
    // NOTICE that we first must destruct any related node_registers before we can unload the plugin_manager!
    plugin_manager->unload(verbose);
  }

  /*==========================================================================================
  Run the flowchart
  ==========================================================================================*/
  void GeoflowRunner::run() {
    
    // currently there is a lot of loggin and debug messages printed. We can turn this off using this:
    if( verbose ) {
      std::clog.setstate(std::ios_base::failbit);
      std::cout.setstate(std::ios_base::failbit);
      // std::cerr.setstate(std::ios_base::failbit);
    }

    flowchart.run_all();

    if( verbose ) {
      std::clog.clear();
      std::cout.clear();
      // std::cerr.clear();
    }
  };

}
