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
    T& min_, max_;
    public:
    ParameterRange(T& val, T& min, T& max) : ParameterBase<T>(val), min_(min), max_(max) {};
    T& min() { return min_;}
    T& max() { return max_;}
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

  typedef std::variant<ParamBool, ParamInt, ParamFloat, ParamIntRange, ParamFloatRange, ParamPath> ParameterVariant;
  typedef std::map<std::string, ParameterVariant> ParameterSet;
  // class ParameterSet : public ParameterMap {

  // };
}