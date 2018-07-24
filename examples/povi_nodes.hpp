#include "geoflow.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "imgui.h"
#include "gloo.h"

using namespace geoflow;

class PoviPainterNode:public Node {
  std::shared_ptr<Painter> painter;
  public:
  PoviPainterNode(NodeManager& manager):Node(manager, "Adder") {
    painter = std::make_shared<Painter>();
    add_input("data", TT_vec_float);
    add_output("painter", TT_povi_painter);
  }

  void on_connect(InputTerminal t){
    painter->set_data(...);
  }
  void on_clear(InputTerminal t){
    painter->set_data(...);
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