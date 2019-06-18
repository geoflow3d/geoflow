// This file is part of Geoflow
// Copyright (C) 2018-2019  Ravi Peters, 3D geoinformation TU Delft

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

#include <iostream>
#include <fstream>
#include <iomanip>

#include "geoflow.hpp"

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
    cdata = std::move(data);
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
      throw Exception("Failed to connect ouput " +get_name()+ " from "+parent.get_name()+" to input " + in.get_name() + " from " +in.parent.get_name()+ ". Terminals have incompatible types!");
      
    if (detect_loop(*this, *in.get_ptr().lock()))
      throw Exception("Failed to connect ouput " +get_name()+ " from "+parent.get_name()+" to input " + in.get_name() + " from " +in.parent.get_name()+ ". Loop detected!");
    
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

  void OutputGroup::connect(InputGroup& ig) {
      connections.insert(ig.get_ptr());
      ig.connections.insert(get_ptr());
    }

  void OutputGroup::disconnect(InputGroup& ig) {
    connections.erase(ig.get_ptr());
    ig.connections.erase(get_ptr());
    // ig.clear();
    ig.parent.notify_children();
  }

  void OutputGroup::propagate() {
    is_propagated=true;
    for (auto conn_ : connections) {
      auto input_group = conn_.lock();
      //        input_group-> ->clear();
      for (auto& [name, term] : terminals) {
        if(input_group->types.count(term->type)) {
          if(input_group->terminals.count(name)==0) {
            auto& iT = input_group->add(name, term->type);
            iT.cdata = term->cdata;
            iT.connected_type = term->type;
          } else {
            auto& iT = *input_group->terminals[name];
            iT.cdata = term->cdata;
            iT.connected_type = term->type;
          }
        }
      }
      input_group->parent.update();
    }
  }

  Node::~Node() {
//    std::cout<< "Destructing geoflow::Node " << type_name << " " << name << "\n";
    notify_children();
  }
  void Node::remove_from_manager() {
    manager.remove_node(get_handle());
  }
  bool Node::set_name(std::string new_name) { 
    return manager.name_node(get_handle(), new_name); 
  };
  void Node::set_param(std::string name, Parameter param, bool quiet) {
    if (parameters.find(name) != parameters.end()) {
      if(parameters[name].index() == param.index())
        parameters[name] = param;
      else {
        std::cout << "Incorrect datatype for parameter: '" << name <<"', node type: " << type_name << "\n";
      }
    } else if(!quiet) {
      std::cout << "No such parameter in this node: '" << name <<"', node type: " << type_name << "\n";
    }
  }
  void Node::set_params(ParameterMap new_map, bool quiet) {
    for (auto& kv : new_map) {
      set_param(kv.first, kv.second, quiet);
    }
  }
  const ParameterMap& Node::dump_params() {
    return parameters;
  }
  void Node::add_input(std::string name, std::initializer_list<std::type_index> types) {
    // TODO: check if name is unique key in inputTerminals map
    inputTerminals[name] = std::make_shared<InputTerminal>(
      *this, name, types
    );
  }
  void Node::add_input(std::string name, std::type_index type) {
    add_input(name, {type});
  }
  void Node::add_output(std::string name, std::type_index type) {
    outputTerminals[name] = std::make_shared<OutputTerminal>(
      *this, name, type
    );
  }
  void Node::preprocess() {
    for (auto& [name, oG] : outputGroups) {
      oG->is_propagated = false;
    }
  }
  bool Node::update() {
    bool success = true;
    for (auto& [name,iT] : inputTerminals) { // only check normal inputs, not input_groups!
//    for_each_input([&success](InputTerminal& iT) {
      if (!iT->has_data())
        success &= false;
    }
    for (auto& [name, iG] : inputGroups) {
      for (auto c : iG->connections) {
        if (!c.lock()->is_propagated)
          success &= false;
      }
    }
    if (!success) {
      status = WAITING;
      return false;
    }

    manager.queue(get_handle());
    status = READY;
    return true;
  };
  void Node::propagate_outputs() {
    for_each_output([](OutputTerminal& oT) {
      oT.propagate();
    });
    for(auto& [name,group] : outputGroups) {
      group->propagate();
    }
  }
  void Node::notify_children() {
    std::queue<Node*> nodes_to_check;
    std::set<Node*> visited;
    nodes_to_check.push(this);

    while (!nodes_to_check.empty()) {
      auto n = nodes_to_check.front();
      nodes_to_check.pop();
      
      n->for_each_output([&nodes_to_check, &visited](OutputTerminal& oT) {
        oT.clear(); // clear output terminal
        for (auto& conn : oT.get_connections()) {
          auto iT = conn.lock();
          iT->clear();
          auto child_node = iT->parent.get_handle().get();
          if (visited.count(child_node)==0) {
            visited.insert(child_node);
            nodes_to_check.push(child_node);
          }
        }
      });

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
  json Node::dump_json() {
    json n;
    n["type"] = {node_register->get_name(), get_type_name()};
    n["position"] = {position.x, position.y};
    for ( const auto& [pname, pvalue] : parameters ) {
      if (std::holds_alternative<bool>(pvalue))
        n["parameters"][pname] = param<bool>(pname);
      else if (std::holds_alternative<int>(pvalue))
        n["parameters"][pname] = param<int>(pname);
      else if (std::holds_alternative<float>(pvalue))
        n["parameters"][pname] = param<float>(pname);
      else if (std::holds_alternative<std::string>(pvalue))
        n["parameters"][pname] = param<std::string>(pname);
    }

    for (const auto& [name, oTerm] : outputTerminals) {
      std::vector<std::pair<std::string, std::string>> connection_vec;
      if(oTerm->connections.size() > 0) {
        for (auto& conn : oTerm->connections) {
          if (auto iTerm = conn.lock())
            connection_vec.push_back(std::make_pair(iTerm->parent.get_name(), iTerm->get_name()));
        }
        n["connections"][name] = connection_vec;
      }
    }
    return n;
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
        n->preprocess();
        // std::cout << "Starting node " << n->get_name();
        n->process();
        // std::cout << " ...processing complete\n";
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
  NodeHandle NodeManager::create_node(NodeRegisterHandle node_register, std::string type_name) {
    // add node through a node register
    NodeHandle handle = node_register->create(type_name, *this);
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
  NodeHandle NodeManager::create_node(NodeRegisterHandle node_register, std::string type_name, std::pair<float,float> pos) {
    auto handle = create_node(node_register, type_name);
    handle->set_position(pos.first, pos.second);
    return handle;
  }
  void NodeManager::remove_node(NodeHandle node) {
    nodes.erase(node->get_name());
  }
  void NodeManager::clear() {
    nodes.clear();
    data_offset.reset();
    ID=0;
  }
  bool NodeManager::name_node(NodeHandle node, std::string new_name) {
    // rename a node, ensure uniqueness of name, return true if it wasn't already used
    if(nodes.find(new_name)==nodes.end()) // check if new_name already exists
     if (nodes.find(node->get_name())!=nodes.end()) // check if we can find node's current name 
      if (nodes[node->get_name()] == node) { // node object must exist in nodes
        remove_node(node);
        nodes[new_name] = node;
        node->name = new_name;
        return true;
      }
    return false;
  }
  std::vector<NodeHandle> NodeManager::dump_nodes() {
    std::vector<NodeHandle> node_dump;
    for (auto& kv : nodes) {
      node_dump.push_back(kv.second);
    }
    return node_dump;
  }
  void NodeManager::dump_json(std::string filepath) {
    json j;
    j["nodes"] = json::object();
    for (auto& [name, handle] : nodes) {
      j["nodes"][name] = handle->dump_json();
    }
    std::ofstream o(filepath);
    o << std::setw(2) << j << std::endl;
  }
  std::vector<NodeHandle> NodeManager::load_json(std::string filepath, NodeRegisterMap& registers) {
    json j;
    std::vector<NodeHandle> new_nodes;
    std::ifstream i(filepath);
    if (i.peek() == std::ifstream::traits_type::eof()) {
      std::cout << "bad json file\n";
      return new_nodes;
    }
    i >> j;
    json nodes_j = j["nodes"];
    for (auto node_j : nodes_j.items()) {
      auto tt = node_j.value().at("type").get<std::array<std::string,2>>();
      if (registers.count(tt[0])) {
        std::array<float,2> pos = node_j.value().at("position");
        auto nhandle = create_node(registers.at(tt[0]), tt[1], {pos[0], pos[1]});
        new_nodes.push_back(nhandle);
        std::string node_name = node_j.key();
        name_node(nhandle, node_name);
        if (node_j.value().count("parameters")) {
          auto params_j = node_j.value().at("parameters");
          for (auto& pel : params_j.items()) {
            if (std::holds_alternative<bool>(nhandle->parameters[pel.key()]))
              nhandle->set_param(pel.key(), pel.value().get<bool>());
            else if (std::holds_alternative<int>(nhandle->parameters[pel.key()]))
              nhandle->set_param(pel.key(), pel.value().get<int>());
            else if (std::holds_alternative<float>(nhandle->parameters[pel.key()]))
              nhandle->set_param(pel.key(), pel.value().get<float>());
            else if (std::holds_alternative<std::string>(nhandle->parameters[pel.key()]))
              nhandle->set_param(pel.key(), pel.value().get<std::string>());
          }
        }
      } else {
        std::cout << "Could not load node of type " << tt[1] << ", register not found: " << tt[0] <<"\n";
      }
    }
    for (auto node_j : nodes_j.items()) {
      auto tt = node_j.value().at("type").get<std::array<std::string,2>>();
      if (registers.count(tt[0])) {
        auto nhandle = nodes[node_j.key()];
        if (node_j.value().count("connections")) {
          auto conns_j = node_j.value().at("connections");
          for (json::const_iterator conn_j = conns_j.begin(); conn_j!= conns_j.end(); ++conn_j) {
            for (json::const_iterator c=conn_j->begin(); c!=conn_j->end(); ++c) {
              auto cval = c.value().get<std::array<std::string,2>>();
              if (nodes.count(cval[0]))
                nhandle->output(conn_j.key()).connect(nodes[cval[0]]->input(cval[1]));
              else 
                std::cout << "Could not connect output " << conn_j.key() << "\n";
            }
          }
        }
      }
    }
    return new_nodes;
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
    auto& oT = static_cast<OutputTerminal&>(t1);
    auto& iT = static_cast<InputTerminal&>(t2);
    return geoflow::connect(oT, iT);
  }
//  bool geoflow::connect(TerminalGroup<OutputTerminal> input_group, TerminalGroup<InputTerminal> output_group) {
//    for (auto& [name, iT] : input_group.terminals) {
//      if(output_group.types.count(iT->type)) {
//        auto& oT = output_group.add(name, iT->type);
//        connect(oT, *iT);
//      }
//    }
//    return true;
//  }
  bool geoflow::is_compatible(Terminal& t1, Terminal& t2) {
    auto& oT = static_cast<OutputTerminal&>(t1);
    auto& iT = static_cast<InputTerminal&>(t2);
    return oT.is_compatible(iT);
  }
  void geoflow::disconnect(Terminal& t1, Terminal& t2) {
    auto& oT = static_cast<OutputTerminal&>(t1);
    auto& iT = static_cast<InputTerminal&>(t2);
    oT.disconnect(iT);
  }
  bool geoflow::detect_loop(Terminal& t1, Terminal& t2) {
    auto& oT = static_cast<OutputTerminal&>(t1);
    auto& iT = static_cast<InputTerminal&>(t2);
    return detect_loop(oT, iT);
  }
  bool geoflow::detect_loop(OutputTerminal& outputT, InputTerminal& inputT) {
    auto node = inputT.parent.get_handle();
    std::queue<std::shared_ptr<Node>> nodes_to_check;
    std::set<std::shared_ptr<Node>> visited;
    nodes_to_check.push(node);
    bool loop_detected=false;
    while (!nodes_to_check.empty()) {
      auto n = nodes_to_check.front();
      nodes_to_check.pop();
      
      n->for_each_output([&outputT, &nodes_to_check, &visited, &loop_detected](OutputTerminal& oT) {
        if (&oT == &outputT){
          loop_detected=true;
          return;
        }
        for (auto& c :oT.connections) {
          if (auto iT = c.lock()) {
            auto child_node = iT->parent.get_handle();
            if (visited.count(child_node)==0) {
              visited.insert(child_node);
              nodes_to_check.push(child_node);
            }
          }
        }
      });
    }
    return loop_detected;
  }
  geoflow::ConnectionList geoflow::dump_connections(std::vector<NodeHandle> node_vec) {
    // collect all connections attached to nodes in this manager
    // return tuples, one for each connection: <output_node, input_node, output_term, input_term>
    ConnectionList connections;
    for (auto& node : node_vec) {
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
