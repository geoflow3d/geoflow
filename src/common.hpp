#pragma once

#include <algorithm>
#include <vector>
#include <array>
#include <optional>
#include <glm/glm.hpp>

namespace geoflow {

typedef std::array<float,3> arr3f;
typedef std::vector<arr3f> vec3f;
typedef std::vector<std::array<float,2>> vec2f;
typedef std::vector<int> vec1i;
typedef std::vector<float> vec1f;
typedef std::vector<size_t> vec1ui;

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
  glm::vec3 center() const {
      return {(pmax[0]+pmin[0])/2, (pmax[1]+pmin[1])/2, (pmax[2]+pmin[2])/2};
  }
};

template<typename geom_def> class GeometryCollection : public std::vector<geom_def> {
  protected:
  std::optional<Box> bbox;

  public:
  virtual size_t vertex_count()=0;
  virtual const Box& box()=0;
  size_t dimension() {
    return 3;
  }
};

// geometry types:
typedef arr3f Point;
typedef std::array<Point, 3> Triangle;
typedef vec3f LineString;
typedef vec3f LinearRing;
class TriangleCollection:public GeometryCollection<Triangle> {
  public:
  size_t vertex_count() {
    return size()*3;
  }
  virtual const Box& box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& t : *this){
        bbox->add(t[0]);
        bbox->add(t[1]);
        bbox->add(t[2]);
      }
    }
    return *bbox;
  }
};
class PointCollection:public GeometryCollection<Point> {
  public:
  size_t vertex_count() {
    return size();
  }
  virtual const Box& box() {
    if (!bbox.has_value()) {
      bbox=Box();
      bbox->add(*this);
    }
    return *bbox;
  }
};
// typedef GeometryCollection<arr3f, point> PointCollection;
class LineStringCollection:public GeometryCollection<LineString> {
  public:
  size_t vertex_count() {
    size_t result=0;
    for (auto& vec : *this) {
      result += vec.size();
    }
    return result;
  }
  virtual const Box& box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& vec : *this){
        bbox->add(vec);
      }
    }
    return *bbox;
  }
};
class LinearRingCollection:public GeometryCollection<LinearRing> {
  size_t vertex_count() {
    size_t result=0;
    for (auto& vec : *this) {
      result += vec.size();
    }
    return result;
  }
  virtual const Box& box() {
    if (!bbox.has_value()) {
      bbox=Box();
      for(auto& vec : *this){
        bbox->add(vec);
      }
    }
    return *bbox;
  }
};

}