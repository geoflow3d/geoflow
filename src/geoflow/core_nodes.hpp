#include "geoflow.hpp"
#ifdef GF_BUILD_WITH_GUI
  #include "imgui.h"
  #include "gui/parameter_widgets.hpp"
#endif

namespace geoflow::nodes::core {

  class ProxyNode : public Node {
    public:
    using Node::Node;
    void init(){};
    void process(){};
  };

  class NestNode : public Node {
    private:
    bool flowchart_loaded=false;
    std::string filepath_;
    std::unique_ptr<NodeManager> nested_node_manager_;
    std::vector<std::weak_ptr<gfInputTerminal>> nested_inputs_;
    std::vector<std::weak_ptr<gfOutputTerminal>> nested_outputs_;

    bool load_nodes() {
      if (fs::exists(filepath_)) {
        input_terminals.clear();
        output_terminals.clear();
        nested_node_manager_->clear();
        nested_outputs_.clear();
        nested_inputs_.clear();
        // load nodes from json file
        auto nodes = nested_node_manager_->load_json(filepath_);
        // find inputs and outputs to connect to this node's terminals...
        for (auto& node : nodes) {
          for (auto [name, input_term] : node->input_terminals) {
            if (!input_term->has_connection() && input_term->get_family() == GF_BASIC)
              nested_inputs_.push_back(input_term->get_ptr());
          }
          for (auto [name, output_term] : node->output_terminals) {
            if (output_term->is_marked() && output_term->get_family() == GF_BASIC)
              nested_outputs_.push_back(output_term->get_ptr());
          }
        }
        // create vectormonoinputs/outputs on this node
        for (auto input_term_ : nested_inputs_) {
          auto input_term = (gfBasicMonoInputTerminal*)(input_term_.lock().get());
          add_vector_input(input_term->get_name(), input_term->get_types());
        }
        for (auto output_term_ : nested_outputs_) {
          auto output_term = (gfBasicMonoOutputTerminal*)(output_term_.lock().get());
          add_vector_output(output_term->get_name(), output_term->get_type());
        }

        return true;
      }
      return false;
    }

    public:
    using Node::Node;

    void init() {
      nested_node_manager_ = std::make_unique<NodeManager>(manager.get_node_registers()); // this will only transfer the node registers
      add_param("filepath", ParamPath(filepath_, "Flowchart file"));

    };
    void post_parameter_load() {
      flowchart_loaded = load_nodes();
    }

    #ifdef GF_BUILD_WITH_GUI
      void gui() {
        for( auto& [name, node] : nested_node_manager_->get_nodes() ) {
          ImGui::PushID(node.get());
          if (ImGui::CollapsingHeader(name.c_str())) {
            geoflow::draw_parameters(node);
          }
          ImGui::PopID();
        }
        ImGui::Separator();
        if(ImGui::Button("Load Nodes"))
          flowchart_loaded = load_nodes();
      };
    #endif

    void process() {
      if(flowchart_loaded) {
        std::cout << "Begin processing for NestNode " << get_name() << "\n";
        // set up proxy node
        auto R = std::make_shared<NodeRegister>("ProxyRegister");
        R->register_node<ProxyNode>("Proxy");
        auto proxy_node = nested_node_manager_->create_node(R, "Proxy");
        for(auto nested_input : nested_inputs_) {
          auto input_term = (gfBasicMonoInputTerminal*)(nested_input.lock().get());
          auto& input_name = input_term->get_name();
          proxy_node->add_output(input_name, vector_input(input_name).get_connected_type());
          proxy_node->output(input_name).connect(*input_term);
        }
        // repack input data
        // assume all vector inputs have the same size
        // std::map<std::string, std::vector<std::any>> output_vecs;
        size_t n = vector_input(nested_inputs_[0].lock()->get_name()).get().size();
        for(size_t i=0; i<n; ++i) {
          // prep inputs
          for(auto nested_input_ : nested_inputs_) {
            auto& name = nested_input_.lock()->get_name();
            auto& data_vec = vector_input(name).get();
            proxy_node->output(name) = data_vec[i];
          }
          // run
          std::cout << "Processing item " << i+1 << "/" << n << "\n";
          for (auto& child : proxy_node->get_child_nodes()) {
            nested_node_manager_->run(child);
          }
          // collect outputs and push directly to vector outputs
          for(auto nested_output : nested_outputs_) {
            auto output_term = (gfBasicMonoOutputTerminal*)(nested_output.lock().get());
            if (output_term->has_data()) {
              auto& data = output_term->get_data();
              vector_output(output_term->get_name()).push_back_any(data);
            }
          }
        }
        std::cout << "End processing for NestNode " << get_name() << "\n";
      }
    };
  };
}