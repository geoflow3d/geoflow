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
      if(iterm->accepts_type(typeid(std::string))) {
        computer->add_symbol(iterm->get_full_name(), "a.", "");
      } else {
        computer->add_symbol(iterm->get_full_name(), "a.", 0);
      }
    }

    if(as_string_) {
      computer->add_str_result_symbol();
    }
    
    for(auto& [name, expr_str] : attribute_expressions) {
      computer->add_expression(name, expr_str);
      if(as_string_) {
        poly_output("attributes").add_vector(name, typeid(std::string));
      } else {
        poly_output("attributes").add_vector(name, typeid(float));
      }
    }
    
    size_t isize = poly_input("attributes").size();
    // std::cout << "Expression results:" << std::endl;
    for(size_t i=0; i<isize; ++i) {
      // assign input attributes
      for (auto& iterm : poly_input("attributes").sub_terminals()) {
        std::string name = "a." + iterm->get_full_name();
        if (iterm->accepts_type(typeid(float))) {
          computer->set_symbol(name, iterm->get<float>(i));
        } else if (iterm->accepts_type(typeid(int))) {
          computer->set_symbol(name, float(iterm->get<int>(i)));
        } else if (iterm->accepts_type(typeid(bool))) {
          computer->set_symbol(name, float(iterm->get<bool>(i)));
        } else if (iterm->accepts_type(typeid(std::string))) {
          computer->set_symbol(name, iterm->get<std::string>(i));
        }
      }  
      
      for(auto& [name, expr] : attribute_expressions) {
        // assign expression vars/consts
        // evaluate expression
        // push result to output
        // std::cout << result << std::endl;
        if(as_string_) {
          poly_output("attributes").sub_terminal(name).push_back(computer->eval_str(name));
        } else {
          poly_output("attributes").sub_terminal(name).push_back(float(computer->eval(name)));
        }
      }
    }
  };

}