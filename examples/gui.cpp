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
#include <geoflow/gui/osdialog.hpp>

#include "Arithmetic/nodes.hpp"

// #include <nlohmann/json.hpp>

// using json = nlohmann::json;

// #include <boost/program_options.hpp>

  class FileOpenNode:public Node {
    public:
    using Node::Node;
    void init() {
      add_output("result", typeid(int));
      add_param("path", (std::string) "");
    }

    void gui() {
        ImGui::FilePicker(OSDIALOG_OPEN, param<std::string>("path"));
    }

    void process(){
      output("result").set(1);
    }
  };


int main(int ac, const char * av[])
{
    auto R = NodeRegister::create("Arithmetic");
    R->register_node<nodes::arithmetic::AdderNode>("Adder");
    R->register_node<nodes::arithmetic::NumberNode>("Number");
    auto R_gui = NodeRegister::create("BasicShapes");
    R_gui->register_node<nodes::gui::CubeNode>("Cube");
    R_gui->register_node<nodes::gui::TriangleNode>("Triangle");
    R_gui->register_node<FileOpenNode>("FileOpen");

    NodeRegisterMap RM;
    RM.emplace(R);
    RM.emplace(R_gui);
    NodeManager N;
    N.create_node(R_gui, "Cube", {0,-200});
    auto adder = N.create_node(R, "Adder", {300,0});
    auto number = N.create_node(R, "Number", {0, 000});

    number->set_params({
        {"number_value", (int) 5}
    });

    connect(number->output("result"), adder->input("in1"));
    connect(number->output("result"), adder->input("in2"));

    launch_flowchart(N, {R,R_gui});

}