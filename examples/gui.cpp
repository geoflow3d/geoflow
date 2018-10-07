#include <iostream>
#include <fstream>

#include "imgui.h"
#include "app_povi.h"
#include "nodes.h"
#include <array>

#include "basic_nodes.hpp"

// #include <boost/program_options.hpp>

static auto a = std::make_shared<poviApp>(1280, 800, "Step edge detector");
static geoflow::NodeManager N;
static ImGui::Nodes nodes_(N, *a);

void on_draw() {
    ImGui::Begin("Nodes");
        nodes_.ProcessNodes();
    ImGui::End();
}

int main(int ac, const char * av[])
{
    N.register_node<TriangleNode>("Triangle");
    N.register_node<CubeNode>("Cube");
    N.register_node<ColorMapperNode>("ColorMapper");
    N.register_node<GradientMapperNode>("GradientMapper");
    N.register_node<AdderNode>("Adder");
    N.register_node<NumberNode>("Number");
    N.register_node<NumberNodeI>("NumberI");
    a->draw_that(on_draw);

    ImGui::NodeStore ns;
    ns.push_back(std::make_tuple("Cube", "TheCube", ImVec2(75,75)));
    ns.push_back(std::make_tuple("PoviPainter", "ThePoviPainter", ImVec2(300,75)));
    nodes_.PreloadNodes(ns);
    
    ImGui::LinkStore ls;
    ls.push_back(std::make_tuple("TheCube", "ThePoviPainter", "vertices", "vertices"));
    ls.push_back(std::make_tuple("TheCube", "ThePoviPainter", "normals", "normals"));
    nodes_.PreloadLinks(ls);

    a->run();
}