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
  
  public:
  std::string name = "mypainter";
  PoviPainterNode(NodeManager& manager):Node(manager, "PoviPainter") {
    painter = std::make_shared<Painter>();
    painter->set_data(nullptr, 0, {3,3});
    painter->attach_shader("basic.vert");
    painter->attach_shader("basic.frag");
    painter->set_drawmode(GL_TRIANGLES);
    // a.add_painter(painter, "mypainter");
    add_input("data", TT_vec_float);
  }

  std::shared_ptr<Painter> get_painter() {
    return painter;
  }

  void on_push(InputTerminal& t) {
    auto& d = std::any_cast<std::vector<float>&>(t.cdata);
    std::cout << d.size() << " " << "\n";
    painter->set_data(d.data(), d.size(), {3,3});
    std::cout << "set data on painter\n";
  }
  void on_clear(InputTerminal& t) {
    painter->set_data(nullptr, 0, {3,3}); // put empty array
  }

  void gui(){
    // type: points, lines, triangles
    // fp_painter->attach_shader("basic.vert");
    // fp_painter->attach_shader("basic.frag");
    // fp_painter->set_drawmode(GL_LINE_STRIP);
  }
};

class TriangleNode:public Node {
  public:
  // int thenumber=0;
  std::vector<float> vertices = {
        // Positions         // Colors
        0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,  // Bottom Right
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,  // Bottom Left
        0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f   // Top 
    };

  TriangleNode(NodeManager& manager):Node(manager, "Triangle") {
    add_output("out", TT_vec_float);
  }

  void gui(){
    // ImGui::InputInt("The number", &thenumber);
    ImGui::ColorEdit3("col1", &vertices.data()[3]);
    ImGui::ColorEdit3("col2", &vertices.data()[9]);
    ImGui::ColorEdit3("col3", &vertices.data()[15]);
  }

  void process(){
    // Set up vertex data (and buffer(s)) and attribute pointers
    set_value("out", vertices);
    // std::cout << "end NumberNode::process()" << "\n";
  }
};