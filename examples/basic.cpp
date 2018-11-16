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
  // connect(*adder2, *adder, "result", "in1");
  // connect(*adder, *adder2, "result", "in1");
  connect(*number, *adder, "result", "in1");
  connect(*number, *adder, "result", "in2");
  connect(*adder, *adder2, "result", "in1");
  connect(*adder, *adder2, "result", "in2");
  bool success = N.run(*number);
  if (success){
    try{
      std::cout << "Result: " << adder2->outputs("result").get<float>() << "\n";
    } catch(const std::bad_any_cast& e) {
      std::cout << "Oops... " << e.what() << '\n';
    }
  } else {
    std::cout << "No result, missing inputs\n";
  }
}