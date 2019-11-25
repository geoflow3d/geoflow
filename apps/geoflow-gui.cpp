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

#include <DLLoader.h>
#include "CLI11.hpp"

#include <geoflow/geoflow.hpp>
#include <geoflow/plugin_manager.hpp>

#define GF_INCLUDE_WITH_GUI
#include <geoflow/core_nodes.hpp>
#undef GF_INCLUDE_WITH_GUI

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <geoflow/gui/gfImNodes.hpp>

using namespace geoflow;

int main(int argc, const char * argv[]) {

  std::string flowchart_path = "/Users/ravi/git/geoflow/apps/add.json";
  std::string plugin_folder = GF_PLUGIN_FOLDER;


  CLI::App cli{"Geoflow"};

  CLI::Option* opt_flowchart_path = cli.add_option("-f,--flowchart", flowchart_path, "Flowchart file");
  CLI::Option* opt_plugin_folder = cli.add_option("-p,--plugin-folder", plugin_folder, "Plugin folder");

  try {
    cli.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return cli.exit(e);
  }

  // load node registers from libraries
  {
    NodeRegisterMap node_registers;
    auto R_core = NodeRegister::create("Core");
    R_core->register_node<nodes::core::NestNode>("NestedFlowchart");
    node_registers.emplace(R_core);

    auto R_gui = NodeRegister::create("Visualisation");
    R_gui->register_node<nodes::gui::ColorMapperNode>("ColorMapper");
    R_gui->register_node<nodes::gui::GradientMapperNode>("GradientMapper");
    R_gui->register_node<nodes::gui::PainterNode>("Painter");
    R_gui->register_node<nodes::gui::VectorPainterNode>("VectorPainter");
    R_gui->register_node<nodes::gui::CubeNode>("Cube");
    R_gui->register_node<nodes::gui::TriangleNode>("Triangle");
    node_registers.emplace(R_gui);
    
    ImGui::CreateContext();

    PluginManager plugin_manager;
    plugin_manager.load(plugin_folder, node_registers);

    // load flowchart from file
    NodeManager node_manager(node_registers);
    if(*opt_flowchart_path)
      node_manager.load_json(flowchart_path);

    // launch gui
    launch_flowchart(node_manager);
  }

  return 0;
}