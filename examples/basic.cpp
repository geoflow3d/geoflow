#include "geoflow.hpp"
#include "basic_nodes.hpp"
#include <iostream>

using namespace geoflow;

int main(void) {
  NodeManager N = NodeManager();
  N.register_node<AdderNode>("Adder");
  N.register_node<NumberNode>("Number");
  auto adder = N.create("Adder");
  auto number = N.create("Number");
  auto adder2 = N.create("Adder");
  auto number2 = N.create("Number");
  N.connect(number, adder, "result", "in1");
  N.connect(number, adder, "result", "in2");
  N.connect(adder, adder2, "result", "in1");
  N.connect(adder, adder2, "result", "in2");
  bool success = N.run(*number);
  if (success){
    try{
      std::cout << "Result: " << std::any_cast<float>(adder2->outputTerminals["result"]->cdata) << "\n";
    } catch(const std::bad_any_cast& e) {
      std::cout << "Oops... " << e.what() << '\n';
    }
  } else {
    std::cout << "No result, missing inputs\n";
  }
}