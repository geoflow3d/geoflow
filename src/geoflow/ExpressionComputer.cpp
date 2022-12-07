#include "exprtk.hpp"
#include <unordered_map>
#include "ExpressionComputer.hpp"

namespace geoflow {

  class ExpressionComputer : public ExpressionComputerInterface {
    typedef exprtk::symbol_table<float> symbol_table_t;
    typedef exprtk::expression<float>   expression_t;
    typedef exprtk::parser<float>       parser_t;

    parser_t parser;
    symbol_table_t symbol_table;

    std::unordered_map<std::string, float> symbols;
    std::unordered_map<std::string, expression_t> expressions;

    void add_expression(const std::string& name, const std::string& expr_str) override {
      expression_t expression;
      expression.register_symbol_table(symbol_table);
      if (!parser.compile(expr_str, expression))
      {
          throw gfException("Exprtk could not compile expression: " + expr_str);
      }
      expressions.insert({name, expression});

    };

    void add_symbol(const std::string& name, const std::string& prefix, float value) override {
      float& dval = (symbols.insert({prefix+name, value}).first)->second; // get iterator, then value
      symbol_table.add_variable(prefix+name, dval); // or use add_constants ?
    };
    
    void add_symbols(NodeManager& manager) override {
      for (auto& [key,param_ptr] : manager.global_flowchart_params) {
        if(param_ptr->is_type(typeid(int))){
          float val = static_cast<ParameterByValue<int>*>(param_ptr.get())->get();
          add_symbol(key, "g.", val);
        } else if(param_ptr->is_type(typeid(float))){
          float val = static_cast<ParameterByValue<float>*>(param_ptr.get())->get();
          add_symbol(key, "g.", val);
        } else if(param_ptr->is_type(typeid(bool))){
          float val = static_cast<ParameterByValue<bool>*>(param_ptr.get())->get();
          add_symbol(key, "g.", val);
        } 
        // else if(param_ptr->is_type(typeid(std::string))){
        //   auto* val = static_cast<ParameterByValue<std::string>*>(param_ptr.get());
        //   symbol_table.add_stringvar("g."+key, val->get()); // or use add_constants ?
        // }
        // symbol_table.add_stringvar(key, val);
      }
    };

    void set_symbol(const std::string& name, const float& value) override {
      symbols[name] = value;
    };
    // T& get_symbol(name);

    float eval(const std::string& name) override {
      return expressions[name].value();
    };
  
  };

  std::unique_ptr<ExpressionComputerInterface> createExpressionComputer() {
    return std::make_unique<ExpressionComputer>();
  };

}