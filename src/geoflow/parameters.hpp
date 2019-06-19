#pragma once

#include <string>
#include <map>
#include <variant>


namespace geoflow {
  template <typename T> class ParameterBase {
    T& value;
    public:
    ParameterBase(T& val) : value(val) {};
    T& get() {
      return value;
    }
    void set(T val) {
      value = val;
    }
  };
  template<typename T> class ParameterRange : public ParameterBase<T> {
    T min, max;
    ParameterRange(T& val, T min, T max) : ParameterBase<T>(val), min(min), max(max) {};
  };
  class ParamPath : public ParameterBase<std::string> {
    using ParameterBase::ParameterBase;
  };
  typedef ParameterBase<float> ParamFloat;
  typedef ParameterRange<float> ParamFloatRange;
  typedef ParameterBase<int> ParamInt;
  typedef ParameterRange<int> ParamIntRange;
  typedef ParameterBase<bool> ParamBool;
  typedef ParameterBase<std::string> ParamString;

  typedef std::variant<ParamBool, ParamInt, ParamFloat, ParamPath> ParameterVariant;
  typedef std::map<std::string, ParameterVariant> ParameterSet;
  // class ParameterSet : public ParameterMap {

  // };

  struct Path {
    std::string value;
    Path(std::string val) : value(val){};
  };
}