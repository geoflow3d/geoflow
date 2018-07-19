#include <iostream>
#include <fstream>

#include "imgui.h"
#include "app_povi.h"
#include "nodes.h"
#include <array>

// #include <boost/program_options.hpp>

static poviApp a(1280, 800, "Step edge detector");
static ImGui::Nodes nodes_;

void on_draw() {
    ImGui::Begin("Nodes");
        nodes_.ProcessNodes();
    ImGui::End();
}

int main(int ac, const char * av[])
{
    a.draw_that(on_draw);
    a.run();
}