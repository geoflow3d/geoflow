#include "core_nodes.hpp"
#include "ExpressionComputer.hpp"

namespace geoflow::nodes::core {

  void FloatExprNode::process(){
    auto computer = createExpressionComputer();

    computer->add_symbols(manager);
    computer->add_expression("result", expr_string_);

    output("value").set(computer->eval("result"));
  };

  void AttributeCalcNode::process() {

    auto computer = createExpressionComputer();

    computer->add_symbols(manager);

    // add input attributes
    for (auto& iterm : poly_input("attributes").sub_terminals()) {
      if(iterm->accepts_type(typeid(std::string))) continue;
      computer->add_symbol(iterm->get_full_name(), "a.");
    }
    
    for(auto& [name, expr_str] : attribute_expressions) {
      computer->add_expression(name, expr_str);
      poly_output("attributes").add_vector(name, typeid(float));
    }
    
    size_t isize = poly_input("attributes").size();
    std::cout << "Expression results:" << std::endl;
    for(size_t i=0; i<isize; ++i) {
      // assign input attributes
      for (auto& iterm : poly_input("attributes").sub_terminals()) {
        if (iterm->accepts_type(typeid(std::string))) continue;
        std::string name = "a." + iterm->get_full_name();
        if (iterm->accepts_type(typeid(float))) {
          computer->set_symbol(name, iterm->get<float>(i));
        } else if (iterm->accepts_type(typeid(int))) {
          computer->set_symbol(name, float(iterm->get<int>(i)));
        } else if (iterm->accepts_type(typeid(bool))) {
          computer->set_symbol(name, float(iterm->get<bool>(i)));
        }
      }  
      
      for(auto& [name, expr] : attribute_expressions) {
        // assign expression vars/consts
        // evaluate expression
        // push result to output
        float result = computer->eval(name);
        std::cout << result << std::endl;
        poly_output("attributes").sub_terminal(name).push_back(float(result));
      }
    }
  };

}