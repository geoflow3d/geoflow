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
    painter->set_data(nullptr, 0, {3,3});
    painter->attach_shader("basic.vert");
    painter->attach_shader("basic.frag");
    painter->set_drawmode(GL_TRIANGLES);
    // a.add_painter(painter, "mypainter");
    add_input("data", TT_vec_float);
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
    auto& d = std::any_cast<std::vector<float>&>(t.cdata);
    painter->set_data(d.data(), d.size(), {3,3});
  }
  void on_clear(InputTerminal& t) {
    painter->set_data(nullptr, 0, {3,3}); // put empty array
  }

  void gui(){
    const char* items[] = { "GL_POINTS", "GL_LINES", "GL_TRIANGLES", "GL_LINE_STRIP", "GL_LINE_LOOP" };
    static const char* item_current = items[2];            // Here our selection is a single pointer stored outside the object.
    if (ImGui::BeginCombo("draw mode", item_current)) // The second parameter is the label previewed before opening the combo.
    {
      for (int n = 0; n < IM_ARRAYSIZE(items); n++)
      {
        bool is_selected = (item_current == items[n]);
        if (ImGui::Selectable(items[n], is_selected))
          item_current = items[n];
          ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        if (item_current==items[0])
          painter->set_drawmode(GL_POINTS);
        else if (item_current==items[1])
          painter->set_drawmode(GL_LINES);
        else if (item_current==items[2])
          painter->set_drawmode(GL_TRIANGLES);
        else if (item_current==items[3])
          painter->set_drawmode(GL_LINE_STRIP);
        else if (item_current==items[4])
          painter->set_drawmode(GL_LINE_LOOP);
      }
        ImGui::EndCombo();
    }
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