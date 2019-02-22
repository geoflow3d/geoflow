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

#include <geoflow/core/geoflow.hpp>
#include <iostream>
#include <thread>
#include <chrono>

namespace geoflow::nodes::arithmetic {
  class AdderNode:public Node {
    public:
    using Node::Node;

    void init() {
      add_input("in1", TT_float);
      add_input("in2", TT_float);
      add_output("result", TT_float);
    }

    void gui(){
      if (output("result").has_data()) {
        ImGui::Text("Result %f", output("result").get<float>());
      }
    }

    void process(){
      std::cout << "begin AddderNode::process()" << "\n";
      auto in1 = input("in1").get<float>();
      auto in2 = input("in2").get<float>();
      std::this_thread::sleep_for(std::chrono::microseconds(200));
      output("result").set(float(in1+in2));
      std::cout << "end AddderNode::process()" << "\n";
    }
  };

  class NumberNode:public Node {
    public:
    using Node::Node;

    void init() {
      add_output("result", TT_float);
      
      add_param("number_value", (int) 42);
    }

    void gui(){
      ImGui::InputInt("Number value", &param<int>("number_value"));
    }

    void process(){
      std::cout << "begin NumberNode::process()" << "\n";
      output("result").set(float(param<int>("number_value")));
      std::cout << "end NumberNode::process()" << "\n";
    }
  };

  class NumberNodeI:public Node {
    public:
    using Node::Node;
    void init() {
      add_output("result", TT_int);
    }

    void process(){
      std::cout << "begin NumberNode::process()" << "\n";
      output("result").set(1);
      std::cout << "end NumberNode::process()" << "\n";
    }
  };

  NodeRegister create_register() {
    NodeRegister R("Arithmetic");
    R.register_node<AdderNode>("Adder");
    R.register_node<NumberNode>("Number");
    // R.register_node<NumberNodeI>("NumberI");
    return R;
  }
}