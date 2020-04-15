// This file is part of Geoflow
// Copyright (C) 2018-2019  Ravi Peters, 3D geoinformation TU Delft

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <string>
#include <map>
#include <variant>
#include <vector>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace geoflow {
  class Parameter {
    protected:
    std::string label_, help_;
    std::type_index type_;
    std::weak_ptr<Parameter> master_parameter_;
    public:
    Parameter(std::string label, std::string help, std::type_index ttype=typeid(void)) : label_(label), help_(help), type_(ttype) {};
    std::string get_label() {
      return label_;
    }
    virtual json as_json() const = 0;
    virtual void from_json(const json& json_object) = 0;
    // virtual void to_string(std::string& str) const = 0;
    // virtual void from_string(const std::string& str) = 0;
    bool is_type(std::type_index type) {
      return type_ == type;
    }
    bool is_type_compatible(const Parameter& other_parameter) {
      return type_ == other_parameter.type_;
    }
    void set_master(std::weak_ptr<Parameter> master_parameter) {
      if (!is_type_compatible(*master_parameter.lock()))
        std::cout << "Attempting to set incompatible master parameter\n";
      else
        master_parameter_ = master_parameter;
    };
    void copy_value_from_master() {
      if(!master_parameter_.expired()) {
        from_json(master_parameter_.lock()->as_json());
      }
    };
    bool has_master() const {
      return !master_parameter_.expired();
    }
    void clear_master() {
      master_parameter_.reset();
    }
    std::weak_ptr<Parameter> get_master() const {
      return master_parameter_;
    }
    // void set(T val) {
    //   value = val;
    // }
    // bool visible() {
    //   return is_visible;
    // }
    // void set_visible(bool vis) {
    //   is_visible=vis;
    // }
  };
  template<typename T> class ParameterByReference : public Parameter {
    protected:
    T& value_;
    
    public:
    ParameterByReference(T& value, std::string label, std::string help) 
    : value_(value), Parameter(label, help, typeid(T)) {};

    virtual json as_json() const override {
      return json(value_);
    };
    virtual void from_json(const json& json_object) override {
      value_ = json_object.get<T>();
    };
    T& get() {
      return value_;
    }
    void set(T val) {
      value_ = val;
    }
  };
  template<typename T> class ParameterByValue : public Parameter {
    protected:
    T value_;

    public:
    ParameterByValue(T value, std::string label, std::string help) 
    : value_(value), Parameter(label, help, typeid(T)) {};

    virtual json as_json() const override {
      return json(value_);
    };
    virtual void from_json(const json& json_object) override {
      value_ = json_object.get<T>();
    };
    T& get() {
      return value_;
    }
    void set(T val) {
      value_ = val;
    }
  };

  template<typename T> class ParameterBounded : public ParameterByReference<T> {
    T min_, max_;
    public:
    ParameterBounded(T& val, T min, T max, std::string label, std::string help="") : ParameterByReference<T>(val, label, help), min_(min), max_(max) {};
    T min() { return min_;}
    T max() { return max_;}
    void set_bounds(T min, T max) {
      min_ = min;
      max_ = max;
    }
  };
  class ParamPath : public ParameterByReference<std::string> {
    using ParameterByReference::ParameterByReference;
  };

  typedef std::unordered_map<std::string,std::string> StrMap;
  class ParamStrMap : public ParameterByReference<StrMap> {
    public:
    vec1s& key_options_;
    ParamStrMap(StrMap& val, vec1s& key_options, std::string label, std::string help="") : key_options_(key_options), ParameterByReference(val, label, help) {};
  };

  class ParamSelector : public ParameterByReference<size_t> {
    std::vector<std::string> options;

    public:
    ParamSelector(std::vector<std::string> options, size_t& index, std::string label, std::string help="") : ParameterByReference(index, label, help) {};

    std::vector<std::string> get_options() {
      return options;
    }
    std::string get_selected_option() {
      return options.at(value_);
    }
  };
  typedef ParameterByReference<float> ParamFloat;
  typedef ParameterByReference<double> ParamDouble;
  typedef ParameterByReference<std::pair<float,float>> ParamFloatRange;
  typedef ParameterByReference<std::pair<int,int>> ParamIntRange;
  typedef ParameterByReference<std::pair<double,double>> ParamDoubleRange;
  typedef ParameterBounded<float> ParamBoundedFloat;
  typedef ParameterBounded<double> ParamBoundedDouble;
  typedef ParameterByReference<int> ParamInt;
  typedef ParameterBounded<int> ParamBoundedInt;
  typedef ParameterByReference<bool> ParamBool;
  typedef ParameterByReference<std::string> ParamString;

  typedef std::map<std::string, std::shared_ptr<Parameter>> ParameterMap;
}