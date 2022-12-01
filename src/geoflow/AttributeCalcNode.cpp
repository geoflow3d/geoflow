#include "core_nodes.hpp"
#include "exprtk.hpp"

namespace geoflow::nodes::core {

  void AttributeCalcNode::process() {
    typedef float T; // numeric type (float, T, mpfr etc...)
    typedef exprtk::symbol_table<T> symbol_table_t;
    typedef exprtk::expression<T>   expression_t;
    typedef exprtk::parser<T>       parser_t;

    //  = "z := x - (3 * y)";

    // T x = T(123.456);
    // T y = T(98.98);
    // T z = T(0.0);

    symbol_table_t symbol_table;

    std::unordered_map<std::string, T> gf_symbols;
    for (auto& [key,param_ptr] : manager.global_flowchart_params) {
      if(param_ptr->is_type(typeid(int))){
        auto* val = static_cast<ParameterByValue<int>*>(param_ptr.get());
        T& dval = (gf_symbols.insert({key, T(val->get())}).first)->second; // get iterator, then value
        symbol_table.add_variable("g."+key, dval); // or use add_constants ?
      } else if(param_ptr->is_type(typeid(float))){
        auto* val = static_cast<ParameterByValue<float>*>(param_ptr.get());
        T& dval = (gf_symbols.insert({key, T(val->get())}).first)->second; // get iterator, then value
        symbol_table.add_variable("g."+key, dval); // or use add_constants ?
      } else if(param_ptr->is_type(typeid(bool))){
        auto* val = static_cast<ParameterByValue<bool>*>(param_ptr.get());
        T& dval = (gf_symbols.insert({key, T(val->get())}).first)->second; // get iterator, then value
        symbol_table.add_variable("g."+key, dval); // or use add_constants ?
      } else if(param_ptr->is_type(typeid(std::string))){
        auto* val = static_cast<ParameterByValue<std::string>*>(param_ptr.get());
        symbol_table.add_stringvar("g."+key, val->get()); // or use add_constants ?
      }
      
      // symbol_table.add_stringvar(key, val);
    }
    // idem for input attributes
    for (auto& iterm : poly_input("attributes").sub_terminals()) {
      if(iterm->accepts_type(typeid(std::string))) continue;

      std::string name = "a." + iterm->get_full_name();
      T& dval = (gf_symbols.insert({name, 0}).first)->second;
      symbol_table.add_variable(name,dval);
    }
    std::cout << "Symbol table" << std::endl;
    for(auto& [name, val] : gf_symbols) {
      std::cout << " " << name << " = " << val << std::endl;
    } 

    parser_t parser;
    std::unordered_map<std::string, expression_t> expressions;
    for(auto& [name, expr_str] : attribute_expressions) {
      expression_t expression;
      expression.register_symbol_table(symbol_table);
      if (!parser.compile(expr_str, expression))
      {
          throw gfException("Exprtk could not compile expression: " + expr_str);
      }
      expressions.insert({name, expression});
      poly_output("attributes").add_vector(name, typeid(float));
    }
    
    size_t isize = poly_input("attributes").size();
    std::string expression_string;
    std::cout << "Expression results:" << std::endl;
    for(size_t i=0; i<isize; ++i) {
      // assign input attributes
      for (auto& iterm : poly_input("attributes").sub_terminals()) {
        if (iterm->accepts_type(typeid(std::string))) continue;
        std::string name = "a." + iterm->get_full_name();
        if (iterm->accepts_type(typeid(float))) {
          gf_symbols[name] = iterm->get<float>(i);
        } else if (iterm->accepts_type(typeid(int))) {
          gf_symbols[name] = T(iterm->get<int>(i));
        } else if (iterm->accepts_type(typeid(bool))) {
          gf_symbols[name] = T(iterm->get<bool>(i));
        }
      }  
      
      for(auto& [name, expr] : expressions) {
        // assign expression vars/consts
        // evaluate expression
        // push result to output
        T result = expr.value();
        std::cout << result << std::endl;
        poly_output("attributes").sub_terminal(name).push_back(float(result));
      }
    }
  };

}