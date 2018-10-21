#include <iostream>
#include <fstream>

#include "imgui.h"
#include "app_povi.h"
#include "nodes.h"
#include <array>

#include "basic_nodes.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// #include <boost/program_options.hpp>

static auto a = std::make_shared<poviApp>(1280, 800, "Geoflow");
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

    std::ifstream i("../examples/basic.gf.json");
    json j;
    i >> j;

    ImGui::NodeStore ns;
    auto& nodes = j["nodes"];
    for (json::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        auto node = it.value();
        ns.push_back(std::make_tuple(node["type"], it.key(), ImVec2(node["canvas-pos"][0],node["canvas-pos"][1])));
    }
    nodes_.PreloadNodes(ns);
    
    ImGui::LinkStore ls;
    for (auto& link : j["links"]) {
        std::string source=link["source"];
        std::string target=link["target"];
        auto ns = source.find("::");
        auto nt = target.find("::");
        ls.push_back(std::make_tuple(source.substr(0,ns), target.substr(0,nt), source.substr(ns+2), target.substr(nt+2)));
    }
    nodes_.PreloadLinks(ls);

    a->run();
}