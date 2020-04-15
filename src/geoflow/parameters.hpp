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
    Parameter(std::string label, std::string help, std::type_index ttype=typeid(void));
    std::string get_label();
    virtual json as_json() const = 0;
    virtual void from_json(const json& json_object) = 0;
    // virtual void to_string(std::string& str) const = 0;
    // virtual void from_string(const std::string& str) = 0;
    bool is_type(std::type_index type);
    bool is_type_compatible(const Parameter& other_parameter);
    void set_master(std::weak_ptr<Parameter> master_parameter);
    void copy_value_from_master();
    bool has_master() const;
    void clear_master();
    std::weak_ptr<Parameter> get_master() const;
  };

  template<typename T> class ParameterByReference : public Parameter {
    protected:
    T& value_;
    
    public:
    ParameterByReference(T& value, std::string label, std::string help);

    virtual json as_json() const override;
    virtual void from_json(const json& json_object) override;
    T& get();
    void set(T val);
  };
  template<typename T> class ParameterByValue : public Parameter {
    protected:
    T value_;

    public:
    ParameterByValue(T value, std::string label, std::string help);

    virtual json as_json() const override;
    virtual void from_json(const json& json_object) override;
    T& get();
    void set(T val);
  };

  template<typename T> class ParameterBounded : public ParameterByReference<T> {
    T min_, max_;
    public:
    ParameterBounded(T& val, T min, T max, std::string label, std::string help="");
    T min();
    T max();
    void set_bounds(T min, T max);
  };
  class ParamPath : public ParameterByReference<std::string> {
    using ParameterByReference::ParameterByReference;
  };

  typedef std::unordered_map<std::string,std::string> StrMap;
  class ParamStrMap : public ParameterByReference<StrMap> {
    public:
    std::vector<std::string>& key_options_;
    ParamStrMap(StrMap& val, std::vector<std::string>& key_options, std::string label, std::string help="");
  };

  class ParamSelector : public ParameterByReference<size_t> {
    std::vector<std::string> options;

    public:
    ParamSelector(std::vector<std::string> options, size_t& index, std::string label, std::string help="");

    std::vector<std::string> get_options();
    std::string get_selected_option();
  };

  extern template class ParameterByValue<float>;
  extern template class ParameterByValue<double>;
  extern template class ParameterByValue<int>;
  extern template class ParameterByValue<bool>;
  extern template class ParameterByValue<std::string>;
  extern template class ParameterByReference<float>;
  extern template class ParameterByReference<double>;
  extern template class ParameterByReference<int>;
  extern template class ParameterByReference<bool>;
  extern template class ParameterByReference<std::string>;
  extern template class ParameterByReference<std::pair<float,float>>;
  extern template class ParameterByReference<std::pair<int,int>>;
  extern template class ParameterByReference<std::pair<double,double>>;
  extern template class ParameterByReference<std::unordered_map<std::string,std::string>>;
  extern template class ParameterBounded<float>;
  extern template class ParameterBounded<double>;
  extern template class ParameterBounded<int>;
  
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