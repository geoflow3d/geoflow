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
    if (!is_compatible(in) && !detect_loop(*this, *in.get_ptr().lock()))
      throw ConnectionException();
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

  void Node::add_input(std::string name, std::initializer_list<TerminalType> types) {
    inputTerminals[name] = std::make_shared<InputTerminal>(
      *this, types
    );
  }
  void Node::add_input(std::string name, TerminalType type) {
    add_input(name, {type});
  }
  void Node::add_output(std::string name, TerminalType type) {
    outputTerminals[name] = std::make_shared<OutputTerminal>(
      *this, type
    );
  }
  bool Node::update() {
    for(auto &input : inputTerminals) {
      if(!input.second->has_data()) {
        status = WAITING;
        return false;
        }
    }
    manager.queue(get_ptr());
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
          auto child_node = iT->parent.get_ptr().get();
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
        n->process();
        n->status = DONE;
        n->propagate_outputs();
      }
      return true;
    } 
    return false;
  }

  bool geoflow::connect(Node& n1, Node& n2, std::string s1, std::string s2) {
    auto oT = n1.outputs(s1);
    auto iT = n2.inputs(s2);
    if (detect_loop(oT, iT))
      return false;
    oT.connect(iT);
    return true;
  }
  bool geoflow::connect(Terminal& t1, Terminal& t2) {
    auto& oT = dynamic_cast<OutputTerminal&>(t1);
    auto& iT = dynamic_cast<InputTerminal&>(t2);
    if (detect_loop(oT, iT))
      return false;
    oT.connect(iT);
    return true;
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
    auto node = inputT.parent.get_ptr();
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
            auto child_node = iT->parent.get_ptr();
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