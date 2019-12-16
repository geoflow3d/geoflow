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

namespace geoflow {
  template <typename T> class ParameterBase {
    protected:
    T& value;
    std::string label;
    bool is_visible;
    public:
    ParameterBase(T& val, std::string label, bool visible=true) : value(val), label(label), is_visible(visible) {};
    std::string get_label() {
      return label;
    }
    T& get() {
      return value;
    }
    void set(T val) {
      value = val;
    }
    bool visible() {
      return is_visible;
    }
    void set_visible(bool vis) {
      is_visible=vis;
    }
  };
  template<typename T> class ParameterBounded : public ParameterBase<T> {
    T min_, max_;
    public:
    ParameterBounded(T& val, T min, T max, std::string label, bool visible=true) : ParameterBase<T>(val, label, visible), min_(min), max_(max) {};
    T min() { return min_;}
    T max() { return max_;}
    void set_bounds(T min, T max) {
      min_ = min;
      max_ = max;
    }
  };
  class ParamPath : public ParameterBase<std::string> {
    using ParameterBase::ParameterBase;
  };
  class ParamSelector : public ParameterBase<size_t> {
    std::vector<std::string> options;

    public:
    ParamSelector(std::vector<std::string> options, size_t& index, std::string label, bool visible=true) : ParameterBase(index, label, visible) {};

    std::vector<std::string> get_options() {
      return options;
    }
    std::string get_selected_option() {
      return options.at(value);
    }
  };
  typedef ParameterBase<float> ParamFloat;
  typedef ParameterBase<double> ParamDouble;
  typedef ParameterBase<std::pair<float,float>> ParamFloatRange;
  typedef ParameterBase<std::pair<int,int>> ParamIntRange;
  typedef ParameterBase<std::pair<double,double>> ParamDoubleRange;
  typedef ParameterBounded<float> ParamBoundedFloat;
  typedef ParameterBounded<double> ParamBoundedDouble;
  typedef ParameterBase<int> ParamInt;
  typedef ParameterBounded<int> ParamBoundedInt;
  typedef ParameterBase<bool> ParamBool;
  typedef ParameterBase<std::string> ParamString;

  typedef std::variant<
    ParamBool,
    ParamInt,
    ParamFloat,
    ParamDouble,
    ParamBoundedInt,
    ParamBoundedFloat,
    ParamBoundedDouble,
    ParamFloatRange,
    // ParamDoubleRange,
    ParamIntRange,
    ParamString,
    ParamPath,
    ParamSelector
    > ParameterVariant;
  typedef std::map<std::string, ParameterVariant> ParameterMap;
  // class ParameterSet : public ParameterMap {

  // };
}