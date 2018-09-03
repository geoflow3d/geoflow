#include "geoflow.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "imgui.h"
#include "gloo.h"
#include "app_povi.h"
#include "imgui_color_gradient.h"
#include <algorithm>

using namespace geoflow;

class ColorMapperNode:public Node {
  std::shared_ptr<Sampler> sampler;

  ImGradient gradient;
  ImGradientMark* draggingMark = nullptr;
  ImGradientMark* selectedMark = nullptr;

  size_t n_bins=10;
  float min, max, bin_width;
  vec1f histogram;

  public:
  ColorMapperNode(NodeManager& manager):Node(manager, "ColorMapper") {
    add_input("values", TT_vec1f);
    add_output("colormap", TT_colmap);
  }

  void on_push(InputTerminal& t) {
    if(inputTerminals["values"].get() == &t) {
      auto& d = std::any_cast<vec1f&>(t.cdata);
      min = *std::min_element(d.begin(), d.end());
      max = *std::max_element(d.begin(), d.end());
      histogram.clear();
      histogram.resize(n_bins,0);
      bin_width = (max-min)/(n_bins-1);
      for(auto& val : d) {
        auto bin = std::floor((val-min)/bin_width);
        histogram[bin]++;
      }
      auto max_bin_count = *std::max_element(histogram.begin(), histogram.end());
      for(auto &bin : histogram) bin /= max_bin_count;
    }
  }

  void gui(){
    ImGui::PlotHistogram("Histogram", histogram.data(), histogram.size(), 0, NULL, 0.0f, 1.0f, ImVec2(200,80));
    if(ImGui::GradientEditor("Colormap", &gradient, draggingMark, selectedMark, ImVec2(200,80))){
      
    }

  }
};

class PoviPainterNode:public Node {
  std::shared_ptr<Painter> painter;
  std::weak_ptr<poviApp> pv_app;
  
  public:
  std::string name = "mypainter";
  PoviPainterNode(NodeManager& manager):Node(manager, "PoviPainter") {
    painter = std::make_shared<Painter>();
    painter->set_attribute("position", nullptr, 0, {3});
    painter->attach_shader("basic.vert");
    painter->attach_shader("basic.frag");
    painter->set_drawmode(GL_TRIANGLES);
    // a.add_painter(painter, "mypainter");
    add_input("vertices", TT_vec3f);
  }
  ~PoviPainterNode(){
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

  void on_push(InputTerminal& t) {
    // auto& d = std::any_cast<std::vector<float>&>(t.cdata);
    auto& d = std::any_cast<vec3f&>(t.cdata);
    painter->set_attribute("position", d[0].data(), d.size()*3, {3});
  }
  void on_clear(InputTerminal& t) {
    painter->set_attribute("position", nullptr, 0, {3}); // put empty array
  }

  void gui(){
    painter->gui();
    // type: points, lines, triangles
    // fp_painter->attach_shader("basic.vert");
    // fp_painter->attach_shader("basic.frag");
    // fp_painter->set_drawmode(GL_LINE_STRIP);
  }
};

class TriangleNode:public Node {
  public:
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
  vec1f attr = {1.0,6.0,6.0, 6.5, 9.0,10.0};

  TriangleNode(NodeManager& manager):Node(manager, "Triangle") {
    add_output("vertices", TT_vec3f);
    add_output("colors", TT_vec3f);
    add_output("attr", TT_vec1f);
  }

  void gui(){
    ImGui::ColorEdit3("col1", colors[0].data());
    ImGui::ColorEdit3("col2", colors[1].data());
    ImGui::ColorEdit3("col3", colors[2].data());
  }

  void process(){
    set_value("vertices", vertices);
    set_value("colors", colors);
    set_value("attr", attr);
  }
};