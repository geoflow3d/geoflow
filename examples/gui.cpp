#include <iostream>
#include <fstream>

#include <viewer/app_povi.h>
#include <geoflow/gui/nodes.h>
#include <array>

#include "basic_nodes.hpp"

// #include <nlohmann/json.hpp>

// using json = nlohmann::json;

// #include <boost/program_options.hpp>


int main(int ac, const char * av[])
{
    NodeRegister R("Arithmetic");
    R.register_node<nodes::arithmetic::AdderNode>("Adder");
    R.register_node<nodes::arithmetic::NumberNode>("Number");
    NodeRegister R_gui("BasicShapes");
    R_gui.register_node<nodes::gui::CubeNode>("Cube");
    R_gui.register_node<nodes::gui::TriangleNode>("Triangle");

    NodeManager N;
    N.create_node(R_gui, "Cube", {0,-200});
    auto adder = N.create_node(R, "Adder", {300,0});
    auto number = N.create_node(R, "Number", {0, 000});

    connect(number->output("result"), adder->input("in1"));
    connect(number->output("result"), adder->input("in2"));

    auto a = std::make_shared<poviApp>(1280, 800, "Geoflow");
    
    ImGui::Nodes nodes(N, *a, {R, R_gui});
    a->draw_that(&nodes);

    // std::ifstream i("../examples/basic.gf.json");
    // json j;
    // i >> j;

    // ImGui::NodeStore ns;
    // auto& nodes = j["nodes"];
    // for (json::iterator it = nodes.begin(); it != nodes.end(); ++it) {
    //     auto node = it.value();
    //     ns.push_back(std::make_tuple(node["type"], it.key(), ImVec2(node["canvas-pos"][0],node["canvas-pos"][1])));
    // }
    // nodes_.PreloadNodes(ns);
    
    // ImGui::LinkStore ls;
    // for (auto& link : j["links"]) {
    //     std::string source=link["source"];
    //     std::string target=link["target"];
    //     auto ns = source.find("::");
    //     auto nt = target.find("::");
    //     ls.push_back(std::make_tuple(source.substr(0,ns), target.substr(0,nt), source.substr(ns+2), target.substr(nt+2)));
    // }
    // nodes_.PreloadLinks(ls);

    a->run();
}