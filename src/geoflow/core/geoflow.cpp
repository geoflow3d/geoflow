#include "geoflow.hpp"
#include <iostream>

using namespace geoflow;
  // TODO: what happens if connect several output terminals to 1 input terminal? -> should clear other connections to same input terminal
  void InputTerminal::push(std::any data) {
    cdata = data;
    parent.update();
    parent.on_push(*this);
  }
  void InputTerminal::clear() {
    parent.on_clear(*this);
    cdata.reset();
    parent.status = WAITING;
  }

  OutputTerminal::~OutputTerminal() {
    for(auto& conn : connections) {
      if (auto in = conn.lock()) {
        in->clear();
      }
    }
  }
  OutputTerminal::connection_set& OutputTerminal::get_connections(){
    for (auto conn = connections.begin(); conn != connections.end();) {
      if (auto c = conn->lock()) { 
        c->clear();
        ++conn;
      } else {
        conn = connections.erase(conn);
      }
    }
    return connections;
  }
  void OutputTerminal::push(std::any data) {
    cdata = data;
  }
  void OutputTerminal::clear() {
    cdata.reset();
  }
  void OutputTerminal::propagate() {
    for (auto& conn : get_connections()) {
      conn.lock()->push(cdata); // conn is the inputTerminal on the other end of the connection
    }
  }
  bool OutputTerminal::is_compatible(InputTerminal& in) {
    for (auto& in_type : in.get_types()) {
      if (type == in_type) return true;
    }
    return false;
  }
  void OutputTerminal::connect(InputTerminal& in) {
    //check type compatibility
    if (!is_compatible(in))
      throw Exception("Failed to connect. Terminals have incompatible types!");
    if (detect_loop(*this, *in.get_ptr().lock()))
      throw Exception("Failed to connect. Loop detected!");
    
    in.connected_type = type;
    parent.on_connect(*this);
    connections.insert(in.get_ptr());
    if (has_data()) {
      in.push(cdata);
    }
  }
  void OutputTerminal::disconnect(InputTerminal& in) { 
    connections.erase(in.get_ptr());
    // clear data to enforce a new connection must be made to this input terminal before the parent node can use it
    in.clear();
    in.parent.notify_children();
  }

  bool Node::set_name(std::string new_name) { 
    return manager.name_node(get_handle(), new_name); 
  };
  void Node::set_param(std::string name, Parameter param) {
    if (parameters.find(name) != parameters.end()) {
      if(parameters[name].index() == param.index())
        parameters[name] = param;
      else {
        std::cout << "Incorrect datatype for parameter: '" << name <<"', node type: " << type_name << "\n";
      }
    } else {
      std::cout << "No such parameter in this node: '" << name <<"', node type: " << type_name << "\n";
    }
  }
  void Node::set_params(ParameterMap new_map) {
    for (auto& kv : new_map) {
      set_param(kv.first, kv.second);
    }
  }
  const ParameterMap& Node::dump_params() {
    return parameters;
  }
  void Node::add_input(std::string name, std::initializer_list<TerminalType> types) {
    // TODO: check if name is unique key in inputTerminals map
    inputTerminals[name] = std::make_shared<InputTerminal>(
      *this, name, types
    );
  }
  void Node::add_input(std::string name, TerminalType type) {
    add_input(name, {type});
  }
  void Node::add_output(std::string name, TerminalType type) {
    outputTerminals[name] = std::make_shared<OutputTerminal>(
      *this, name, type
    );
  }
  bool Node::update() {
    for(auto &input : inputTerminals) {
      if(!input.second->has_data()) {
        status = WAITING;
        return false;
        }
    }
    manager.queue(get_handle());
    status = READY;
    return true;
  };
  void Node::propagate_outputs() {
    for (auto &oT : outputTerminals) {
      oT.second->propagate();
    }
  }
  void Node::notify_children() {
    std::queue<Node*> nodes_to_check;
    std::set<Node*> visited;
    nodes_to_check.push(this);

    while (!nodes_to_check.empty()) {
      auto n = nodes_to_check.front();
      nodes_to_check.pop();
      
      for (auto& oT : n->outputTerminals) {
         oT.second->clear(); // clear output terminal
        for (auto& conn : oT.second->get_connections()) {
          auto iT = conn.lock();
          iT->clear();
          auto child_node = iT->parent.get_handle().get();
          if (visited.count(child_node)==0) {
            visited.insert(child_node);
            nodes_to_check.push(child_node);
          }
        }
      }
    }
  }
  std::string Node::get_info() {
    std::stringstream s;
    s << "status: ";
    switch (status) {
      case WAITING: s << "WAITING"; break;
      case READY: s << "READY"; break;
      case PROCESSING: s << "PROCESSING"; break;
      case DONE: s << "DONE"; break;
      default: s << "UNKNOWN"; break;
    }
    s << "\n";
    return s.str();
  }

  void NodeManager::queue(std::shared_ptr<Node> n) {
    node_queue.push(n);
  }
  bool NodeManager::run(Node &node) {
    std::queue<std::shared_ptr<Node>>().swap(node_queue); // clear to prevent double processing of nodes ()
    if (node.update()) {
      node.notify_children();
      while (!node_queue.empty()) {
        auto n = node_queue.front();
        node_queue.pop();
        n->status = PROCESSING;
        std::cout << "Starting node " << n->get_name();
        n->process();
        std::cout << " ...processing complete\n";
        n->status = DONE;
        n->propagate_outputs();
      }
      return true;
    } 
    return false;
  }
  NodeHandle NodeManager::create_node(NodeRegister& node_register, std::string type_name) {
    // add node through a node register
    NodeHandle handle = node_register.create(type_name, *this);
    std::string new_name = type_name + "(" + std::to_string(++ID) + ")";
    handle->name = new_name;
    nodes[new_name] = handle;
    return handle;
  }
  NodeHandle NodeManager::create_node(NodeRegister& node_register, std::string type_name, std::pair<float,float> pos) {
    auto handle = create_node(node_register, type_name);
    handle->set_position(pos.first, pos.second);
    return handle;
  }
  void NodeManager::remove_node(NodeHandle node) {
    nodes.erase(node->get_name());
  }
  bool NodeManager::name_node(NodeHandle node, std::string new_name) {
    // rename a node, ensure uniqueness of name, return true if it wasn't already used
    if(nodes.find(new_name)==nodes.end()){
      node->name = new_name;
      return false;
    }
    remove_node(node);
    nodes[new_name] = node;
    node->name = new_name;
    return true;
  }
  std::vector<NodeHandle> NodeManager::dump_nodes() {
    std::vector<NodeHandle> node_dump;
    for (auto& kv : nodes) {
      node_dump.push_back(kv.second);
    }
    return node_dump;
  }
  NodeManager::ConnectionList NodeManager::dump_connections() {
    // collect all connections attached to nodes in this manager
    // return tuples, one for each connection: <output_node, input_node, output_term, input_term>
    ConnectionList connections;
    for (auto& kv : nodes) {
      auto node = kv.second;
      for (auto& output_term : node->outputTerminals) {
        for (auto& input_term : output_term.second->connections) {
          auto iT = input_term.lock();
          auto oT = output_term.second;
          connections.push_back(std::make_tuple(
            node->get_name(), 
            iT->parent.get_name(),
            oT->get_name(),
            iT->get_name()
          ));
        }
      }
    }
    return connections;
  }

  bool geoflow::connect(OutputTerminal& oT, InputTerminal& iT) {
    if (detect_loop(oT, iT))
      return false;
    oT.connect(iT);
    return true;
  }
  bool geoflow::connect(Node& n1, Node& n2, std::string s1, std::string s2) {
    auto& oT = n1.output(s1);
    auto& iT = n2.input(s2);
    return geoflow::connect(oT, iT);
  }
  bool geoflow::connect(NodeHandle n1, NodeHandle n2, std::string s1, std::string s2) {
    return geoflow::connect(n1->output(s1), n2->input(s2));
  }
  bool geoflow::connect(Terminal& t1, Terminal& t2) {
    auto& oT = dynamic_cast<OutputTerminal&>(t1);
    auto& iT = dynamic_cast<InputTerminal&>(t2);
    return geoflow::connect(oT, iT);
  }
  bool geoflow::is_compatible(Terminal& t1, Terminal& t2) {
    auto& oT = dynamic_cast<OutputTerminal&>(t1);
    auto& iT = dynamic_cast<InputTerminal&>(t2);
    return oT.is_compatible(iT);
  }
  void geoflow::disconnect(Terminal& t1, Terminal& t2) {
    auto& oT = dynamic_cast<OutputTerminal&>(t1);
    auto& iT = dynamic_cast<InputTerminal&>(t2);
    oT.disconnect(iT);
  }
  bool geoflow::detect_loop(Terminal& t1, Terminal& t2) {
    auto& oT = dynamic_cast<OutputTerminal&>(t1);
    auto& iT = dynamic_cast<InputTerminal&>(t2);
    return detect_loop(oT, iT);
  }
  bool geoflow::detect_loop(OutputTerminal& outputT, InputTerminal& inputT) {
    auto node = inputT.parent.get_handle();
    std::queue<std::shared_ptr<Node>> nodes_to_check;
    std::set<std::shared_ptr<Node>> visited;
    nodes_to_check.push(node);
    while (!nodes_to_check.empty()) {
      auto n = nodes_to_check.front();
      nodes_to_check.pop();
      
      for (auto& oT : n->outputTerminals) {
        if (oT.second.get() == &outputT){
          return true;
        }
        for (auto& c :oT.second->connections) {
          if (auto iT = c.lock()) {
            auto child_node = iT->parent.get_handle();
            if (visited.count(child_node)==0) {
              visited.insert(child_node);
              nodes_to_check.push(child_node);
            }
          }
        }
      }
    }
    return false;
  }