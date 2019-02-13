#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <random>
#include "../core/geoflow.hpp"
#include "../../viewer/gloo.h"
#include "../../viewer/app_povi.h"
#include "imgui_color_gradient.h"

namespace geoflow::nodes::gui {
  struct ColorMap {
    std::shared_ptr<Uniform> u_valmax, u_valmin;
    bool is_gradient=false;
    std::weak_ptr<Texture1D> tex;
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
      add_input("values", TT_vec1i);
      add_output("colormap", TT_colmap);
      texture = std::make_shared<Texture1D>();
      texture->set_interpolation_nearest();
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

    void on_push(InputTerminal& t) {
      if(&input("values") == &t) {
        count_values();
      }
    }

    void on_connect(OutputTerminal& t) {
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
    std::shared_ptr<Texture1D> texture;
    std::shared_ptr<Uniform1f> u_maxval, u_minval;
    unsigned char tex[256*3];
    ColorMap colormap;

    ImGradient gradient;
    ImGradientMark* draggingMark = nullptr;
    ImGradientMark* selectedMark = nullptr;

    int n_bins=100;
    float minval, maxval, bin_width;
    size_t max_bin_count;
    vec1f histogram;

    public:
    using Node::Node;
    
    void init() {
      add_input("values", TT_vec1f);
      add_output("colormap", TT_colmap);
      texture = std::make_shared<Texture1D>();
      u_maxval = std::make_shared<Uniform1f>("u_value_max");
      u_minval = std::make_shared<Uniform1f>("u_value_min");
    }

    void update_texture(){
      gradient.getTexture(tex);
      if (texture->is_initialised())
        texture->set_data(tex, 256);
    }

    void compute_histogram(float min, float max) {
      auto data = input("values").get<vec1f>();
      histogram.resize(n_bins);
      for(auto& el:histogram) {
        el=0;
      }
      bin_width = (max-min)/(n_bins-1);
      for(auto& val : data) {
        if(val>max || val<min) continue;
        auto bin = std::floor((val-minval)/bin_width);
        if(bin>=0 && bin<n_bins)
          histogram[bin]++;
      }
      max_bin_count = *std::max_element(histogram.begin(), histogram.end());
    }

    void on_push(InputTerminal& t) {
      if(&input("values") == &t) {
        auto& d = t.get<vec1f&>();
        minval = *std::min_element(d.begin(), d.end());
        maxval = *std::max_element(d.begin(), d.end());
        compute_histogram(minval, maxval);
        u_maxval->set_value(maxval);
        u_minval->set_value(minval);
      }
    }

    void gui(){
      ImGui::DragFloatRange2("range", &u_minval->get_value(), &u_maxval->get_value(), 0.1f, minval, maxval, "Min: %.2f", "Max: %.2f");
      ImGui::DragInt("N of bins", &n_bins);
      if(input("values").has_data())
        if(ImGui::Button("Rescale histogram")){
          compute_histogram(u_minval->get_value(), u_maxval->get_value());
        }
      ImGui::PlotHistogram("Histogram", histogram.data(), histogram.size(), 0, NULL, 0.0f, (float)max_bin_count, ImVec2(200,80));
      if(ImGui::GradientEditor("Colormap", &gradient, draggingMark, selectedMark, ImVec2(200,80))){
        update_texture();
      }
    }

    void process(){
      update_texture();
      colormap.tex = texture;
      colormap.is_gradient = true;
      colormap.u_valmax = u_maxval;
      colormap.u_valmin = u_minval;
      output("colormap").set(colormap);
    }
  };

  class PainterNode:public Node {
    std::shared_ptr<Painter> painter;
    std::weak_ptr<poviApp> pv_app;
    
    public:
    // using Node::Node;
     PainterNode (NodeManager &nm, std::string type_name):Node(nm,type_name){
      painter = std::make_shared<Painter>();
      // painter->set_attribute("position", nullptr, 0, {3});
      // painter->set_attribute("value", nullptr, 0, {1});
      painter->attach_shader("basic.vert");
      painter->attach_shader("basic.frag");
      painter->set_drawmode(GL_TRIANGLES);
      // a.add_painter(painter, "mypainter");
      add_input("geometries", {
        TT_point_collection, 
        TT_triangle_collection,
        TT_segment_collection,
        TT_line_string_collection,
        TT_linear_ring_collection
        });
      add_input("normals", TT_vec3f);
      add_input("colormap", TT_colmap);
      add_input("values", TT_vec1f);
      add_input("identifiers", TT_vec1i);
    }
    ~PainterNode() {
      // note: this assumes we have only attached this painter to one poviapp
      if (auto a = pv_app.lock()) {
        std::cout << "remove painter\n";
        a->remove_painter(painter);
      } else std::cout << "remove painter failed\n";
    }
    void init(){}

    void add_to(poviApp& a, std::string name) {
      a.add_painter(painter, name);
      pv_app = a.get_ptr();
    }

    void map_identifiers() {
      if (input("identifiers").has_data() && input("colormap").has_data()) {
        auto cmap = input("colormap").get<ColorMap>();
        if (cmap.is_gradient) return;
        auto values = input("identifiers").get<vec1i>();
        vec1f mapped;
        for(auto& v : values){
          mapped.push_back(float(cmap.mapping[v])/256);
        }
        painter->set_attribute("identifier", mapped.data(), mapped.size(), 1);
      }
    }

    void on_push(InputTerminal& t) {
      // auto& d = std::any_cast<std::vector<float>&>(t.cdata);
      if(t.has_data() && painter->is_initialised()) {
        if(inputTerminals["geometries"].get() == &t) {
          if (t.connected_type == TT_point_collection) {
            auto& gc = t.get<PointCollection&>();
            painter->set_geometry(gc);
            painter->set_drawmode(GL_POINTS);
          } else if (t.connected_type == TT_triangle_collection) {
            auto& gc = t.get<TriangleCollection&>();
            painter->set_geometry(gc);
            painter->set_drawmode(GL_TRIANGLES);
          } else if(t.connected_type == TT_line_string_collection) {
            auto& gc = t.get<LineStringCollection&>();
            painter->set_geometry(gc);
            painter->set_drawmode(GL_LINE_STRIP);
          } else if(t.connected_type == TT_segment_collection) {
            auto& gc = t.get<SegmentCollection&>();
            painter->set_geometry(gc);
            painter->set_drawmode(GL_LINES);
          } else if (t.connected_type == TT_linear_ring_collection) {
            auto& gc = t.get<LinearRingCollection&>();
            painter->set_geometry(gc);
            painter->set_drawmode(GL_LINE_LOOP);
          }
        } else if(&input("normals") == &t) {
          auto& d = std::any_cast<vec3f&>(t.cdata);
          painter->set_attribute("normal", d[0].data(), d.size(), 3);
        } else if(&input("values") == &t) {
          auto& d = std::any_cast<vec1f&>(t.cdata);
          painter->set_attribute("value", d.data(), d.size(), 1);
        } else if(&input("identifiers") == &t) {
          map_identifiers();
        } else if(&input("colormap") == &t) {
          auto& cmap = t.get<ColorMap&>();
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
    void on_clear(InputTerminal& t) {
      // clear attributes...
      // painter->set_attribute("position", nullptr, 0, {3}); // put empty array
      if(&input("geometries") == &t) {
          painter->clear_attribute("position");
        } else if(&input("values") == &t) {
          painter->clear_attribute("value");
        } else if(&input("colormap") == &t) {
          if(t.cdata.has_value()) {
            auto& cmap = t.get<ColorMap&>();
            painter->unregister_uniform(cmap.u_valmax);
            painter->unregister_uniform(cmap.u_valmin);
          }
          painter->remove_texture();
        }
    }

    void gui(){
      painter->gui();
      // type: points, lines, triangles
      // fp_painter->attach_shader("basic.vert");
      // fp_painter->attach_shader("basic.frag");
      // fp_painter->set_drawmode(GL_LINE_STRIP);
    }
    void process(){};
  };

  class PoviPainterNode:public Node {
    std::shared_ptr<Painter> painter;
    std::weak_ptr<poviApp> pv_app;
    
    public:
    using Node::Node;
    void init() {
      painter = std::make_shared<Painter>();
      // painter->set_attribute("position", nullptr, 0, {3});
      // painter->set_attribute("value", nullptr, 0, {1});
      painter->attach_shader("basic.vert");
      painter->attach_shader("basic.frag");
      painter->set_drawmode(GL_TRIANGLES);
      // a.add_painter(painter, "mypainter");
      add_input("vertices", TT_vec3f);
      add_input("normals", TT_vec3f);
      add_input("colormap", TT_colmap);
      add_input("values", TT_vec1f);
      add_input("identifiers", TT_vec1i);
    }
    ~PoviPainterNode() {
      // note: this assumes we have only attached this painter to one poviapp
      if (auto a = pv_app.lock()) {
        std::cout << "remove painter\n";
        a->remove_painter(painter);
      } else std::cout << "remove painter failed\n";
    }

    void add_to(poviApp& a, std::string name) {
      a.add_painter(painter, name);
      pv_app = a.get_ptr();
    }

    void map_identifiers() {
      if (input("identifiers").has_data() && input("colormap").has_data()) {
        auto cmap = input("colormap").get<ColorMap>();
        if (cmap.is_gradient) return;
        auto values = input("identifiers").get<vec1i>();
        vec1f mapped;
        for(auto& v : values){
          mapped.push_back(float(cmap.mapping[v])/256);
        }
        painter->set_attribute("identifier", mapped.data(), mapped.size(), 1);
      }
    }

    void on_push(InputTerminal& t) {
      // auto& d = std::any_cast<std::vector<float>&>(t.cdata);
      if(t.has_data() && painter->is_initialised()) {
        if(&input("vertices") == &t) {
          auto& d = t.get<vec3f&>();
          painter->set_attribute("position", d[0].data(), d.size(), 3);
        } else if(&input("normals") == &t) {
          auto& d = t.get<vec3f&>();
          painter->set_attribute("normal", d[0].data(), d.size(), 3);
        } else if(&input("values") == &t) {
          auto& d = t.get<vec1f&>();
          painter->set_attribute("value", d.data(), d.size(), 1);
        } else if(&input("identifiers") == &t) {
          map_identifiers();
        } else if(&input("colormap") == &t) {
          auto& cmap = t.get<ColorMap&>();
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
    void on_clear(InputTerminal& t) {
      // clear attributes...
      // painter->set_attribute("position", nullptr, 0, {3}); // put empty array
      if(&input("vertices") == &t) {
          painter->clear_attribute("position");
        } else if(&input("values") == &t) {
          painter->clear_attribute("value");
        } else if(&input("colormap") == &t) {
          if(t.cdata.has_value()) {
            auto& cmap = t.get<ColorMap&>();
            painter->unregister_uniform(cmap.u_valmax);
            painter->unregister_uniform(cmap.u_valmin);
          }
          painter->remove_texture();
        }
    }

    void gui(){
      painter->gui();
      // type: points, lines, triangles
      // fp_painter->attach_shader("basic.vert");
      // fp_painter->attach_shader("basic.frag");
      // fp_painter->set_drawmode(GL_LINE_STRIP);
    }
    void process(){};
  };

  // class Vec3SplitterNode:public Node {
  //   public:

  //   Vec3SplitterNode(NodeManager& manager):Node(manager) {
  //     add_input("vec3f", TT_vec3f);
  //     add_output("x", TT_vec1f);
  //     add_output("y", TT_vec1f);
  //     add_output("z", TT_vec1f);
  //   }

  //   void gui(){
  //   }

  //   void process(){
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
      add_output("vertices", TT_vec3f);
      add_output("colors", TT_vec3f);
      add_output("attrf", TT_vec1f);
      add_output("attri", TT_vec1i);
    }

    void gui(){
      ImGui::ColorEdit3("col1", colors[0].data());
      ImGui::ColorEdit3("col2", colors[1].data());
      ImGui::ColorEdit3("col3", colors[2].data());
    }

    void process(){
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
      add_output("triangle_collection", TT_triangle_collection);
      add_output("normals", TT_vec3f);
    }

    void gui(){
    }

    void process(){
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