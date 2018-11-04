#pragma once

#include <algorithm>
#include <vector>
#include <array>
#include <glm/glm.hpp>


namespace geoflow {

typedef std::array<float,3> arr3f;
typedef std::vector<arr3f> vec3f;
typedef std::vector<std::array<float,2>> vec2f;
typedef std::vector<int> vec1i;
typedef std::vector<float> vec1f;
typedef std::vector<size_t> vec1ui;

enum GeometryType {
  point,
  point_set,
  line_string,
  linear_ring,
  triangle,
};

inline const size_t vertex_count(const vec3f& vec) {
  return vec.size();
}
inline const size_t vertex_count(const std::vector<vec3f>& vec_of_vec) {
  size_t result=0;
  for (auto& vec : vec_of_vec) {
    result += vec.size();
  }
  return result;
}
// inline const size_t vertex_count(const std::vector<std::array<arr3f,3>>& vec_of_vec) {
//   return vec_of_vec.size()*3;
// }

template<typename geom_def, GeometryType GT> class GeometryCollection {
  protected:
  const GeometryType geometry_type = GT;
  std::vector<geom_def> geometry_vec; // geom_def = arr3f, vec3f, vecvec3f etc

  public:
  size_t size() {
    return geometry_vec.size();
  }
  size_t vertex_count() {
    return vertex_count(geometry_vec);
  }
  size_t dimension() {
    return 3;
  }
  void push_back(const geom_def& geom) {
    geometry_vec.push_back(geom);
  }
  void clear() {
    geometry_vec.clear();
  }
  GeometryType type() {
    return geometry_type;
  }
  std::vector<geom_def>& geometries(){
    return geometry_vec;
  }
  geom_def& operator[](std::size_t idx) {
    return geometry_vec[idx];
  }
  const geom_def& operator[](std::size_t idx) const {
    return geometry_vec[idx];
  }
};

// could add template specialisations if we need special features for a geometry type collection
class TriangleCollection:public GeometryCollection<arr3f, triangle> {
  size_t size() {
    return geometry_vec.size()/3;
  }
  void push_back(const std::array<arr3f,3>& triangle) {
    geometry_vec.push_back(triangle[0]);
    geometry_vec.push_back(triangle[1]);
    geometry_vec.push_back(triangle[2]);
  }
};
class PointCollection:public GeometryCollection<arr3f, point> {};
// typedef GeometryCollection<arr3f, point> PointCollection;
class LineStringCollection:public GeometryCollection<vec3f, line_string> {};
class LinearRingCollection:public GeometryCollection<vec3f, linear_ring> {};
// typedef GeometryCollection<arr3f, triangle> TriangleCollection;


class Box {
  private:
  std::array<float,3> pmin, pmax;
  bool just_cleared;
  public:
  Box(){
      clear();
  }

	std::array<float, 3> min() {
		return pmin;
	}
	std::array<float, 3> max() {
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
  void add(Box& otherBox){
      add(otherBox.min().data());
      add(otherBox.max().data());
  }
  void add(vec3f& vec){
      for (auto& p : vec)
        add(p.data());
  }
  void clear(){
      pmin.fill(0);
      pmax.fill(0);
      just_cleared = true;
  }
  bool isEmpty(){
      return just_cleared;
  }
  glm::vec3 center(){
      return {(pmax[0]+pmin[0])/2, (pmax[1]+pmin[1])/2, (pmax[2]+pmin[2])/2};
  }
};

}