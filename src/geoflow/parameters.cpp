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

#include <string>
#include <map>
#include <variant>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <iostream>

#include "parameters.hpp"

namespace geoflow {
  Parameter::Parameter(std::string label, std::string help, std::type_index ttype) : label_(label), help_(help), type_(ttype) {};
  std::string Parameter::get_label() {
    return label_;
  }
  const std::string& Parameter::get_help() const {
    return help_;
  }
  bool Parameter::is_type(std::type_index type) {
    return type_ == type;
  }
  bool Parameter::is_type_compatible(const Parameter& other_parameter) {
    return type_ == other_parameter.type_;
  }
  void Parameter::set_master(std::weak_ptr<Parameter> master_parameter) {
    if (!is_type_compatible(*master_parameter.lock()))
      std::cout << "Attempting to set incompatible master parameter\n";
    else
      master_parameter_ = master_parameter;
  };
  void Parameter::copy_value_from_master() {
    if(!master_parameter_.expired()) {
      from_json(master_parameter_.lock()->as_json());
    }
  };
  bool Parameter::has_master() const {
    return !master_parameter_.expired();
  }
  void Parameter::clear_master() {
    master_parameter_.reset();
  }
  std::weak_ptr<Parameter> Parameter::get_master() const {
    return master_parameter_;
  }

  template <typename T> ParameterByReference<T>::ParameterByReference(T& value, std::string label, std::string help) 
    : value_(value), Parameter(label, help, typeid(T)) {};
  template <typename T> json ParameterByReference<T>::as_json() const {
    return json(value_);
  };
  template <typename T> void ParameterByReference<T>::from_json(const json& json_object) {
    value_ = json_object.get<T>();
  };
  template <typename T> T& ParameterByReference<T>::get() {
    return value_;
  }
  template <typename T> void ParameterByReference<T>::set(T val) {
    value_ = val;
  }

  template <typename T> ParameterByValue<T>::ParameterByValue(T value, std::string label, std::string help) 
    : value_(value), Parameter(label, help, typeid(T)) {};
  template <typename T> json ParameterByValue<T>::as_json() const {
    return json(value_);
  };
  template <typename T> void ParameterByValue<T>::from_json(const json& json_object) {
    value_ = json_object.get<T>();
  };
  template <typename T> T& ParameterByValue<T>::get() {
    return value_;
  }
  template <typename T> void ParameterByValue<T>::set(T val) {
    value_ = val;
  }

  template<typename T> ParameterBounded<T>::ParameterBounded(T& val, T min, T max, std::string label, std::string help) : ParameterByReference<T>(val, label, help), min_(min), max_(max) {};
  template<typename T> T ParameterBounded<T>::min() { return min_;};
  template<typename T> T ParameterBounded<T>::max() { return max_;};
  template<typename T> void ParameterBounded<T>::set_bounds(T min, T max) {
    min_ = min;
    max_ = max;
  };

  std::vector<std::string> ParamSelector::get_options() {
    return options;
  }
  std::string ParamSelector::get_selected_option() {
    return options.at(value_);
  }

  ParamStrMap::ParamStrMap(StrMap& val, std::vector<std::string>& key_options, std::string label, std::string help) : key_options_(key_options), ParameterByReference(val, label, help) {};

  ParamSelector::ParamSelector(std::vector<std::string> options, size_t& index, std::string label, std::string help) : ParameterByReference(index, label, help) {};

  template class ParameterByValue<float>;
  template class ParameterByValue<double>;
  template class ParameterByValue<int>;
  template class ParameterByValue<bool>;
  template class ParameterByValue<std::string>;
  template class ParameterByReference<float>;
  template class ParameterByReference<double>;
  template class ParameterByReference<int>;
  template class ParameterByReference<bool>;
  template class ParameterByReference<std::string>;
  template class ParameterByReference<std::pair<float,float>>;
  template class ParameterByReference<std::pair<int,int>>;
  template class ParameterByReference<std::pair<double,double>>;
  template class ParameterByReference<std::unordered_map<std::string,std::string>>;
  template class ParameterBounded<float>;
  template class ParameterBounded<double>;
  template class ParameterBounded<int>;
};