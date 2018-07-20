#include "geoflow.hpp"
#include <iostream>

using namespace geoflow;

class AdderNode:public Node {
  public:
  AdderNode(NodeManager& manager):Node(manager, "Adder") {
    add_input("in1", TT_float);
    add_input("in2", TT_float);
    add_output("result", TT_float);
  }

  void process(){
    std::cout << "begin AddderNode::process()" << "\n";
    auto in1 = get_value("in1");
    auto in2 = get_value("in2");
    set_value("result", std::any_cast<float>(in1)+std::any_cast<float>(in2));
    std::cout << "end AddderNode::process()" << "\n";
  }
};

class NumberNode:public Node {
  public:
  NumberNode(NodeManager& manager):Node(manager, "Number") {
    add_output("result", TT_float);
  }

  void process(){
    std::cout << "begin NumberNode::process()" << "\n";
    set_value("result", float(1));
    std::cout << "end NumberNode::process()" << "\n";
  }
};

int main(void) {
  NodeManager N = NodeManager();
  N.register_node<AdderNode>("Adder");
  N.register_node<NumberNode>("Number");
  auto adder = N.add("Adder");
  auto number = N.add("Number");
  auto adder2 = N.add("Adder");
  auto number2 = N.add("Number");
  N.connect(number, adder, "result", "in1");
  N.connect(number, adder, "result", "in2");
  N.connect(adder, adder2, "result", "in1");
  N.connect(adder, adder2, "result", "in2");
  N.run(*number.lock());
  try{
    std::cout << "Result: " << std::any_cast<float>(adder2.lock()->outputTerminals["result"]->cdata) << "\n";
  } catch(const std::bad_any_cast& e) {
    std::cout << "Oops... " << e.what() << '\n';
  }
}