#include "geoflow.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "imgui.h"

using namespace geoflow;

class AdderNode:public Node {
  public:
  AdderNode(NodeManager& manager):Node(manager) {
    add_input("in1", TT_float);
    add_input("in2", TT_float);
    add_output("result", TT_float);
  }

  void gui(){
    auto a = outputTerminals["result"]->cdata;
    if (a.has_value()) {
      ImGui::Text("Result %f", std::any_cast<float>(a));
    }
  }

  void process(){
    std::cout << "begin AddderNode::process()" << "\n";
    auto in1 = get_value("in1");
    auto in2 = get_value("in2");
    std::this_thread::sleep_for(std::chrono::microseconds(200));
    set_value("result", std::any_cast<float>(in1)+std::any_cast<float>(in2));
    std::cout << "end AddderNode::process()" << "\n";
  }
};

class NumberNode:public Node {
  public:
  int thenumber=0;

  NumberNode(NodeManager& manager):Node(manager) {
    add_output("result", TT_float);
  }

  void gui(){
    ImGui::InputInt("The number", &thenumber);
  }

  void process(){
    std::cout << "begin NumberNode::process()" << "\n";
    set_value("result", float(thenumber));
    std::cout << "end NumberNode::process()" << "\n";
  }
};

class NumberNodeI:public Node {
  public:
  NumberNodeI(NodeManager& manager):Node(manager) {
    add_output("result", TT_int);
  }

  void process(){
    std::cout << "begin NumberNode::process()" << "\n";
    set_value("result", int(1));
    std::cout << "end NumberNode::process()" << "\n";
  }
};