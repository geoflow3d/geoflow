// This file is part of Geoflow
// Copyright (C) 2018-2022 Ravi Peters

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include "geoflow/geoflow.hpp"
#include "povi_nodes.hpp"
#include "parameter_widgets.hpp"
#include <thread>
#include "misc/cpp/imgui_stdlib.h"

#ifdef GF_BUILD_GUI_FILE_DIALOGS
  #include "osdialog.hpp"
#endif

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#   define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "ImNodesEz.h"

class gfImNodes : public RenderObject {
  private:
  geoflow::NodeManager& node_manager_;
  poviApp& app_;

  ImNodes::CanvasState canvas_;

  typedef std::vector<std::tuple<geoflow::NodeHandle, ImVec2, bool, std::string>> NodeDrawVec;
  NodeDrawVec node_draw_list_;

  std::string global_new_key="";

  std::string flowchart_file_;

  public:
  void init_node_draw_list() {
    node_draw_list_.clear();
    for (auto& [name, node] : node_manager_.get_nodes()) {
        if(node.get() == nullptr) {
            std::cout << "Failed to load node " << name << "\n";
            continue;
        }
      node_draw_list_.push_back(std::make_tuple(
        node, 
        ImVec2(node->position[0], node->position[1]),
        false,
        node->get_name()
      ));
      if(node->get_type_name() == "Painter" || node->get_type_name() == "VectorPainter") {
        auto* painter_node = (geoflow::nodes::gui::PainterNode*)(node.get());
        painter_node->add_to(app_);
      }
    }
  }

  gfImNodes(geoflow::NodeManager& node_manager, poviApp& app, std::string flowchart_file)
    : node_manager_(node_manager), app_(app), flowchart_file_(flowchart_file) {
      init_node_draw_list();
      canvas_.style.curve_thickness = 2.f;
    };
  ~gfImNodes() {
    node_draw_list_.clear();
    node_manager_.clear();
  }

  void menu() {
		{
			if (ImGui::BeginMenu("File"))
			{
        if (ImGui::MenuItem("Save", "")) {
          for (auto& [node,pos,selected,name_buffer] : node_draw_list_) {
            node->set_position(pos.x, pos.y);
          }
          node_manager_.dump_json(flowchart_file_);
        }
        #ifdef GF_BUILD_GUI_FILE_DIALOGS
          if (ImGui::MenuItem("Save as  ...", "")) {
            auto result = osdialog_file(OSDIALOG_SAVE, "flowchart.json", "JSON:json");
            if (result.has_value()) {
              for (auto& [node,pos,selected,name_buffer] : node_draw_list_) {
                node->set_position(pos.x, pos.y);
              }
              node_manager_.dump_json(result.value());
            }
          }
          if (ImGui::MenuItem("Load from JSON", "")) {
            auto result = osdialog_file(OSDIALOG_OPEN, NULL, "JSON:json");
            if (result.has_value()) {
              node_manager_.clear();

              // set current work directory to folder containing flowchart file
              auto abs_path = fs::absolute(fs::path(result.value()));
              fs::current_path(abs_path.parent_path());
              flowchart_file_ = abs_path.string();
              auto new_nodes = node_manager_.load_json(flowchart_file_);
              init_node_draw_list();
              canvas_.center_on_nodes = true;
              // CenterScroll();
            }
          }
        #endif
				ImGui::EndMenu();
			}
		}
    if (ImGui::BeginMenu("Flowchart")) {
        if (ImGui::MenuItem("Run all root nodes")) {
          try {
					  node_manager_.run_all();
          } catch (const gfException& e) {
            std::cerr << e.what() << "\n";
          }
				}
        // This sort of works, but very prone to crashes because most of geoflow is not threadsafe atm. Especially painters.
        // if (ImGui::MenuItem("Run all threaded")) {
        //   std::thread t_run(&geoflow::NodeManager::run_all, &node_manager_);
        //   t_run.detach();
        // }
				ImGui::Separator();
        if (ImGui::MenuItem("Clear flowchart")) {
					node_draw_list_.clear();
					node_manager_.clear();
				}
				if (ImGui::MenuItem("Center flowchart")) {
          canvas_.center_on_nodes = true;
				}
        ImGui::Separator();
        if (ImGui::MenuItem("Clear flowchart offset")) {
          (*node_manager_.data_offset)[0]=0;
          (*node_manager_.data_offset)[1]=0;
          (*node_manager_.data_offset)[2]=0;
					node_manager_.data_offset.reset();
				}
        ImGui::InputDouble("Offset X", &(*node_manager_.data_offset)[0]);
        ImGui::InputDouble("Offset Y", &(*node_manager_.data_offset)[1]);
        ImGui::InputDouble("Offset Z", &(*node_manager_.data_offset)[2]);
        // ImGui::InputDouble3("Offset", &(*node_manager_.data_offset));
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Globals")) {
      // std::string to_remove;
      for (auto it=node_manager_.global_flowchart_params.begin(); it!=node_manager_.global_flowchart_params.end(); ) {
        ImGui::PushID(it->first.c_str());
        auto gparam = it->second.get();
        if(ImGui::Button("X")) {
          node_manager_.global_flowchart_params.erase(it++);
          continue;
        } else {
          ++it;
        }
        ImGui::SameLine();
        draw_global_parameter(gparam);
        ImGui::PopID();
      }
      ImGui::InputTextWithHint("##key", "Global name", &global_new_key);
      ImGui::SameLine();
      static ImGuiComboFlags flags = 0;
      if (!global_new_key.empty())
      if (ImGui::BeginCombo("##createglobal", "create", flags)) // The second parameter is the label previewed before opening the combo.
      {
        if (ImGui::Selectable("as string", false)) {
          node_manager_.global_flowchart_params[global_new_key]=std::make_shared<ParameterByValue<std::string>>("", global_new_key, "");
          global_new_key="";
        }
        if (ImGui::Selectable("as bool", false)) {
          node_manager_.global_flowchart_params[global_new_key]=std::make_shared<ParameterByValue<bool>>(bool(), global_new_key, "");
          global_new_key="";
        }
        if (ImGui::Selectable("as float", false)) {
          node_manager_.global_flowchart_params[global_new_key]=std::make_shared<ParameterByValue<float>>(float(), global_new_key, "");
          global_new_key="";
        }
        if (ImGui::Selectable("as int", false)) {
          node_manager_.global_flowchart_params[global_new_key]=std::make_shared<ParameterByValue<int>>(int(), global_new_key, "");
          global_new_key="";
        }
        ImGui::EndCombo();
      }
      // if(ImGui::Button("Create") && !global_new_key.empty()) {
      //   node_manager_.global_flowchart_params[global_new_key]="";
      //   global_new_key="";
      // }
      ImGui::EndMenu();
    }
	}
    
  void render() {

    const ImGuiStyle& style = ImGui::GetStyle();


    if (ImGui::Begin("Flowchart", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        // We probably need to keep some state, like positions of nodes/slots for rendering connections.
        ImNodes::BeginCanvas(&canvas_);
        bool one_node_hovered = false;
        for (auto node_it = node_draw_list_.begin(); node_it!=node_draw_list_.end();) {
            auto& node = std::get<0>(*node_it);
            auto& pos = std::get<1>(*node_it);
            auto& selected = std::get<2>(*node_it);
            auto& name_buffer = std::get<3>(*node_it);
            // Start rendering node
            if (ImNodes::Ez::BeginNode(node.get(), &pos, &selected))
            {
                // Render input nodes first (order is important)
                ImNodes::Ez::InputSlots(node->input_terminals);

                // Custom node content may go here
                // ImGui::Text("Pos: %f, %f", pos.x, pos.y);

                // Render output nodes first (order is important)
                ImNodes::Ez::OutputSlots(node->output_terminals);

                // Store new connections when they are created
                void* source_node_=nullptr;
                void* target_node_=nullptr;
                const char* source_term_title=nullptr;
                const char* target_term_title=nullptr;
                if (ImNodes::GetNewConnection(&target_node_, &target_term_title,
                    &source_node_, &source_term_title))
                {
                    auto source_node = (geoflow::Node*)(source_node_);
                    auto target_node = (geoflow::Node*)(target_node_);
                    auto& source_term = source_node->output_terminals[std::string(source_term_title)];
                    auto& target_term = target_node->input_terminals[std::string(target_term_title)];
                    // std::cerr << "connect " << source_node->get_name() << " [" << source_term_title << ", " << &source_term << "] to " << target_node->get_name() << " [" << target_term_title << ", " << &target_term << "]\n";
                    source_term->connect(*target_term);
                    try {
                      node_manager_.run(*target_node);
                    } catch (const gfException& e) {
                      std::cerr << e.what() << "\n";
                    }                    
                }

                // Render output connections of this node
                for (const auto& [name, output_term] : node->output_terminals)
                {
                    void* source_node = node.get();
                    std::vector<std::shared_ptr<geoflow::gfInputTerminal>> to_delete;
                    for(const auto& connection : output_term->get_connections()) {
                      const auto input_term = connection.lock();
                      void* target_node = input_term->get_parent().get_handle().get();

                      if (!ImNodes::Connection(
                        target_node, 
                        input_term->get_name().c_str(), 
                        source_node,
                        output_term->get_name().c_str()
                      )) {
                        // Remove deleted connection
                        to_delete.push_back(input_term);
                      }
                    }
                    for (auto& input_term : to_delete) {
                      output_term->disconnect(*input_term);
                    }
                }
            }
            // Node rendering is done. This call will render node background based on size of content inside node.
            ImNodes::Ez::EndNode();

            ImGui::PushID(node.get());

            if(ImGui::IsItemHovered(
              ImGuiHoveredFlags_AllowWhenOverlapped) &&
              ImGui::IsMouseReleased(1) && 
              !ImGui::IsMouseDragging(1)
            ) {
              one_node_hovered |= true;
              ImGui::OpenPopup("NodeActionsContextMenu");
            }

            if (ImGui::BeginPopup("NodeActionsContextMenu"))
            {
              // ImGui::Text("%s", node->debug_info().c_str());
              // ImGui::Text("position: %.2f, %.2f", element_.node_slot0_->position_.x, element_.node_slot0_->position_.y);
              // node->gui();
              ImGui::InputText("##name", &name_buffer);
              ImGui::SameLine();
              if(ImGui::Button("Rename")) {
                if(!node_manager_.name_node(node, name_buffer))
                  name_buffer = node->get_name();
              }
              ImGui::Checkbox("Autorun", &(node->autorun));
              if (ImGui::MenuItem("Run")) {
                try {
                  node_manager_.run(*node);
                } catch (const gfException& e) {
                  std::cerr << e.what() << "\n";
                }
              }
              ImGui::Separator();
              
              node->gui();
              if (node->get_register().get_name() != "Visualisation") {
                if (ImGui::CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
                  geoflow::draw_parameters(node);
                  ImGui::Text("%s", node->info().c_str());
                }
              }
              // if (ImGui::MenuItem("Destroy")) {					
              // 	gf_manager.run(*node);
              // 	// element_.node_slot0_ = nullptr;
              // }
              ImGui::EndPopup();
            } else {
              name_buffer = node->get_name();
            }
            ImGui::PopID();

            if (selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete)) {
              node_manager_.remove_node(node);
              node_draw_list_.erase(node_it);
            } else
              ++node_it;
        }

        const ImGuiIO& io = ImGui::GetIO();
        if (
          !one_node_hovered && 
          ImGui::IsMouseReleased(1) && 
          ImGui::IsWindowHovered() && 
          !ImGui::IsMouseDragging(1)
        ) {
            ImGui::FocusWindow(ImGui::GetCurrentWindow());
            ImGui::OpenPopup("NodesContextMenu");
            canvas_.new_creation_mouse_pos = ImGui::GetMousePos();
        }

        if (ImGui::BeginPopup("NodesContextMenu"))
        {
          // add nodes where the popup was opened
          auto registers = node_manager_.get_node_registers();
          for(auto& [name, node_register] : registers) {
            if ( ImGui::BeginMenu(node_register->get_name().c_str())) {
              for (auto& kv : node_register->node_types) {
                auto type_name = kv.first;
                if (ImGui::MenuItem(type_name.c_str())) {
                  auto handle = node_manager_.create_node(node_register, type_name);
                  if (handle->get_type_name()=="Painter" || handle->get_type_name()=="VectorPainter") {
                    auto* painter_node = (geoflow::nodes::gui::PainterNode*)(handle.get());
                    painter_node->add_to(app_);
                  }
                  node_draw_list_.push_back(std::make_tuple(
                    handle, 
                    ImVec2(),
                    false,
                    handle->get_name()
                  ));
                  ImNodes::AutoPositionNode(handle.get());
                }
              }
              ImGui::EndMenu();
            }
          }
          // ImGui::Separator();
          // if (ImGui::MenuItem("Reset Zoom"))
          //     canvas.zoom = 1;

          // if (ImGui::IsAnyMouseDown() && !ImGui::IsWindowHovered())
          //     ImGui::CloseCurrentPopup();
          ImGui::EndPopup();
        }

        ImNodes::EndCanvas();
    }
    ImGui::End();
  };
};

namespace geoflow {
	void launch_gui(NodeManager& manager, std::string flowchart_file) {
		auto a = std::make_shared<poviApp>(1280, 800, "Geoflow - Technology Preview");
		gfImNodes nodes(manager, *a, flowchart_file);
		a->draw_that(&nodes);
		a->run();
	};
}