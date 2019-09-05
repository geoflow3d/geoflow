//
// Copyright (c) 2019 Rokas Kupstys.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#   define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "ImNodesEz.h"
#include <iostream>

namespace ImNodes
{

extern CanvasState* gCanvas;

namespace Ez
{

bool BeginNode(void* node_id, ImVec2* pos, bool* selected)
{
    bool result = ImNodes::BeginNode(node_id, pos, selected);
    auto gf_node = (geoflow::Node*)node_id;
    auto title = gf_node->get_name().c_str();

    auto* storage = ImGui::GetStateStorage();
    float node_width = storage->GetFloat(ImGui::GetID("node-width"));
    if (node_width > 0)
    {
        // Center node title
        ImVec2 title_size = ImGui::CalcTextSize(title);
        if (node_width > title_size.x)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + node_width / 2.f - title_size.x / 2.f);
    }

    // Render node title
    ImGui::TextUnformatted(title);
    // ImGui::SameLine();
    // ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s [%s]", 
            gf_node->get_type_name().c_str(), 
            gf_node->get_register().get_name().c_str()
        );
        ImGui::EndTooltip();
    }

    ImGui::BeginGroup();
    return result;
}

void EndNode()
{
    // Store node width which is needed for centering title.
    auto* storage = ImGui::GetStateStorage();
    ImGui::EndGroup();
    storage->SetFloat(ImGui::GetID("node-width"), ImGui::GetItemRectSize().x);
    ImNodes::EndNode();
}

bool Slot(geoflow::gfTerminal* term, int kind)
{
    auto* storage = ImGui::GetStateStorage();
    const auto& style = ImGui::GetStyle();
    const float CIRCLE_RADIUS = 4.f * gCanvas->zoom;
    auto title = term->get_name().c_str();
    ImVec2 title_size = ImGui::CalcTextSize(title);
    // Pull entire slot a little bit out of the edge so that curves connect into int without visible seams
    float item_offset_x = style.ItemSpacing.x * gCanvas->zoom;
    if (!ImNodes::IsOutputSlotKind(kind))
        item_offset_x = -item_offset_x;
    ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2{item_offset_x, 0});

    if (ImNodes::BeginSlot(term, title, kind))
    {
        auto* draw_lists = ImGui::GetWindowDrawList();

        // Slot appearance can be altered depending on curve hovering state.
        bool is_active = ImNodes::IsSlotCurveHovered() ||
                         (ImNodes::IsConnectingCompatibleSlot() /*&& !IsAlreadyConnectedWithPendingConnection(title, kind)*/);

        ImColor color = gCanvas->colors[is_active ? ImNodes::ColConnectionActive : ImNodes::ColConnection];

        ImGui::PushStyleColor(ImGuiCol_Text, color.Value);

        if (ImNodes::IsOutputSlotKind(kind))
        {
            // Align output slots to the right edge of the node.
            ImGuiID max_width_id = ImGui::GetID("output-max-title-width");
            float output_max_title_width = ImMax(storage->GetFloat(max_width_id, title_size.x), title_size.x);
            storage->SetFloat(max_width_id, output_max_title_width);
            float offset = (output_max_title_width + style.ItemSpacing.x) - title_size.x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

            ImGui::TextUnformatted(title);
            ImGui::SameLine();
        }

        ImRect circle_rect{
            ImGui::GetCursorScreenPos(),
            ImGui::GetCursorScreenPos() + ImVec2{CIRCLE_RADIUS * 2, CIRCLE_RADIUS * 2}
        };
        // Vertical-align circle in the middle of the line.
        float circle_offset_y = title_size.y / 2.f - CIRCLE_RADIUS;
        circle_rect.Min.y += circle_offset_y;
        circle_rect.Max.y += circle_offset_y;
        auto status_color = gCanvas->colors[term->has_data() ? ImNodes::ColNodeDoneBorder : ImNodes::ColNodeWaitingBorder];
        draw_lists->AddCircleFilled(circle_rect.GetCenter(), CIRCLE_RADIUS, color);
        draw_lists->AddCircle(circle_rect.GetCenter(), CIRCLE_RADIUS, status_color, 12, 2.0f);

        ImGui::ItemSize(circle_rect.GetSize());
        ImGui::ItemAdd(circle_rect, ImGui::GetID(title));

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f,1.0f,1.0f,1.0f));
            // ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            if (term->get_side()==geoflow::GF_IN) {
                auto* it = (geoflow::gfInputTerminal*) term;
                ImGui::Text("Input %s", it->is_optional() ? "(optional)" : "");
            } else {
                auto* ot = (geoflow::gfOutputTerminal*) term;
                ImGui::Text("Output (%lu connections)", ot->get_connections().size());
            }
            ImGui::Text("Has data: %s", term->has_data() ? "yes" : "no");
            if (term->get_family()==geoflow::GF_VECTOR ) {
                ImGui::TextUnformatted("Family: Vector");
                if(term->has_data()) {
                    if (term->get_side()==geoflow::GF_IN) {
                        auto* it = (geoflow::gfVectorMonoInputTerminal*) term;
                        ImGui::SameLine(); ImGui::Text("(size: %lu)", it->size());
                    } else{
                        auto* ot = (geoflow::gfVectorMonoOutputTerminal*) term;
                        ImGui::SameLine(); ImGui::Text("(size: %lu)", ot->size());
                    }
                }
            } else if (term->get_family()==geoflow::GF_BASIC )
                ImGui::TextUnformatted("Family: Basic");
            else if (term->get_family()==geoflow::GF_POLY )
                ImGui::TextUnformatted("Family: Poly");
            else
                ImGui::TextUnformatted("Family: Unknown");
            
            ImGui::Text("Compatible types:");
            for (auto& ti : term->get_types())
                ImGui::Text("  %s", ti.name());
            // ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();
            ImGui::EndTooltip();
        }

        if (ImNodes::IsInputSlotKind(kind))
        {
            ImGui::SameLine();
            ImGui::TextUnformatted(title);
        }

        ImGui::PopStyleColor();
        ImNodes::EndSlot();

        // A dirty trick to place output slot circle on the border.
        ImGui::GetCurrentWindow()->DC.CursorMaxPos.x -= item_offset_x;
        return true;
    }
    return false;
}

void InputSlots(const geoflow::Node::InputTerminalMap& slots)
{
    const auto& style = ImGui::GetStyle();

    // Render input slots
    ImGui::BeginGroup();
    {
        for (const auto& [name, term] : slots) {
            // const char* title = term->get_name().c_str();
            ImNodes::Ez::Slot(&(*term), ImNodes::InputSlotKind(1));
        }
    }
    ImGui::EndGroup();

    // Move cursor to the next column
    ImGui::SetCursorScreenPos({ImGui::GetItemRectMax().x + style.ItemSpacing.x, ImGui::GetItemRectMin().y});

    // Begin region for node content
    ImGui::BeginGroup();
}

void OutputSlots(const geoflow::Node::OutputTerminalMap& slots)
{
    const auto& style = ImGui::GetStyle();

    // End region of node content
    ImGui::EndGroup();

    // Render output slots in the next column
    ImGui::SetCursorScreenPos({ImGui::GetItemRectMax().x + style.ItemSpacing.x, ImGui::GetItemRectMin().y});
    ImGui::BeginGroup();
    {
        for (const auto& [name, term] : slots) {
            // const char* title = term->get_name().c_str();
            ImNodes::Ez::Slot(&(*term), ImNodes::OutputSlotKind(1));
        }
    }
    ImGui::EndGroup();
}

}

}
