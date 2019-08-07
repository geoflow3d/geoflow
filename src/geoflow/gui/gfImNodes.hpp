
#include "geoflow/geoflow.hpp"
#include "povi_nodes.hpp"
#include "parameter_widgets.hpp"

// #ifndef IMGUI_DEFINE_MATH_OPERATORS
// #   define IMGUI_DEFINE_MATH_OPERATORS
// #endif
#include "ImNodesEz.h"

class gfImNodes : public RenderObject {
  private:
  geoflow::NodeManager& node_manager_;
  poviApp& app_;

  typedef std::vector<std::tuple<geoflow::NodeHandle, ImVec2, bool>> NodeDrawVec;
  NodeDrawVec node_draw_list_;

  public:
  void init_node_draw_list() {
    node_draw_list_.clear();
    for (auto& [name, node] : node_manager_.get_nodes()) {
      node_draw_list_.push_back(std::make_tuple(
        node, 
        ImVec2(node->position[0], node->position[1]),
        false
      ));
    }
  }

  gfImNodes(geoflow::NodeManager& node_manager, poviApp& app)
    : node_manager_(node_manager), app_(app) {
      init_node_draw_list();
    };

  void menu() {
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save to JSON", "Ctrl+S")) {
					#ifndef __APPLE__
					auto result = osdialog_file(OSDIALOG_SAVE, "flowchart.json", "JSON:json");
					#else
					std::optional<std::string> result = "flowchart.json";
					#endif
					if (result.has_value()) {
						for (auto& [node,pos,selected] : node_draw_list_) {
							node->set_position(pos.x, pos.y);
						}
						node_manager_.dump_json(result.value());
					}
				}
				if (ImGui::MenuItem("Load from JSON", "Ctrl+O")) {
					auto result = osdialog_file(OSDIALOG_OPEN, NULL, "JSON:json");
					if (result.has_value()) {
						node_manager_.clear();

						auto new_nodes = node_manager_.load_json(result.value());
						init_node_draw_list();
						// CenterScroll();
					}
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Clear flowchart")) {
					node_draw_list_.clear();
					node_manager_.clear();
				}
				if (ImGui::MenuItem("Center flowchart")) {
					// CenterScroll();
				}
				ImGui::EndMenu();
			}
		}
	}
    
  void render() {
    static ImNodes::CanvasState canvas{};
    canvas.style.curve_thickness = 2.f;

    const ImGuiStyle& style = ImGui::GetStyle();


    if (ImGui::Begin("Flowchart", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        // We probably need to keep some state, like positions of nodes/slots for rendering connections.
        ImNodes::BeginCanvas(&canvas);
        bool one_node_hovered = false;
        for (auto node_it = node_draw_list_.begin(); node_it!=node_draw_list_.end();) {
            auto& node = std::get<0>(*node_it);
            auto& pos = std::get<1>(*node_it);
            auto& selected = std::get<2>(*node_it);
            // Start rendering node
            if (ImNodes::Ez::BeginNode(node.get(), &pos, &selected))
            {
                // Render input nodes first (order is important)
                ImNodes::Ez::InputSlots(node->input_terminals);

                
                // Custom node content may go here
                // ImGui::Text("Content of %s", node->title);


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
                    auto& source_term = source_node->output(std::string(source_term_title));
                    auto& target_term = target_node->input(std::string(target_term_title));
                    // std::cerr << "connect " << source_node->get_name() << " [" << source_term_title << ", " << &source_term << "] to " << target_node->get_name() << " [" << target_term_title << ", " << &target_term << "]\n";
                    source_term.connect(target_term);
                    node_manager_.run(*target_node);
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
              if (node->get_register().get_name() == "Visualisation") {
                node->gui();
              } else { 
                geoflow::draw_parameters(node);
                ImGui::Text("%s", node->info().c_str());
              }
              if (ImGui::MenuItem("Run")) {		
                node_manager_.run(*node);
              }
              // if (ImGui::MenuItem("Destroy")) {					
              // 	gf_manager.run(*node);
              // 	// element_.node_slot0_ = nullptr;
              // }
              ImGui::EndPopup();
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
                  if (handle->get_type_name()=="Painter") {
                    auto* painter_node = (geoflow::nodes::gui::PainterNode*)(handle.get());
                    painter_node->add_to(app_);
                  }
\
                  node_draw_list_.push_back(std::make_tuple(
                    handle, 
                    ImVec2(),
                    false
                  ));
                  ImNodes::AutoPositionNode(handle.get());
                }
              }
              ImGui::EndMenu();
            }
          }
          ImGui::Separator();
          if (ImGui::MenuItem("Reset Zoom"))
              canvas.zoom = 1;

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
	void launch_flowchart(NodeManager& manager) {
		auto a = std::make_shared<poviApp>(1280, 800, "Geoflow");
		gfImNodes nodes(manager, *a);
		a->draw_that(&nodes);
		a->run();
	};
}