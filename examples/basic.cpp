#include "basic_nodes.hpp"
#include <iostream>

using namespace geoflow;

int main(void) {
  NodeRegister R = nodes::arithmetic::create_register();
  NodeManager N;
  auto adder = N.create_node(R, "Adder");
  auto number = N.create_node(R, "Number");
  auto adder2 = N.create_node(R, "Adder");
  auto number2 = N.create_node(R, "Number");
  
  connect(number->output("result"), adder->input("in1"));
  connect(number, adder, "result", "in2");
  connect(adder, adder2, "result", "in1");
  connect(adder, adder2, "result", "in2");

  number->load_params({
    {"number_value", (int) 5}
  });

  bool success = N.run(number);
  if (success){
    std::cout << "Result: " << adder2->output("result").get<float>() << "\n";
  } else {
    std::cout << "No result\n";
  }
}