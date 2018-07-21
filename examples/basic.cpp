#include "geoflow.hpp"
#include "basic_nodes.hpp"
#include <iostream>

using namespace geoflow;

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