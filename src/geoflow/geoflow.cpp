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
#include <algorithm>

#include "geoflow.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace geoflow;

std::string random_string(size_t length) {
  auto randchar = []() -> char {
    const char charset[] =
    "0123456789"
    "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[ rand() % max_index ];
  };
  std::string str(length,0);
  std::generate_n( str.begin(), length, randchar );
  return str;
}

bool gfTerminal::accepts_type(std::type_index ttype) const {
  for (auto& t : types_) {
    if (t==ttype) 
      return true;
  }
  return false;
}

gfMonoInputTerminal::~gfMonoInputTerminal(){
  if (auto connected_output = connected_output_.lock())
    connected_output->connections_.erase(get_ptr());
}
bool gfMonoInputTerminal::is_connected_type(std::type_index ttype) const {
  if(auto output_term = connected_output_.lock()) {
    return output_term->accepts_type(ttype);
  }
  return false;
}
std::type_index gfMonoInputTerminal::get_connected_type() const {
  if(auto output_term = connected_output_.lock()) {
    return output_term->types_[0];
  }
  return typeid(void);
}

void gfInputTerminal::clear() {
  parent_.update_status();
  parent_.on_clear(*this);
}
void gfMonoInputTerminal::update_on_receive(bool queue) {
  if(has_data()) {
    if (queue && parent_.update_status() && parent_.autorun) 
      parent_.queue();
    parent_.on_receive(*this);
  }
}
bool gfMonoInputTerminal::has_connection() {
  return !connected_output_.expired();
}
bool gfMonoInputTerminal::has_data() {
  if (auto output_term = connected_output_.lock()) {
    return output_term->has_data();
  }
  return false;
}
void gfMonoInputTerminal::connect_output(gfOutputTerminal& output_term) {
  //check if we are already connected and if so disconnect from that output term first 
  if(auto output = connected_output_.lock()) {
    output->disconnect(*this);
  }
  connected_output_ = output_term.get_ptr();
}
void gfMonoInputTerminal::disconnect_output(gfOutputTerminal& output_term) {
  connected_output_.reset();
}


gfOutputTerminal::~gfOutputTerminal() {
  for(auto& conn : connections_) {
    if (auto in = conn.lock()) {
      in->clear();
    }
  }
}
bool gfOutputTerminal::is_compatible(gfInputTerminal& input_terminal) {
  bool type_compatible = true;
  // ensure that each output type of this terminal can be handles by the input_terminal
  for (auto& out_type : get_types()) {
    type_compatible &= input_terminal.accepts_type(out_type);
  }
  // Everything can  be connected to a POLY input, otherwise the families need to be equal
  bool family_compatible = (get_family()==input_terminal.get_family()) || (input_terminal.get_family() == GF_POLY);
  return family_compatible && type_compatible;
}
const InputConnectionSet& gfOutputTerminal::get_connections(){
  return connections_;
}
void gfOutputTerminal::propagate() {
  for (auto& conn : get_connections()) {
    if (has_data())
      conn.lock()->update_on_receive(true); // conn is the inputTerminal on the other end of the connection
  }
}
std::set<NodeHandle> gfOutputTerminal::get_child_nodes() {
  std::set<NodeHandle> child_nodes;
  for (auto& conn : connections_) {
    if (auto input_term = conn.lock()) {
      child_nodes.insert(input_term->get_parent().get_handle());
    }
  }
  return child_nodes;
}
void gfOutputTerminal::connect(gfInputTerminal& in) {
    //check type compatibility
  if (!is_compatible(in))
    throw gfException("Failed to connect ouput " +get_name()+ " from "+parent_.get_name()+" to input " + in.get_name() + " from " +in.parent_.get_name()+ ". Terminals have incompatible types!");
    
  if (detect_loop(*this, in))
    throw gfException("Failed to connect ouput " +get_name()+ " from "+parent_.get_name()+" to input " + in.get_name() + " from " +in.parent_.get_name()+ ". Loop detected!");

  in.connect_output(*this);
  connections_.insert(in.get_ptr());
  parent_.on_connect_output(*this);
  in.get_parent().on_connect_input(in);
  if (has_data()) {
    in.update_on_receive(false);
  }
};
void gfOutputTerminal::disconnect(gfInputTerminal& in) {
  connections_.erase(in.get_ptr());
  in.disconnect_output(*this);
  in.clear();
  in.parent_.notify_children();
};

void gfBasicMonoOutputTerminal::clear() {
  data_.reset();
}
bool gfBasicMonoOutputTerminal::has_data() {
  return data_.has_value();
}

const std::vector<std::any>& gfVectorMonoInputTerminal::get() {
  auto output_term = connected_output_.lock();
  auto sot = (gfVectorMonoOutputTerminal*)(output_term.get());
  return sot->get();
}
size_t gfVectorMonoInputTerminal::size() {
  auto output_term = connected_output_.lock();
  auto sot = (gfVectorMonoOutputTerminal*)(output_term.get());
  return sot->size(); 
}

void gfVectorMonoOutputTerminal::clear() {
  data_.clear();
}
bool gfVectorMonoOutputTerminal::has_data() {
  return data_.size()!=0;
}

gfPolyInputTerminal::~gfPolyInputTerminal(){
  for (auto output_term_ : connected_outputs_) {
    if(auto output_term = output_term_.lock())
      output_term->connections_.erase(get_ptr());
  }
}
void gfPolyInputTerminal::clear() {
  rebuild_terminal_refs();
  gfInputTerminal::clear();
}
void gfPolyInputTerminal::connect_output(gfOutputTerminal& output_term) {
  connected_outputs_.insert(output_term.get_ptr());
}
void gfPolyInputTerminal::disconnect_output(gfOutputTerminal& output_term) {
  connected_outputs_.erase(output_term.get_ptr());
}
void gfPolyInputTerminal::push_term_ref(gfOutputTerminal* term_ptr) {
  if (auto basic_term_ptr = dynamic_cast<gfBasicMonoOutputTerminal*>(term_ptr)) {
    basic_terminals_.push_back(basic_term_ptr);
  } else if (auto vector_term_ptr = dynamic_cast<gfVectorMonoOutputTerminal*>(term_ptr)) {
    vector_terminals_.push_back(vector_term_ptr);
  }
}
void gfPolyInputTerminal::rebuild_terminal_refs() {
  basic_terminals_.clear();
  vector_terminals_.clear();
  for (auto& wptr : connected_outputs_) {
    if (auto term = wptr.lock()) {
      auto term_ptr = term.get();
      if (auto poly_term_ptr = dynamic_cast<gfPolyOutputTerminal*>(term_ptr)) {
        for (auto& [name, sub_term] : poly_term_ptr->get_terminals()) {
          push_term_ref(sub_term.get());
        }
      } else {
        push_term_ref(term_ptr);
      }
    }
  }
}
void gfPolyInputTerminal::update_on_receive(bool queue) {
  rebuild_terminal_refs();
  if (parent_.update_status()) {
    parent_.on_receive(*this);
    if (queue && parent_.autorun)
      parent_.queue();
  }
}
bool gfPolyInputTerminal::has_data() {
  if (connected_outputs_.size()==0)
    return false;
  for (auto output_term_ : connected_outputs_){
    if (auto output_term = output_term_.lock()) {
      if (!output_term->has_data()) {
        return false;
      }
    }
  }
  return true;
}

void gfPolyOutputTerminal::clear() {
  // for (auto& [name, t] : terminals_) {
  //   t->clear();
  // }
  terminals_.clear();
}
bool gfPolyOutputTerminal::has_data() {
  for (auto& [name, t] : terminals_) {
    if(!t->has_data())
      return false;
  }
  return true;
}
gfBasicMonoOutputTerminal& gfPolyOutputTerminal::add(std::string term_name, std::type_index ttype ) {
      return add<gfBasicMonoOutputTerminal>(term_name, {ttype});
    };
gfVectorMonoOutputTerminal& gfPolyOutputTerminal::add_vector(std::string term_name, std::type_index ttype ) {
  return add<gfVectorMonoOutputTerminal>(term_name, {ttype});
};

Node::~Node() {
//    std::cout<< "Destructing geoflow::Node " << type_name << " " << name << "\n";
  notify_children();
}
void Node::remove_from_manager() {
  manager.remove_node(get_handle());
}
void Node::set_name(std::string new_name) { 
  name_ = new_name;
};
void Node::set_param(std::string name, ParameterVariant param, bool quiet) {
  if (parameters.find(name) != parameters.end()) {
    if(parameters.at(name).index() == param.index()) {
      parameters.emplace(name, param);
    } else {
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
std::set<NodeHandle> Node::get_child_nodes() {
  std::set<NodeHandle> child_nodes;
  for (auto& [name, oT] : output_terminals) {
    for (auto& conn : oT->get_connections()) {
      if (auto input_term = conn.lock()) {
        child_nodes.insert(input_term->get_parent().get_handle());
      }
    }
  }
  return child_nodes;
}
const ParameterMap& Node::dump_params() {
  return parameters;
}
// void Node::preprocess() {
//   for (auto& [name, oG] : outputGroups) {
//     oG->is_propagated = false;
//   }
// }
bool Node::update_status() {
  auto status_before = status_;
  bool success = true;
  for (auto& [name,iT] : input_terminals) {
    if (!iT->has_data() && !iT->is_optional())
      success &= false;
  }
  if (success)
    status_ = GF_NODE_READY;
  else
    status_ = GF_NODE_WAITING;
  return status_ != status_before;
}
bool Node::queue() {
  if(status_==GF_NODE_READY) {
    manager.queue(get_handle());
    return true;
  }
  return false;
};
void Node::propagate_outputs() {
  for_each_output([](gfOutputTerminal& oT) {
    oT.propagate();
  });
  // for(auto& [name,group] : outputGroups) {
  //   group->propagate();
  // }
}
void Node::notify_children() {
  std::queue<Node*> nodes_to_check;
  std::set<Node*> visited;
  nodes_to_check.push(this);

  while (!nodes_to_check.empty()) {
    auto n = nodes_to_check.front();
    nodes_to_check.pop();
    
    n->for_each_output([&nodes_to_check, &visited](gfOutputTerminal& oT) {
      oT.clear();
      for (auto& conn : oT.get_connections()) {
        if (auto iT = conn.lock()) {
          iT->clear();
          auto child_node = iT->get_parent().get_handle().get();
          if (visited.count(child_node)==0) {
            visited.insert(child_node);
            nodes_to_check.push(child_node);
          }
        }
      }
    });

  }
}
std::string Node::debug_info() {
  std::stringstream s;
  s << "addr: " << this << "\n";
  s << "status: ";
  switch (status_) {
    case GF_NODE_WAITING: s << "WAITING"; break;
    case GF_NODE_READY: s << "READY"; break;
    case GF_NODE_PROCESSING: s << "PROCESSING"; break;
    case GF_NODE_DONE: s << "DONE"; break;
    default: s << "UNKNOWN"; break;
  }
  s << "\n";
  s << "Outputerminals:\n";
  for (auto& [name, oT] : output_terminals) {
    s << "- [" << oT->has_data() <<"/"<< oT->get_connections().size() << "] " << name << ", " << &(*oT) << "\n";
  }
  return s.str();
}

void NodeManager::queue(std::shared_ptr<Node> n) {
  node_queue.push(n);
}
bool NodeManager::run() {
  // find all root nodes with autorun enabled
  bool ran_something = false;
  for (auto& [name, node] : nodes) {
    if(node->is_root() && node->autorun) {
      ran_something |= run(node);
    }
  }
  return ran_something;
}
bool NodeManager::run(Node &node) {
  std::queue<std::shared_ptr<Node>>().swap(node_queue); // clear to prevent double processing of nodes ()
  node.update_status();
  if (node.queue()) {
    node.notify_children();
    while (!node_queue.empty()) {
      auto n = node_queue.front();
      node_queue.pop();
      n->status_ = GF_NODE_PROCESSING;
      // n->preprocess();
      // std::cout << "Starting node " << n->get_name();
      n->process();
      // std::cout << " ...processing complete\n";
      n->status_ = GF_NODE_DONE;
      n->propagate_outputs();
    }
    return true;
  } 
  return false;
}
NodeHandle NodeManager::create_node(NodeRegisterHandle node_register, std::string type_name) {
  // add node through a node register
  std::string new_name = type_name + "-" + random_string(6);
  NodeHandle handle = node_register->create(
    new_name,
    type_name, 
    *this
  );
  nodes[new_name] = handle;
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
}
bool NodeManager::name_node(NodeHandle node, std::string new_name) {
  // rename a node, ensure uniqueness of name, return true if it wasn't already used
  if(nodes.find(new_name)==nodes.end()) // check if new_name already exists
    if (nodes.find(node->get_name())!=nodes.end()) // check if we can find node's current name 
    if (nodes[node->get_name()] == node) { // node object must exist in nodes
      remove_node(node);
      nodes[new_name] = node;
      node->set_name(new_name);
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
  for (auto& [name, node_handle] : nodes) {
    json n;
    n["type"] = {node_handle->node_register->get_name(), node_handle->get_type_name()};
    n["position"] = {node_handle->position[0], node_handle->position[1]};
    for ( auto& [pname, pvalue] : node_handle->parameters ) {
      if (auto ptr = std::get_if<ParamBool>(&pvalue))
        n["parameters"][pname] = ptr->get();
      else if (auto ptr = std::get_if<ParamInt>(&pvalue))
        n["parameters"][pname] = ptr->get();
      else if (auto ptr = std::get_if<ParamFloat>(&pvalue))
        n["parameters"][pname] = ptr->get();
      else if (auto ptr = std::get_if<ParamDouble>(&pvalue))
        n["parameters"][pname] = ptr->get();
      else if (auto ptr = std::get_if<ParamBoundedDouble>(&pvalue))
        n["parameters"][pname] = ptr->get();
      else if (auto ptr = std::get_if<ParamBoundedInt>(&pvalue))
        n["parameters"][pname] = ptr->get();
      else if (auto ptr = std::get_if<ParamBoundedFloat>(&pvalue))
        n["parameters"][pname] = ptr->get();
      else if (auto ptr = std::get_if<ParamIntRange>(&pvalue))
        n["parameters"][pname] = ptr->get();
      else if (auto ptr = std::get_if<ParamFloatRange>(&pvalue))
        n["parameters"][pname] = ptr->get();
      else if (auto ptr = std::get_if<ParamPath>(&pvalue))
        n["parameters"][pname] = ptr->get();
      else
        std::cerr << "Parameter '" << pname << "' has an unknown type and is not serialised!\n";
    }
    for (const auto& [name, oTerm] : node_handle->output_terminals) {
      std::vector<std::pair<std::string, std::string>> connection_vec;
      if(oTerm->get_connections().size() > 0) {
        for (auto& conn : oTerm->get_connections()) {
          if (auto iTerm = conn.lock())
            connection_vec.push_back(std::make_pair(iTerm->get_parent().get_name(), iTerm->get_name()));
        }
        n["connections"][name] = connection_vec;
      }
    }
    j["nodes"][name] = n;
  }
  std::ofstream o(filepath);
  o << std::setw(2) << j << std::endl;
}
std::vector<NodeHandle> NodeManager::load_json(std::string filepath, bool strict) {
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
    if (registers_.count(tt[0])) {
      std::array<float,2> pos = node_j.value().at("position");
      auto nhandle = create_node(registers_.at(tt[0]), tt[1], {pos[0], pos[1]});
      new_nodes.push_back(nhandle);
      std::string node_name = node_j.key();
      name_node(nhandle, node_name);
      if (node_j.value().count("parameters")) {
        auto params_j = node_j.value().at("parameters");
        for (auto& pel : params_j.items()) {
          if(!nhandle->parameters.count(pel.key())) {
            std::cerr << "key not found in node parameters: " << pel.key() << "\n";
            continue;
          }

          if (auto param_ptr = std::get_if<ParamBool>(&nhandle->parameters.at(pel.key())))
            param_ptr->set(pel.value().get<bool>());
          else if (auto param_ptr = std::get_if<ParamInt>(&nhandle->parameters.at(pel.key())))
            param_ptr->set(pel.value().get<int>());
          else if (auto param_ptr = std::get_if<ParamFloat>(&nhandle->parameters.at(pel.key())))
            param_ptr->set(pel.value().get<float>());
          else if (auto param_ptr = std::get_if<ParamDouble>(&nhandle->parameters.at(pel.key())))
            param_ptr->set(pel.value().get<double>());
          else if (auto param_ptr = std::get_if<ParamBoundedInt>(&nhandle->parameters.at(pel.key())))
            param_ptr->set(pel.value().get<int>());
          else if (auto param_ptr = std::get_if<ParamBoundedFloat>(&nhandle->parameters.at(pel.key())))
            param_ptr->set(pel.value().get<float>());
          else if (auto param_ptr = std::get_if<ParamBoundedDouble>(&nhandle->parameters.at(pel.key())))
            param_ptr->set(pel.value().get<double>());
          else if (auto param_ptr = std::get_if<ParamIntRange>(&nhandle->parameters.at(pel.key())))
            param_ptr->set(pel.value().get<std::pair<int,int>>());
          else if (auto param_ptr = std::get_if<ParamFloatRange>(&nhandle->parameters.at(pel.key())))
            param_ptr->set(pel.value().get<std::pair<float,float>>());
          else if (auto param_ptr = std::get_if<ParamPath>(&nhandle->parameters.at(pel.key())))
            param_ptr->set(pel.value().get<std::string>());
        }
        nhandle->post_parameter_load();
      }
    } else {
      std::cerr << "Could not load node of type " << tt[1] << ", register not found: " << tt[0] <<"\n";
      if (strict)
        throw gfException("Unable to load json file");
    }
  }
  for (auto node_j : nodes_j.items()) {
    auto tt = node_j.value().at("type").get<std::array<std::string,2>>();
    if (registers_.count(tt[0])) {
      auto nhandle = nodes[node_j.key()];
      if (node_j.value().count("connections")) {
        auto conns_j = node_j.value().at("connections");
        for (json::const_iterator conn_j = conns_j.begin(); conn_j!= conns_j.end(); ++conn_j) {
          for (json::const_iterator c=conn_j->begin(); c!=conn_j->end(); ++c) {
            auto cval = c.value().get<std::array<std::string,2>>();
            if (nodes.count(cval[0]))
              try {
                if (!nodes[cval[0]]->input_terminals.count(cval[1]))
                  throw gfException("No input terminal '" + cval[1] + "' on node '" + cval[0] + "', failed to connect.");
                nhandle->output_terminals[conn_j.key()]->connect(*nodes[cval[0]]->input_terminals[cval[1]]);
              } catch (const gfException& e) {
                if(strict) {
                  throw e;
                } else {
                  std::cerr << e.what() << "\n";
                }
              }
            else 
              std::cerr << "Could not connect output " << conn_j.key() << "\n";
          }
        }
      }
    }
  }
  return new_nodes;
}

bool geoflow::connect(gfOutputTerminal& oT, gfInputTerminal& iT) {
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
bool geoflow::connect(gfTerminal& t1, gfTerminal& t2) {
  auto& oT = static_cast<gfOutputTerminal&>(t1);
  auto& iT = static_cast<gfInputTerminal&>(t2);
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
bool geoflow::is_compatible(gfTerminal& t1, gfTerminal& t2) {
  auto& oT = static_cast<gfOutputTerminal&>(t1);
  auto& iT = static_cast<gfInputTerminal&>(t2);
  return oT.is_compatible(iT);
}
void geoflow::disconnect(gfTerminal& t1, gfTerminal& t2) {
  auto& oT = static_cast<gfOutputTerminal&>(t1);
  auto& iT = static_cast<gfInputTerminal&>(t2);
  oT.disconnect(iT);
}
// bool geoflow::detect_loop(gfTerminal& t1, gfTerminal& t2) {
//   gfOutputTerminal& oT;
//   gfInputTerminal& iT;
//   if (t1.get_side()==GF_IN && ) {
//     auto& oT = static_cast<gfOutputTerminal&>(t1);
//     auto& iT = static_cast<gfInputTerminal&>(t2);
//   } else
//   return detect_loop(oT, iT);
// }
bool geoflow::detect_loop(gfTerminal& outputT, gfTerminal& inputT) {
  auto node = inputT.get_parent().get_handle();
  std::queue<std::shared_ptr<Node>> nodes_to_check;
  std::set<std::shared_ptr<Node>> visited;
  nodes_to_check.push(node);
  bool loop_detected=false;
  while (!nodes_to_check.empty()) {
    auto n = nodes_to_check.front();
    nodes_to_check.pop();
    
    n->for_each_output([&outputT, &nodes_to_check, &visited, &loop_detected](gfOutputTerminal& oT) {
      if (&oT == &outputT){
        loop_detected=true;
        return;
      }
      for (auto& c :oT.get_connections()) {
        if (auto iT = c.lock()) {
          auto child_node = iT->get_parent().get_handle();
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
    for (auto& output_term : node->output_terminals) {
      for (auto& input_term : output_term.second->get_connections()) {
        auto iT = input_term.lock();
        auto oT = output_term.second;
        connections.push_back(std::make_tuple(
          node->get_name(), 
          iT->get_parent().get_name(),
          oT->get_name(),
          iT->get_name()
        ));
      }
    }
  }
  return connections;
}
