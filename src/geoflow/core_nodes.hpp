#include "geoflow.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace geoflow::nodes::core {

  class ProxyNode : public Node {
    public:
    using Node::Node;
    void init(){};
    void process(){};
  }

  class NestNode : public Node {
    private:
    std::string filepath_;
    NodeManager nested_node_manager_;
    std::vector<std::weak_ptr<gfInputTerminal>> nested_inputs_;
    std::vector<std::weak_ptr<gfOutputTerminal>> nested_outputs_;

    bool load_nodes() {
      if (fs::exists(filepath_)) {
        // load nodes from json file
        auto nodes = nested_node_manager_.load_json(filepath_);
        // find inputs and outputs to connect to this node's terminals...
        for (auto& node : nodes) {
          for (auto input_term : node->input_terminals) {
            if (!input_term->has_connection() && input_term->get_family() == GF_BASIC)
              nested_inputs_.push_back(input_term->get_ptr());
          }
          for (auto output_term : node->output_terminals) {
            if (output_term->is_marked() && input_term->get_family() == GF_BASIC)
              nested_outputs_.push_back(output_term->get_ptr());
          }
        }
        // create vectormonoinputs/outputs on this node
        for (auto output_term_ : nested_outputs_) {
          auto output_term = (gfBasicMonoOutputTerminal*)(output_term_.lock().get());
          add_vector_output(output_term->get_name(), output_term->get_type());
        }
        for (auto input_term_ : nested_inputs_) {
          auto input_term = (gfBasicMonoInputTerminal*)(input_term_.lock().get());
          add_vector_input(input_term->get_name(), input_term->get_type());
        }

        return true;
      }
      return false;
    }

    public:
    using Node::Node;

    void init() {
      nested_node_manager_ = manager; // this will only transfer the node registers
      add_param("filepath", ParamPath(filepath, "Flowchart file"));

      // load_nodes();
    };

    void process() {
      if(load_nodes()) {
        // set up proxy node
        NodeRegister R("ProxyRegister");
        R.register_node<ProxyNode>("Proxy");
        proxy_node = nested_node_manager_.create_node(R, "Proxy");
        for(auto nested_input : nested_inputs_) {
          auto input_term = (gfBasicMonoInputTerminal*)(nested_input.lock().get());
          proxy_node->add_output(input_term->get_name(), input_term->get_type());
          proxy_node->output(input_term->get_name()).connect(*nested_input);
        }
        // repack input data
        // assume all vector inputs have the same size
        std::map<std::string, std::vector<std::any>> output_vecs;
        size_t n = input_terminals[0]->get().size();
        for(size_t i=0; i<n; ++i) {
          // prep inputs
          for(auto nested_input : nested_inputs_) {
            auto data_vec = vector_input(nested_input.get_name())->get();
            proxy_node->output(nested_input.get_name()).set(data_vec[i]);
          }
          // run
          nested_node_manager_.run(proxy_node);
          // collect outputs
          for(auto nested_output : nested_outputs_) {
            auto output_term = (gfBasicMonoOutputTerminal*)(nested_output.lock().get());
            output_vecs[output_term->get_name()].push_back( output_term->get_data() );
          }
        }
        // push final output vecs
        for(auto nested_output : nested_outputs_) {
          output(nested_output->get_name()).set(output_vecs[nested_output->get_name()]);
        }
      }
    };
  };
}