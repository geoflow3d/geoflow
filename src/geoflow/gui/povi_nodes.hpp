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

#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <random>
#include "../geoflow.hpp"
#include "../../viewer/gloo.h"
#include "../../viewer/app_povi.h"
#include "imgui_color_gradient.h"

namespace geoflow::nodes::gui {
  struct ColorMap {
    std::shared_ptr<Uniform1f> u_valmax, u_valmin;
    bool is_gradient=false;
    std::shared_ptr<Texture1D> tex;
    std::unordered_map<int,int> mapping;
  };
  class ColorMapperNode:public Node {
    std::shared_ptr<Texture1D> texture;
    std::array<float,256*3> colors;
    unsigned char tex[256*3];
    ColorMap colormap;

    size_t n_bins=100;
    float minval, maxval, bin_width;
    std::map<int,int> value_counts;

    public:
    using Node::Node;
    
    void init() {
      add_input("values", typeid(vec1i));
      add_output("colormap", typeid(ColorMap));
      texture = std::make_shared<Texture1D>();
      texture->set_interpolation_nearest();
      texture->set_wrap_repeat();
      colors.fill(0);
    }
    void update_texture(){
      int i=0;
      for(auto& c : colors){
          tex[i] = c * 255;
          i++;
      }
      if (texture->is_initialised())
        texture->set_data(tex, 256);
    }

    void count_values() {
      value_counts.clear();
      auto data = input("values").get<vec1i>();
      for(auto& val : data) {
        value_counts[val]++;
      }
    }

    void on_receive(gfMonoInputTerminal& t) {
      if(&input("values") == &t) {
        count_values();
      }
    }

    void on_connect_output(gfOutputTerminal& t) {
      if(&output("colormap") == &t) {
        update_texture();
      }
    }

    void gui(){
      // ImGui::PlotHistogram("Histogram", histogram.data(), histogram.size(), 0, NULL, 0.0f, 1.0f, ImVec2(200,80));
      int i=0;
      if(ImGui::Button("Randomize colors")) {
        std::random_device rd;  //Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<> dis(0.0, 1.0);
        for(int i=0; i<colors.size()/3; i++) {
          ImGui::ColorConvertHSVtoRGB((float)dis(gen), 1.0, 1.0, colors[(i*3)+0], colors[(i*3)+1], colors[(i*3)+2]);
        }
        update_texture();
      }
      for (auto& cv : value_counts) {
        ImGui::PushID(i);
        if(ImGui::ColorEdit3("MyColor##3", (float*)&colors[i*3], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
          update_texture();
        ImGui::SameLine();
        ImGui::Text("%d [%d]", cv.first, cv.second);
        ImGui::PopID();
        if(++i==256) break;
      }
    }

    void process(){
      int i=0;
      for (auto& cv : value_counts){
        colormap.mapping[cv.first] = i++;
      }
      colormap.tex = texture;
      output("colormap").set(colormap);
    }
  };

  class GradientMapperNode:public Node {
    // std::shared_ptr<Texture1D> texture;
    // std::shared_ptr<Uniform1f> u_maxval, u_minval;
    unsigned char tex[256*3];
    ColorMap cmap;

    ImGradient gradient;
    ImGradientMark* draggingMark = nullptr;
    ImGradientMark* selectedMark = nullptr;

    int n_bins=30;
    float minval, maxval, bin_width;
    size_t max_bin_count;
    vec1f histogram;

    public:
    using Node::Node;
    
    void init() {
      add_input("values", typeid(vec1f));
      add_output("colormap", typeid(ColorMap));
      cmap.tex = std::make_shared<Texture1D>();
      cmap.tex->set_wrap_clamp();
      cmap.tex->set_interpolation_linear();
      cmap.is_gradient = true;
      cmap.u_valmax = std::make_shared<Uniform1f>("u_value_max");
      cmap.u_valmin = std::make_shared<Uniform1f>("u_value_min");
    }

    void update_texture(){
      gradient.getTexture(tex);
      if (cmap.tex->is_initialised())
        cmap.tex->set_data(tex, 256);
    }

    void compute_histogram(float min, float max) {
      if(!input("values").has_data()) return;
      auto& data = input("values").get<vec1f&>();
      histogram.resize(n_bins);
      std::fill(histogram.begin(), histogram.end(), 0);

      bin_width = (max-min)/(n_bins);
      for(auto& val : data) {
        if(val>max || val<min) continue;
        auto bin = std::floor((val-cmap.u_valmin->get_value())/bin_width);
        if(bin>=0 && bin<n_bins)
          histogram[bin]++;
      }
      max_bin_count = *std::max_element(histogram.begin(), histogram.end());
    }

    void on_receive(gfMonoInputTerminal& t) {
      if(&input("values") == &t) {
        auto& d = input("values").get<vec1f&>();
        minval = *std::min_element(d.begin(), d.end());
        maxval = *std::max_element(d.begin(), d.end());
        compute_histogram(minval, maxval);
        cmap.u_valmax->set_value(maxval);
        cmap.u_valmin->set_value(minval);
      }
    }

    void on_connect_output(gfOutputTerminal& ot){
      update_texture();
      output("colormap").set(cmap);
    }

    void gui() {
      if(ImGui::DragFloatRange2("range", &cmap.u_valmin->get_value(), &cmap.u_valmax->get_value(), 0.1f, minval, maxval, "Min: %.2f", "Max: %.2f") )
        compute_histogram(cmap.u_valmin->get_value(), cmap.u_valmax->get_value());
      if(ImGui::DragInt("N of bins", &n_bins, 1, 2, 100))
        compute_histogram(cmap.u_valmin->get_value(), cmap.u_valmax->get_value());

      ImGui::PlotHistogram("Histogram", histogram.data(), histogram.size(), 0, NULL, 0.0f, (float)max_bin_count, ImVec2(200,80));
      if(ImGui::GradientEditor("Colormap", &gradient, draggingMark, selectedMark, ImVec2(200,80))) {
        // update_texture();
      }
      update_texture();
      output("colormap").set(cmap);
    }

    void process() {
    }
  };

  class BasePainterNode:public Node {
    protected:
    std::shared_ptr<Painter> painter;
    std::weak_ptr<poviApp> pv_app;

    public:    
    BasePainterNode (NodeRegisterHandle nr, NodeManager &nm, std::string type_name, std::string node_name):Node(nr, nm, type_name, node_name) {
      painter = std::make_shared<Painter>();
      // painter->set_attribute("position", nullptr, 0, {3});
      // painter->set_attribute("value", nullptr, 0, {1});
      painter->attach_shader("basic.vert");
      painter->attach_shader("basic.frag");
      painter->set_drawmode(GL_TRIANGLES);
      // a.add_painter(painter, "mypainter");
    }
    ~BasePainterNode() {
      // note: this assumes we have only attached this painter to one poviapp
      if (auto a = pv_app.lock()) {
        std::cout << "remove painter\n";
        a->remove_painter(painter);
      } else std::cout << "remove painter failed\n";
    }
    void add_to(poviApp& a) {
      a.add_painter(painter, get_name_ptr());
      pv_app = a.get_ptr();
    }
  };

  class PainterNode:public BasePainterNode {
    
    public:
    using BasePainterNode::BasePainterNode;
    void init() {
      add_input("geometries", {
        typeid(PointCollection), 
        typeid(TriangleCollection),
        typeid(SegmentCollection),
        typeid(LineStringCollection),
        typeid(LinearRingCollection),
        typeid(LinearRing)
      });
      add_input("normals", typeid(vec3f));
      add_input("colormap", typeid(ColorMap));
      add_input("values", typeid(vec1f));
      add_input("identifiers", typeid(vec1i));
    }

    void map_identifiers() {
      if (input("identifiers").has_data() && input("colormap").has_data()) {
        auto cmap = input("colormap").get<ColorMap>();
        if (cmap.is_gradient) return;
        auto values = input("identifiers").get<vec1i>();
        vec1f mapped;
        for(auto& v : values) {
          mapped.push_back(float(cmap.mapping[v])/256);
        }
        painter->set_attribute("identifier", mapped.data(), mapped.size(), 1);
      }
    }

    void on_receive(gfMonoInputTerminal& t) {
      // auto& d = std::any_cast<std::vector<float>&>(t.cdata);
      if(t.has_data() && painter->is_initialised()) {
        if(input_terminals["geometries"].get() == &t) {
          if (t.is_connected_type(typeid(PointCollection))) {
            auto& gc = input("geometries").get<PointCollection&>();
            painter->set_geometry(gc);
            painter->set_drawmode(GL_POINTS);
          } else if (t.is_connected_type(typeid(TriangleCollection))) {
            auto& gc = input("geometries").get<TriangleCollection&>();
            painter->set_geometry(gc);
            painter->set_drawmode(GL_TRIANGLES);
          } else if(t.is_connected_type(typeid(LineStringCollection))) {
            auto& gc = input("geometries").get<LineStringCollection&>();
            painter->set_geometry(gc);
            painter->set_drawmode(GL_LINE_STRIP);
          } else if(t.is_connected_type(typeid(SegmentCollection))) {
            auto& gc = input("geometries").get<SegmentCollection&>();
            painter->set_geometry(gc);
            painter->set_drawmode(GL_LINES);
          } else if (t.is_connected_type(typeid(LinearRingCollection))) {
            auto& gc = input("geometries").get<LinearRingCollection&>();
            painter->set_geometry(gc);
            painter->set_drawmode(GL_LINE_LOOP);
          } else if (t.is_connected_type(typeid(LinearRing))) {
            auto& gc = input("geometries").get<LinearRing&>();
            LinearRingCollection lrc;
            lrc.push_back(gc);
            painter->set_geometry(lrc);
            painter->set_drawmode(GL_LINE_LOOP);
          }
        } else if(&input("normals") == &t) {
          auto& d = input("normals").get<vec3f&>();
          painter->set_attribute("normal", d[0].data(), d.size(), 3);
        } else if(&input("values") == &t) {
          auto d = input("values").get<vec1f>();
          painter->set_attribute("value", d.data(), d.size(), 1);
        } else if(&input("identifiers") == &t) {
          map_identifiers();
        } else if(&input("colormap") == &t) {
          auto& cmap = input("colormap").get<ColorMap&>();
          if(cmap.is_gradient) {
            painter->register_uniform(cmap.u_valmax);
            painter->register_uniform(cmap.u_valmin);
          } else {
            map_identifiers();
          }
          painter->set_texture(cmap.tex);
        }
      }
    }
    void on_clear(gfInputTerminal& t) {
      // clear attributes...
      // painter->set_attribute("position", nullptr, 0, {3}); // put empty array
      if(&input("geometries") == &t) {
          painter->clear_attribute("position");
        } else if(&input("values") == &t) {
          painter->clear_attribute("value");
        } else if(&input("colormap") == &t) {
          if(t.has_data()) {
            auto& cmap = input("colormap").get<ColorMap&>();
            if (cmap.is_gradient) {
              painter->unregister_uniform(cmap.u_valmax);
              painter->unregister_uniform(cmap.u_valmin);
            }
          }
          painter->remove_texture();
        }
    }

    void gui() {
      painter->gui();
      // type: points, lines, triangles
      // fp_painter->attach_shader("basic.vert");
      // fp_painter->attach_shader("basic.frag");
      // fp_painter->set_drawmode(GL_LINE_STRIP);
    }
    void process() {};
  };

  float* get_data_ptr(vec3f& data) { return data[0].data(); }
  float* get_data_ptr(vec1f& data) { return data.data(); }
  class VectorPainterNode:public BasePainterNode {
    public:
    using BasePainterNode::BasePainterNode;
    void init() {
      add_vector_input("geometries", {
        typeid(PointCollection), 
        typeid(TriangleCollection),
        // typeid(Segment),
        typeid(LineString),
        typeid(LinearRing),
        // typeid(LinearRing)
      });
      add_vector_input("normals", typeid(vec3f));
      add_input("colormap", typeid(ColorMap));
      add_vector_input("attributes", typeid(vec1f));
      add_vector_input("colors", typeid(vec3f));
      // add_vector_input("attributes", {typeid(vec1f),typeid(vec1i)});
    }

    void add_to(poviApp& a) {
      a.add_painter(painter, get_name_ptr());
      pv_app = a.get_ptr();
    }

    // void map_identifiers() {
    //   if (input("identifiers").has_data() && input("colormap").has_data()) {
    //     auto cmap = input("colormap").get<ColorMap>();
    //     if (cmap.is_gradient) return;
    //     auto values = input("identifiers").get<vec1i>();
    //     vec1f mapped;
    //     for(auto& v : values) {
    //       mapped.push_back(float(cmap.mapping[v])/256);
    //     }
    //     painter->set_attribute("identifier", mapped.data(), mapped.size(), 1);
    //   }
    // }

    template <typename T> void set_attribute(std::string name,  gfVectorMonoInputTerminal& aterm, size_t stride) {
      size_t ecount{}, offset{};
      for(size_t i=0; i< aterm.size(); ++i) {
        auto& attr = aterm.get<T&>(i);
        ecount += attr.size();
      }
      painter->begin_sub_attributes(name, ecount, stride);
      for(size_t i=0; i< aterm.size(); ++i) {
        auto& data = aterm.get<T&>(i);
        painter->set_sub_attributes(name, get_data_ptr(data), data.size(), offset);
      }
      painter->end_sub_attributes(name);
    }

    template<typename T> void set_geometry(gfVectorMonoInputTerminal& gterm) {
      size_t vcount{}, offset{};
      for(size_t i=0; i< gterm.size(); ++i) {
        auto& geom = gterm.get<T&>(i);
        vcount += geom.vertex_count();
      }
      painter->begin_sub_geometries(vcount, 3);
      for(size_t i=0; i<gterm.size(); ++i) {
        painter->set_sub_geometry(gterm.get<T&>(i), offset);
      }
      painter->end_sub_geometries();
    }

    void on_receive(gfMonoInputTerminal& t) {
      // auto& d = std::any_cast<std::vector<float>&>(t.cdata);
      if(t.has_data() && painter->is_initialised()) {
        if(input_terminals["geometries"].get() == &t) {
          auto& gterm = vector_input("geometries");
          if (t.is_connected_type(typeid(TriangleCollection))) {
            set_geometry<TriangleCollection>(gterm);
            painter->set_drawmode(GL_TRIANGLES);
          } else if (t.is_connected_type(typeid(PointCollection))) {
            set_geometry<PointCollection>(gterm);
            painter->set_drawmode(GL_POINTS);
          } else if (t.is_connected_type(typeid(LinearRing))) {
            set_geometry<LinearRing>(gterm);
            painter->set_drawmode(GL_LINE_LOOP);
          } else if (t.is_connected_type(typeid(LineString))) {
            set_geometry<LineString>(gterm);
            painter->set_drawmode(GL_LINE_STRIP);
          }
          // } else if (t.is_connected_type(typeid(Segment))) {
          //   set_geometry<Segment>(gterm);
          //   painter->set_drawmode(GL_LINES);
          // }
        } else if(input_terminals["normals"].get() == &t) {
          auto& aterm = vector_input("normals");
          set_attribute<vec3f>("normal", aterm, 3);
        } else if(input_terminals["colors"].get() == &t) {
          auto& aterm = vector_input("colors");
          set_attribute<vec3f>("color", aterm, 3);
        } else if(input_terminals["attributes"].get() == &t) {
          auto aterm = vector_input("attributes");
          set_attribute<vec1f>("value", aterm, 1);
        // } else if(&input("identifiers") == &t) {
        //   map_identifiers();
        } else if(&input("colormap") == &t) {
          auto& cmap = input("colormap").get<ColorMap&>();
          if(cmap.is_gradient) {
            painter->register_uniform(cmap.u_valmax);
            painter->register_uniform(cmap.u_valmin);
          } else {
            // map_identifiers();
          }
          painter->set_texture(cmap.tex);
        }
      }
    }
    void on_clear(gfInputTerminal& t) {
      // clear attributes...
      // painter->set_attribute("position", nullptr, 0, {3}); // put empty array
      if(input_terminals["geometries"].get() == &t) {
          painter->clear_attribute("position");
        } else if(input_terminals["normals"].get() == &t) {
          painter->clear_attribute("normal");
        } else if(input_terminals["colors"].get() == &t) {
          painter->clear_attribute("color");
        } else if(&input("colormap") == &t) {
          if(t.has_data()) {
            auto& cmap = input("colormap").get<ColorMap&>();
            if (cmap.is_gradient) {
              painter->unregister_uniform(cmap.u_valmax);
              painter->unregister_uniform(cmap.u_valmin);
            }
          }
          painter->remove_texture();
        }
    }

    void gui() {
      painter->gui();
    }
    void process() {};
  };

  // class Vec3SplitterNode:public Node {
  //   public:

  //   Vec3SplitterNode(NodeManager& manager):Node(manager) {
  //     add_input("vec3f", typeid(vec3f));
  //     add_output("x", typeid(vec1f));
  //     add_output("y", typeid(vec1f));
  //     add_output("z", typeid(vec1f));
  //   }

  //   void gui() {
  //   }

  //   void process() {
  //     auto v = input("vec3f").get<vec3f>();
  //     vec1f x,y,z;
  //     const size_t size = v.size();
  //     x.reserve(size);
  //     y.reserve(size);
  //     z.reserve(size);
  //     for (auto& el : v) {
  //       x.push_back(el[0]);
  //       y.push_back(el[1]);
  //       z.push_back(el[2]);
  //     }
  //     output("x").set(x);
  //     output("y").set(y);
  //     output("z").set(z);
  //   }
  // };
  class TriangleNode:public Node {
    public:
    using Node::Node;
    vec3f vertices = {
      {10.5f, 9.5f, 0.0f}, 
      {9.5f, 9.5f, 0.0f},
      {10.0f,  10.5f, 0.0f}
    };
    vec3f colors = {
      {1.0f, 0.0f, 0.0f}, 
      {0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 1.0f}
    };
    vec1f attrf = {1.0,5.5,10.0};
    vec1i attri = {1,42,42};

    void init() {
      add_output("vertices", typeid(vec3f));
      add_output("colors", typeid(vec3f));
      add_output("attrf", typeid(vec1f));
      add_output("attri", typeid(vec1i));
    }

    void gui() {
      ImGui::ColorEdit3("col1", colors[0].data());
      ImGui::ColorEdit3("col2", colors[1].data());
      ImGui::ColorEdit3("col3", colors[2].data());
    }

    void process() {
      output("vertices").set(vertices);
      output("colors").set(colors);
      output("attrf").set(attrf);
      output("attrf").set(attrf);
      output("attri").set(attri);
    }
  };
  class CubeNode:public Node {
    public:
    using Node::Node;
    void init() {
      add_output("triangle_collection", typeid(TriangleCollection));
      add_output("normals", typeid(vec3f));
    }

    void gui() {
    }

    void process() {
      typedef std::array<float, 3> point;
      point p0 = {-1.0f, -1.0f, -1.0f};
      point p1 = {1.0f, -1.0f, -1.0f};
      point p2 = {1.0f, 1.0f, -1.0f};
      point p3 = {-1.0f, 1.0f, -1.0f};

      point p4 = {-1.0f, -1.0f, 1.0f};
      point p5 = {1.0f, -1.0f, 1.0f};
      point p6 = {1.0f, 1.0f, 1.0f};
      point p7 = {-1.0f, 1.0f, 1.0f};

      TriangleCollection tc;
      tc.push_back({p2,p1,p0});
      tc.push_back({p0,p3,p2});
      tc.push_back({p4,p5,p6});
      tc.push_back({p6,p7,p4});
      tc.push_back({p0,p1,p5});
      tc.push_back({p5,p4,p0});
      tc.push_back({p1,p2,p6});
      tc.push_back({p6,p5,p1});
      tc.push_back({p2,p3,p7});
      tc.push_back({p7,p6,p2});
      tc.push_back({p3,p0,p4});
      tc.push_back({p4,p7,p3});

      vec3f normals;
      //counter-clockwise winding order
      for(auto& t : tc){
        auto a = glm::make_vec3(t[0].data());
        auto b = glm::make_vec3(t[1].data());
        auto c = glm::make_vec3(t[2].data());
        auto n = glm::cross(b-a, c-b);

        normals.push_back({n.x,n.y,n.z});
        normals.push_back({n.x,n.y,n.z});
        normals.push_back({n.x,n.y,n.z});
      }
      output("triangle_collection").set(tc);
      output("normals").set(normals);
    }
  };
}