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
#include <array>
#include <vector>
#include <optional>

#include <exception>
#include <algorithm>

namespace geoflow {

class gfException: public std::exception
{
public:
  explicit gfException(const std::string& message):
    msg_(message)
    {}
  virtual const char* what() const throw (){
    return msg_.c_str();
  }

protected:
    std::string msg_;
};

class gfObject {
  protected:
  std::string name_;

  public:
  gfObject(std::string name) : name_(name) {};
  const std::string& get_name() const { return name_; };
  const std::string* get_name_ptr() const { return &name_; };
};

typedef std::array<float,2> arr2f;
typedef std::array<float,3> arr3f;
typedef std::vector<arr3f> vec3f;
typedef std::vector<std::array<float,2>> vec2f;
typedef std::vector<int> vec1i;
typedef std::vector<bool> vec1b;
typedef std::vector<float> vec1f;
typedef std::vector<size_t> vec1ui;
typedef std::vector<std::string> vec1s;

typedef std::unordered_map< std::string, std::vector<float>> AttributeMap;

class Box {
  private:
  std::array<float,3> pmin, pmax;
  bool just_cleared;
  public:
  Box(){
      clear();
  }

	std::array<float, 3> min() const {
		return pmin;
	}
	std::array<float, 3> max() const {
		return pmax;
	}
  void set(std::array<float,3> nmin, std::array<float,3> nmax) {
    pmin = nmin;
    pmax = nmax;
    just_cleared = false;
  }
  void add(float p[]){
      if(just_cleared){
          pmin[0] = p[0];
          pmin[1] = p[1];
          pmin[2] = p[2];
          pmax[0] = p[0];
          pmax[1] = p[1];
          pmax[2] = p[2];
          just_cleared = false;
      }
      pmin[0] = std::min(p[0], pmin[0]);
      pmin[1] = std::min(p[1], pmin[1]);
      pmin[2] = std::min(p[2], pmin[2]);
      pmax[0] = std::max(p[0], pmax[0]);
      pmax[1] = std::max(p[1], pmax[1]);
      pmax[2] = std::max(p[2], pmax[2]);
  }
  void add(arr3f a){
    add(a.data());
  }
  void add(const Box& otherBox){
      add(otherBox.min());
      add(otherBox.max());
  }
  void add(Box& otherBox){
      add(otherBox.min());
      add(otherBox.max());
  }
  void add(vec3f& vec){
      for (auto& p : vec)
        add(p);
  }
  void clear(){
      pmin.fill(0);
      pmax.fill(0);
      just_cleared = true;
  }
  bool isEmpty() const {
      return just_cleared;
  }
  arr3f center() const {
      return {(pmax[0]+pmin[0])/2, (pmax[1]+pmin[1])/2, (pmax[2]+pmin[2])/2};
  }
};

class Geometry {
  protected:
  std::optional<Box> bbox;
  virtual void compute_box() =0;
  public:
  virtual size_t vertex_count() const=0;
  virtual const Box& box() {
    if (!bbox.has_value()){
      compute_box();
    }
    return *bbox;
  };
  size_t dimension() {
    return 3;
  }
  virtual float* get_data_ptr() = 0;
};

template<typename geom_def> class GeometryCollection : public Geometry, public std::vector<geom_def> {
};

// geometry types:
// typedef arr3f Point;
typedef std::array<arr3f, 3> Triangle;

typedef std::array<arr3f, 2> Segment;
// typedef vec3f LineString;
// typedef vec3f LinearRing;
// class Segment : public Geometry, public std::array<arr3f,2> {
//   protected:
//   void compute_box() {
//     if (!bbox.has_value()) {
//       bbox=Box();
//       bbox->add((*this)[0]);
//       bbox->add((*this)[1]);
//     }
//   };
//   public:
//   size_t vertex_count() {
//     return 2;
//   }
//   float* get_data_ptr() {
//     return (*this)[0].data();
//   };
// };
class LinearRing : public vec3f, public Geometry {
  protected:
  void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& t : *this){
        bbox->add(t);
      }
    }
  }
  public:
  size_t vertex_count() const {
    return size();
  }
  float* get_data_ptr() {
    return (*this)[0].data();
  }
};
class LineString : public vec3f, public Geometry {
  protected:
  void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& t : *this){
        bbox->add(t);
      }
    }
  }
  public:
  size_t vertex_count() const {
    return size();
  }
  float* get_data_ptr() {
    return (*this)[0].data();
  }
};


class TriangleCollection:public GeometryCollection<Triangle> {
  public:
  size_t vertex_count() const {
    return size()*3;
  }
  virtual void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& t : *this){
        bbox->add(t[0]);
        bbox->add(t[1]);
        bbox->add(t[2]);
      }
    }
  }
  float* get_data_ptr() {
    return (*this)[0][0].data();
  }
};
class SegmentCollection:public GeometryCollection<std::array<arr3f,2>> {
  public:
  size_t vertex_count() const {
    return size()*2;
  }
  virtual void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& t : *this){
        bbox->add(t[0]);
        bbox->add(t[1]);
      }
    }
  }
  float* get_data_ptr() {
    return (*this)[0][0].data();
  }
};
class PointCollection:public GeometryCollection<arr3f> {
  public:
  size_t vertex_count() const{
    return size();
  }
  virtual void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      bbox->add(*this);
    }
  }
  float* get_data_ptr() {
    return (*this)[0].data();
  }
};
// typedef GeometryCollection<arr3f, point> PointCollection;
class LineStringCollection:public GeometryCollection<vec3f> {
  public:
  size_t vertex_count() const{
    size_t result=0;
    for (auto& vec : *this) {
      result += vec.size();
    }
    return result;
  }
  void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& vec : *this){
        bbox->add(vec);
      }
    }
  }
  float* get_data_ptr() {
    return (*this)[0][0].data();
  }
};
class LinearRingCollection:public GeometryCollection<vec3f> {
  public:
  size_t vertex_count() const{
    size_t result=0;
    for (auto& vec : *this) {
      result += vec.size();
    }
    return result;
  }
  void compute_box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& vec : *this){
        bbox->add(vec);
      }
    }
  }
  float* get_data_ptr() {
    return (*this)[0][0].data();
  }
};

}
