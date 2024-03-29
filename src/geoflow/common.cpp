// This file is part of Geoflow
// Copyright (C) 2018-2022 Ravi Peters

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

#include <algorithm>
#include <iomanip>

#include "common.hpp"

namespace geoflow
{

Box::Box()
{
  clear();
}

std::array<float, 3> Box::min() const
{
  return pmin;
}
std::array<float, 3> Box::max() const
{
  return pmax;
}
void Box::set(std::array<float, 3> nmin, std::array<float, 3> nmax)
{
  pmin = nmin;
  pmax = nmax;
  just_cleared = false;
}
void Box::add(float p[])
{
  if (just_cleared)
  {
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
void Box::add(double p[])
{
  if (just_cleared)
  {
    pmin[0] = float(p[0]);
    pmin[1] = float(p[1]);
    pmin[2] = float(p[2]);
    pmax[0] = float(p[0]);
    pmax[1] = float(p[1]);
    pmax[2] = float(p[2]);
    just_cleared = false;
  }
  pmin[0] = std::min(float(p[0]), pmin[0]);
  pmin[1] = std::min(float(p[1]), pmin[1]);
  pmin[2] = std::min(float(p[2]), pmin[2]);
  pmax[0] = std::max(float(p[0]), pmax[0]);
  pmax[1] = std::max(float(p[1]), pmax[1]);
  pmax[2] = std::max(float(p[2]), pmax[2]);
}
void Box::add(arr3f a)
{
  add(a.data());
}
void Box::add(std::array<double, 3> a)
{
  add(a.data());
}
void Box::add(const Box &otherBox)
{
  add(otherBox.min());
  add(otherBox.max());
}
void Box::add(Box &otherBox)
{
  add(otherBox.min());
  add(otherBox.max());
}
void Box::add(vec3f &vec)
{
  for (auto &p : vec)
    add(p);
}
void Box::add(const vec3f &vec)
{
  for (auto &p : vec)
    add(p);
}
float Box::size_x() const
{
  return pmax[0] - pmin[0];
}
float Box::size_y() const
{
  return pmax[1] - pmin[1];
}
bool Box::intersects(Box &otherBox) const
{
  bool intersect_x = (pmin[0] < otherBox.pmax[0]) && (pmax[0] > otherBox.pmin[0]);
  bool intersect_y = (pmin[1] < otherBox.pmax[1]) && (pmax[1] > otherBox.pmin[1]);
  return intersect_x && intersect_y;
}
void Box::clear()
{
  pmin.fill(0);
  pmax.fill(0);
  just_cleared = true;
}
bool Box::isEmpty() const
{
  return just_cleared;
}
arr3f Box::center() const
{
  return {(pmax[0] + pmin[0]) / 2, (pmax[1] + pmin[1]) / 2, (pmax[2] + pmin[2]) / 2};
}

const Box &Geometry::box()
{
  if (!bbox.has_value())
  {
    compute_box();
  }
  return *bbox;
};
size_t Geometry::dimension()
{
  return 3;
}

// geometry helpers:
template<typename T> float ring_signed_area(T& ring)
{
  float result = 0;
  const auto n = ring.size();
  for (size_t i=0; i < n; ++i)
  {
    size_t i_n;
    if ( i == (n-1) ) {
      i_n = 0;
    } else {
      i_n = i+1;
    }

    result += ring[i][0]*ring[i_n][1] - ring[i_n][0]*ring[i][1];
  }
  return result/2;
}

// geometry types:

void LinearRing::compute_box()
{
  if (!bbox.has_value())
  {
    bbox = Box();
    for (auto &t : *this)
    {
      bbox->add(t);
    }
  }
}
float LinearRing::signed_area() const
{
  float result = ring_signed_area(*this);
  for (auto& iring : interior_rings_) {
    // ring_signed_area should be negative if iring is stored clockwise as it should
    result += ring_signed_area(iring);
  }
  return result;
}
size_t LinearRing::vertex_count() const
{
  return size();
}
float *LinearRing::get_data_ptr()
{
  return (*this)[0].data();
}
std::vector<vec3f>& LinearRing::interior_rings() {
  return interior_rings_;
}
const std::vector<vec3f>& LinearRing::interior_rings() const {
  return interior_rings_;
}

Segment::Segment() {}
Segment::Segment(arr3f source, arr3f target) {
  (*this)[0] = source;
  (*this)[1] = target;
}
void Segment::compute_box()
{
  if (!bbox.has_value())
  {
    bbox = Box();
    bbox->add((*this)[0]);
    bbox->add((*this)[1]);
  }
}
size_t Segment::vertex_count() const
{
  return 2;
}
float *Segment::get_data_ptr()
{
  return (*this)[0].data();
}

void LineString::compute_box()
{
  if (!bbox.has_value())
  {
    bbox = Box();
    for (auto &t : *this)
    {
      bbox->add(t);
    }
  }
}
size_t LineString::vertex_count() const
{
  return size();
}
float *LineString::get_data_ptr()
{
  return (*this)[0].data();
}

size_t PointCollection::vertex_count() const
{
  return size();
}
void PointCollection::compute_box()
{
  if (!bbox.has_value())
  {
    bbox = Box();
    bbox->add(*this);
  }
}
float *PointCollection::get_data_ptr()
{
  return (*this)[0].data();
}

attribute_vec_map& AttributeVecMap::get_attributes()
{
  return attribs_;
}
const attribute_vec_map& AttributeVecMap::get_attributes() const
{
  return attribs_;
}
bool AttributeVecMap::has_attributes() const
{
  return attribs_.size() != 0;
}
template<typename T> bool AttributeVecMap::is_attribute_type(const std::string& name) const{
  if (attribs_.find(name) != attribs_.end()) {
    return std::holds_alternative<T>(attribs_.at(name));
  }
  return false;
}
template<typename T> const T* AttributeVecMap::get_attribute(const std::string& name) const{
  if (attribs_.find(name) != attribs_.end()) {
    return std::get_if<T>(&attribs_.at(name));
  }
  return nullptr;
}
template<typename T> T* AttributeVecMap::get_attribute(const std::string& name) {
  if (attribs_.find(name) != attribs_.end()) {
    return std::get_if<T>(&attribs_.at(name));
  }
  return nullptr;
}
bool AttributeVecMap::is_attribute_vec1b(const std::string& name) const {
  return is_attribute_type<vec1b>(name);
}
bool AttributeVecMap::is_attribute_vec1i(const std::string& name) const {
  return is_attribute_type<vec1i>(name);
}
bool AttributeVecMap::is_attribute_vec1s(const std::string& name) const {
  return is_attribute_type<vec1s>(name);
}
bool AttributeVecMap::is_attribute_vec1f(const std::string& name) const {
  return is_attribute_type<vec1f>(name);
}
bool AttributeVecMap::is_attribute_vec3f(const std::string& name) const {
  return is_attribute_type<vec3f>(name);
}
const vec1b* AttributeVecMap::get_attribute_vec1b(const std::string& name) const {
  return get_attribute<vec1b>(name);
}
const vec1i* AttributeVecMap::get_attribute_vec1i(const std::string& name) const {
  return get_attribute<vec1i>(name);
}
const vec1s* AttributeVecMap::get_attribute_vec1s(const std::string& name) const {
  return get_attribute<vec1s>(name);
}
const vec1f* AttributeVecMap::get_attribute_vec1f(const std::string& name) const {
  return get_attribute<vec1f>(name);
}
const vec3f* AttributeVecMap::get_attribute_vec3f(const std::string& name) const {
  return get_attribute<vec3f>(name);
}
vec1b* AttributeVecMap::get_attribute_vec1b(const std::string& name) {
  return get_attribute<vec1b>(name);
}
vec1i* AttributeVecMap::get_attribute_vec1i(const std::string& name) {
  return get_attribute<vec1i>(name);
}
vec1s* AttributeVecMap::get_attribute_vec1s(const std::string& name) {
  return get_attribute<vec1s>(name);
}
vec1f* AttributeVecMap::get_attribute_vec1f(const std::string& name) {
  return get_attribute<vec1f>(name);
}
vec3f* AttributeVecMap::get_attribute_vec3f(const std::string& name) {
  return get_attribute<vec3f>(name);
}

vec1b& AttributeVecMap::add_attribute_vec1b(const std::string& name) {
  attribs_[name] = vec1b{};
  return std::get<vec1b>(attribs_.at(name));
}
vec1i& AttributeVecMap::add_attribute_vec1i(const std::string& name) {
  attribs_[name] = vec1i{};
  return std::get<vec1i>(attribs_.at(name));
}
vec1s& AttributeVecMap::add_attribute_vec1s(const std::string& name) {
  attribs_[name] = vec1s{};
  return std::get<vec1s>(attribs_.at(name));
}
vec1f& AttributeVecMap::add_attribute_vec1f(const std::string& name) {
  attribs_[name] = vec1f{};
  return std::get<vec1f>(attribs_.at(name));
}
vec3f& AttributeVecMap::add_attribute_vec3f(const std::string& name) {
  attribs_[name] = vec3f{};
  return std::get<vec3f>(attribs_.at(name));
}

size_t TriangleCollection::vertex_count() const
{
  return size() * 3;
}
void TriangleCollection::compute_box()
{
  if (!bbox.has_value())
  {
    bbox = Box();
    for (auto &t : *this)
    {
      bbox->add(t[0]);
      bbox->add(t[1]);
      bbox->add(t[2]);
    }
  }
}
float *TriangleCollection::get_data_ptr()
{
  return (*this)[0][0].data();
}

size_t SegmentCollection::vertex_count() const
{
  return size() * 2;
}
void SegmentCollection::compute_box()
{
  if (!bbox.has_value())
  {
    bbox = Box();
    for (auto &t : *this)
    {
      bbox->add(t[0]);
      bbox->add(t[1]);
    }
  }
}
float *SegmentCollection::get_data_ptr()
{
  return (*this)[0][0].data();
}

size_t LineStringCollection::vertex_count() const
{
  size_t result = 0;
  for (auto &vec : *this)
  {
    result += vec.size();
  }
  return result;
}
void LineStringCollection::compute_box()
{
  if (!bbox.has_value())
  {
    bbox = Box();
    for (auto &vec : *this)
    {
      bbox->add(vec);
    }
  }
}
float *LineStringCollection::get_data_ptr()
{
  return (*this)[0][0].data();
}

size_t LinearRingCollection::vertex_count() const
{
  size_t result = 0;
  for (auto &vec : *this)
  {
    result += vec.size();
  }
  return result;
}
void LinearRingCollection::compute_box()
{
  if (!bbox.has_value())
  {
    bbox = Box();
    for (auto &vec : *this)
    {
      bbox->add(vec);
    }
  }
}
float *LinearRingCollection::get_data_ptr()
{
  return (*this)[0][0].data();
}

void Mesh::push_polygon(LinearRing& polygon, int label) {
  polygons_.push_back(polygon);
  labels_.push_back(label);
}
// void Mesh::push_attribute(std::string name, std::any value) {
//   attributes_.at(name).values.push_back(value);
// }
std::vector<LinearRing>& Mesh::get_polygons() {
  return polygons_;
};
const std::vector<LinearRing>& Mesh::get_polygons() const {
  return polygons_;
};

std::vector<int>& Mesh::get_labels() {
  return labels_;
};
const std::vector<int>& Mesh::get_labels() const {
  return labels_;
};
// std::unordered_map<std::string, AttributeVec>&  Mesh::get_attributes(){
//   return attributes_;
// };
// const std::unordered_map<std::string, AttributeVec>&  Mesh::get_attributes()const {
//   return attributes_;
// };

void MultiTriangleCollection::push_back(
  TriangleCollection& trianglecollection)
{
  trianglecollections_.push_back(trianglecollection);
}
void MultiTriangleCollection::push_back(
  AttributeMap& attributemap)
{
  attributes_.push_back(attributemap);
}

size_t MultiTriangleCollection::tri_size() const
{
  return trianglecollections_.size();
}
size_t MultiTriangleCollection::attr_size() const
{
  return attributes_.size();
}

bool MultiTriangleCollection::has_attributes()
{
  return !attributes_.empty();
}
bool MultiTriangleCollection::has_attributes() const
{
  return !attributes_.empty();
}

std::vector<TriangleCollection>& MultiTriangleCollection::get_tricollections()
{
  return trianglecollections_;
}
const std::vector<TriangleCollection>& MultiTriangleCollection::get_tricollections() const
{
  return trianglecollections_;
}

std::vector<AttributeMap>& MultiTriangleCollection::get_attributes()
{
  return attributes_;
}
const std::vector<AttributeMap>& MultiTriangleCollection::get_attributes() const
{
  return attributes_;
}

TriangleCollection& MultiTriangleCollection::tri_at(size_t i)
{
  return trianglecollections_.at(i);
}
const TriangleCollection& MultiTriangleCollection::tri_at(size_t i) const
{
  return trianglecollections_.at(i);
}

AttributeMap& MultiTriangleCollection::attr_at(size_t i)
{
  return attributes_.at(i);
}
const AttributeMap& MultiTriangleCollection::attr_at(size_t i) const
{
  return attributes_.at(i);
}

std::vector<std::string> split_string(const std::string& s, std::string delimiter) {
  std::vector<std::string> parts;
  size_t last = 0;
  size_t next = 0;

  while ((next = s.find(delimiter, last)) != std::string::npos) { 
    parts.push_back(s.substr(last, next-last));
    last = next + 1;
  } 
  parts.push_back(s.substr(last));
  return parts;
}

  std::time_t Date::to_time_t() {
    std::tm tm{};
    tm.tm_year = this->year - 1900;
    tm.tm_mon = this->month - 1;
    tm.tm_mday = this->day;
    return std::mktime(&tm);
  }

  std::string Date::format_to_ietf() {
    std::time_t t = this->to_time_t();
    char timeString[std::size("yyyy-mm-dd")];
    std::strftime(std::data(timeString), std::size(timeString),
                  "%FT", std::gmtime(&t));
    std::string ret(timeString);
    return ret;
  }

  std::time_t DateTime::to_time_t() {
    std::tm tm{};
    tm.tm_year = this->date.year - 1900;
    tm.tm_mon = this->date.month - 1;
    tm.tm_mday = this->date.day;
    tm.tm_hour = this->time.hour;
    tm.tm_min = this->time.minute;
    tm.tm_sec = this->time.second;
    return std::mktime(&tm);
  }

  // Format to date-time, ignoring the time zone and assuming UTC.
  // According to https://datatracker.ietf.org/doc/html/rfc3339#section-5.6
  std::string DateTime::format_to_ietf() {
    std::time_t t = this->to_time_t();
    char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
    std::strftime(std::data(timeString), std::size(timeString),
                  "%FT%TZ", std::gmtime(&t));
    std::string ret(timeString);
    return ret;
  }
} // namespace geoflow
