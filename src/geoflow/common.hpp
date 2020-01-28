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

#include <array>
#include <vector>
#include <optional>
#include <unordered_map>

namespace geoflow
{

typedef std::array<float, 2> arr2f;
typedef std::array<float, 3> arr3f;
typedef std::vector<arr3f> vec3f;
typedef std::vector<std::array<float, 2>> vec2f;
typedef std::vector<int> vec1i;
typedef std::vector<bool> vec1b;
typedef std::vector<float> vec1f;
typedef std::vector<size_t> vec1ui;
typedef std::vector<std::string> vec1s;

typedef std::unordered_map<std::string, std::vector<float>> AttributeMap;

class Box
{
private:
  std::array<float, 3> pmin, pmax;
  bool just_cleared;

public:
  Box();

  std::array<float, 3> min() const;
  std::array<float, 3> max() const;
  void set(std::array<float, 3> nmin, std::array<float, 3> nmax);
  void add(float p[]);
  void add(arr3f a);
  void add(const Box &otherBox);
  void add(Box &otherBox);
  void add(vec3f &vec);
  void clear();
  bool isEmpty() const;
  arr3f center() const;
};

class Geometry
{
protected:
  std::optional<Box> bbox;
  virtual void compute_box() = 0;

public:
  virtual size_t vertex_count() const = 0;
  virtual const Box &box();
  size_t dimension();
  virtual float *get_data_ptr() = 0;
};

// geometry types:
// typedef arr3f Point;
typedef std::array<arr3f, 3> Triangle;
typedef std::array<arr3f, 2> Segment;

class LinearRing : public vec3f, public Geometry
{
  std::vector<vec3f> interior_rings_;
protected:
  void compute_box();

public:
  size_t vertex_count() const;
  float *get_data_ptr();
  std::vector<vec3f>& interior_rings();
  const std::vector<vec3f>& interior_rings() const;
};

// class Polygon : public LinearRing
// {
// public:
  // std::vector<vec3f> interior_rings_;
// };

class LineString : public vec3f, public Geometry
{
protected:
  void compute_box();

public:
  size_t vertex_count() const;
  float *get_data_ptr();
};

template <typename geom_def>
class GeometryCollection : public Geometry, public std::vector<geom_def>
{
};

class TriangleCollection : public GeometryCollection<Triangle>
{
public:
  size_t vertex_count() const;
  virtual void compute_box();
  float *get_data_ptr();
};

class SegmentCollection : public GeometryCollection<std::array<arr3f, 2>>
{
public:
  size_t vertex_count() const;
  virtual void compute_box();
  float *get_data_ptr();
};

class PointCollection : public GeometryCollection<arr3f>
{
public:
  size_t vertex_count() const;
  virtual void compute_box();
  float *get_data_ptr();
};

class LineStringCollection : public GeometryCollection<vec3f>
{
public:
  size_t vertex_count() const;
  void compute_box();
  float *get_data_ptr();
};

class LinearRingCollection : public GeometryCollection<vec3f>
{
public:
  size_t vertex_count() const;
  void compute_box();
  float *get_data_ptr();
};

} // namespace geoflow
