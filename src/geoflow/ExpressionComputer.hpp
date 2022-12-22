#include "geoflow.hpp"

namespace geoflow {

  class ExpressionComputerInterface {

  public:
  virtual void add_expression(const std::string& name, const std::string& expr_string) = 0;
  virtual void add_symbol(const std::string& name, const std::string& prefix, float value=0) = 0;
  virtual void add_symbol(const std::string& name, const std::string& prefix, std::string value="") = 0;
  virtual void add_symbols(NodeManager& manager) = 0;
  virtual void set_symbol(const std::string& name, const float& value) = 0;
  virtual void set_symbol(const std::string& name, const std::string& value) = 0;
  // T& get_symbol(name) = 0;
  virtual float eval(const std::string& name) = 0;

  };

  std::unique_ptr<ExpressionComputerInterface> createExpressionComputer();
}