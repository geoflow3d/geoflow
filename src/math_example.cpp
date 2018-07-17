#include "geoflow.hpp"
#include <iostream>

using namespace geoflow;

class AddderNode:public Node {
  public:
  AddderNode(){
    add_input("in1");
    add_input("in2");
    add_output("result");
  }

  void process(){
    std::cout << "begin AddderNode::process()" << "\n";
    auto in1 = get_value("in1");
    auto in2 = get_value("in2");
    // auto in1 = std::any_cast<float>(get_value("in1"));
    // auto in2 = std::any_cast<float>(get_value("in2"));
    set_value("result", std::any_cast<float>(in1)+std::any_cast<float>(in2));
    // std::cout << "\tin1.type: " << in1.type().name() << "\n";
    // std::cout << "\tin2.type: " << in2.type().name() << "\n";
    std::cout << "end AddderNode::process()" << "\n";
  }
};

class NumberNode:public Node {
  public:
  float number_value=float();
  NumberNode(float val){
    number_value = val;
    add_output("result");
  }
  
  

  void process(){
    std::cout << "begin NumberNode::process()" << "\n";
    set_value("result", number_value);
    std::cout << "end NumberNode::process()" << "\n";
  }
};

int main(void) {
  NodeManager N = NodeManager();
  auto adder = N.add(std::make_shared<AddderNode>());
  auto number = N.add(std::make_shared<NumberNode>(2));
  auto adder2 = N.add(std::make_shared<AddderNode>());
  auto number2 = N.add(std::make_shared<NumberNode>(42));
  N.connect(number, adder, "result", "in1");
  N.connect(number, adder, "result", "in2");
  N.connect(adder, adder2, "result", "in1");
  N.connect(adder, adder2, "result", "in2");
  number.lock()->check_inputs();
  std::cout << "Result: " << std::any_cast<float>(adder2.lock()->outputTerminals["result"]->cdata) << "\n";
}