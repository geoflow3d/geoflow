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

#include <geoflow/gui/flowchart.hpp>

#include "basic_nodes.hpp"

// #include <nlohmann/json.hpp>

// using json = nlohmann::json;

// #include <boost/program_options.hpp>


int main(int ac, const char * av[])
{
    NodeRegisterMap RM;
    NodeRegister R("Arithmetic");
    R.register_node<nodes::arithmetic::AdderNode>("Adder");
    R.register_node<nodes::arithmetic::NumberNode>("Number");
    NodeRegister R_gui("BasicShapes");
    R_gui.register_node<nodes::gui::CubeNode>("Cube");
    R_gui.register_node<nodes::gui::TriangleNode>("Triangle");

    RM.emplace(R.get_name(), R);
    RM.emplace(R_gui.get_name(), R_gui);
    NodeManager N;
    // N.create_node(R_gui, "Cube", {0,-200});
    // auto adder = N.create_node(R, "Adder", {300,0});
    // auto number = N.create_node(R, "Number", {0, 000});

    // number->set_params({
    //     {"number_value", (int) 5}
    // });
    N.load_json("/Users/ravi/git/geoflow/debug/out.json", RM);

    // connect(number->output("result"), adder->input("in1"));
    // connect(number->output("result"), adder->input("in2"));

    launch_flowchart(N, {R,R_gui});

    // std::ifstream i("../examples/basic.gf.json");
    // json j;
    // i >> j;

    // ImGui::NodeStore ns;
    // auto& nodes = j["nodes"];
    // for (json::iterator it = nodes.begin(); it != nodes.end(); ++it) {
    //     auto node = it.value();
    //     ns.push_back(std::make_tuple(node["type"], it.key(), ImVec2(node["canvas-pos"][0],node["canvas-pos"][1])));
    // }
    // nodes_.PreloadNodes(ns);
    
    // ImGui::LinkStore ls;
    // for (auto& link : j["links"]) {
    //     std::string source=link["source"];
    //     std::string target=link["target"];
    //     auto ns = source.find("::");
    //     auto nt = target.find("::");
    //     ls.push_back(std::make_tuple(source.substr(0,ns), target.substr(0,nt), source.substr(ns+2), target.substr(nt+2)));
    // }
    // nodes_.PreloadLinks(ls);

}