#include "geoflow.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "imgui.h"
#include "gloo.h"
#include "app_povi.h"

using namespace geoflow;

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

  TriangleNode(NodeManager& manager):Node(manager, "Triangle") {
    add_output("vertices", TT_vec3f);
    add_output("colors", TT_vec3f);
  }

  void gui(){
    ImGui::ColorEdit3("col1", colors[0].data());
    ImGui::ColorEdit3("col2", colors[1].data());
    ImGui::ColorEdit3("col3", colors[2].data());
  }

  void process(){
    set_value("vertices", vertices);
    set_value("colors", colors);
  }
};