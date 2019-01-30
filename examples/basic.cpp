#include "geoflow.hpp"
#include "basic_nodes.hpp"
#include <iostream>

using namespace geoflow;

int main(void) {
  NodeRegister R = NodeRegister();
  R.register_node<AdderNode>("Adder");
  R.register_node<NumberNode>("Number");
  NodeManager N;
  auto adder = N.create_node(R, "Adder");
  auto number = N.create_node(R, "Number");
  auto adder2 = N.create_node(R, "Adder");
  auto number2 = N.create_node(R, "Number");
  // connect(*adder2, *adder, "result", "in1");
  // connect(*adder, *adder2, "result", "in1");
  connect(*number, *adder, "result", "in1");
  connect(*number, *adder, "result", "in2");
  connect(*adder, *adder2, "result", "in1");
  connect(*adder, *adder2, "result", "in2");
  bool success = N.run(*number);
  if (success){
    // try{
      std::cout << "Result: " << adder2->outputs("result").get<float>() << "\n";
    // } catch(const std::bad_any_cast& e) {
    //   std::cout << "Oops... " << e.what() << '\n';
    // }
  } else {
    std::cout << "No result, missing inputs\n";
  }
}