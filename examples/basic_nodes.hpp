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
    if (outputs("result").has_data()) {
      ImGui::Text("Result %f", outputs("result").get<float>());
    }
  }

  void process(){
    std::cout << "begin AddderNode::process()" << "\n";
    auto in1 = inputs("in1").get<float>();
    auto in2 = inputs("in2").get<float>();
    std::this_thread::sleep_for(std::chrono::microseconds(200));
    outputs("result").set(float(in1+in2));
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
    outputs("result").set(float(thenumber));
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
    outputs("result").set(1);
    std::cout << "end NumberNode::process()" << "\n";
  }
};