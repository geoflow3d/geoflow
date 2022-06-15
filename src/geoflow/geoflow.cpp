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
#include <chrono>
#include <ctime>

#include "geoflow.hpp"

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

std::string gfTerminal::get_full_name() const {
  return parent_.get_name() + "." + get_name();
}

void gfInputTerminal::clear() {
  parent_.update_status();
  parent_.on_clear(*this);
}

gfSingleFeatureInputTerminal::~gfSingleFeatureInputTerminal(){
  if (auto connected_output = connected_output_.lock())
    connected_output->connections_.erase(get_ptr());
}
bool gfSingleFeatureInputTerminal::is_connected_type(std::type_index ttype) const {
  if(auto output_term = connected_output_.lock()) {
    return output_term->accepts_type(ttype);
  }
  return false;
}
std::type_index gfSingleFeatureInputTerminal::get_connected_type() const {
  if(auto output_term = connected_output_.lock()) {
    return output_term->types_[0];
  }
  return typeid(void);
}
void gfSingleFeatureInputTerminal::update_on_receive(bool queue) {
  if(has_data() || is_touched()) {
    if (queue && parent_.update_status() && parent_.autorun) 
      parent_.queue();
    parent_.on_receive(*this);
  }
}
bool gfSingleFeatureInputTerminal::has_connection() {
  return !connected_output_.expired();
}
bool gfSingleFeatureInputTerminal::has_data() const {
  if (auto output_term = connected_output_.lock()) {
    return output_term->has_data();
  }
  return false;
}
bool gfSingleFeatureInputTerminal::is_touched() {
  if (auto output_term = connected_output_.lock()) {
    return output_term->is_touched();
  }
  return false;
}
void gfSingleFeatureInputTerminal::connect_output(gfOutputTerminal& output_term) {
  //check if we are already connected and if so disconnect from that output term first 
  if(auto output = connected_output_.lock()) {
    output->disconnect(*this);
  }
  connected_output_ = output_term.get_ptr();
}
void gfSingleFeatureInputTerminal::disconnect_output(gfOutputTerminal& output_term) {
  connected_output_.reset();
}
const std::vector<std::any>& gfSingleFeatureInputTerminal::get_data_vec() const {
  auto output_term = connected_output_.lock();
  auto sot = (gfSingleFeatureOutputTerminal*)(output_term.get());
  return sot->get_data_vec();
}
size_t gfSingleFeatureInputTerminal::size() const {
  auto output_term = connected_output_.lock();
  auto sot = (gfSingleFeatureOutputTerminal*)(output_term.get());
  return sot->size(); 
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
  // Everything can  be connected to a Multi Feature input, otherwise the families need to be equal
  bool family_compatible = (get_family()==input_terminal.get_family()) || (input_terminal.get_family() == GF_MULTI_FEATURE);
  return family_compatible && type_compatible;
}
const InputConnectionSet& gfOutputTerminal::get_connections(){
  return connections_;
}
void gfOutputTerminal::propagate() {
  for (auto& conn : get_connections()) {
    if (has_data() || is_touched())
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
    throw gfNodeTerminalError("Failed to connect output " +get_name()+ " from "+parent_.get_name()+" to input " + in.get_name() + " from " +in.parent_.get_name()+ ". Terminals have incompatible types!");
    
  if (detect_loop(*this, in))
    throw gfNodeTerminalError("Failed to connect output " +get_name()+ " from "+parent_.get_name()+" to input " + in.get_name() + " from " +in.parent_.get_name()+ ". Loop detected!");

  in.connect_output(*this);
  connections_.insert(in.get_ptr());
  parent_.on_connect_output(*this);
  in.get_parent().on_connect_input(in);
  if (has_data() || is_touched()) {
    in.update_on_receive(false);
  }
};
void gfOutputTerminal::disconnect(gfInputTerminal& in) {
  connections_.erase(in.get_ptr());
  in.disconnect_output(*this);
  in.clear();
  in.parent_.notify_children();
};

// void gfSingleFeatureOutputTerminal::clear() {
//   data_.reset();
// }
// bool gfSingleFeatureOutputTerminal::has_data() {
//   return data_.has_value();
// }
void gfSingleFeatureOutputTerminal::clear() {
  data_.clear();
  is_touched_ = false;
}
bool gfSingleFeatureOutputTerminal::has_data() const {
  return data_.size()!=0;
}

gfMultiFeatureInputTerminal::~gfMultiFeatureInputTerminal(){
  for (auto output_term_ : connected_outputs_) {
    if(auto output_term = output_term_.lock())
      output_term->connections_.erase(get_ptr());
  }
}
void gfMultiFeatureInputTerminal::clear() {
  rebuild_terminal_refs();
  gfInputTerminal::clear();
}
void gfMultiFeatureInputTerminal::connect_output(gfOutputTerminal& output_term) {
  connected_outputs_.insert(output_term.get_ptr());
}
void gfMultiFeatureInputTerminal::disconnect_output(gfOutputTerminal& output_term) {
  connected_outputs_.erase(output_term.get_ptr());
}
void gfMultiFeatureInputTerminal::push_term_ref(gfOutputTerminal* term_ptr) {
  auto oterm_ptr = static_cast<gfSingleFeatureOutputTerminal*>(term_ptr);
  sub_terminals_.push_back(oterm_ptr);
  // if (auto oterm_ptr = dynamic_cast<gfSingleFeatureOutputTerminal*>(term_ptr)) {
  //   sub_terminals_.push_back(oterm_ptr);
  // }
  // else if (auto vector_term_ptr = dynamic_cast<gfSingleFeatureOutputTerminal*>(term_ptr)) {
  //   vector_terminals_.push_back(vector_term_ptr);
  // }
}
void gfMultiFeatureInputTerminal::rebuild_terminal_refs() {
  sub_terminals_.clear();
  // vector_terminals_.clear();
  for (auto& wptr : connected_outputs_) {
    if (auto term = wptr.lock()) {
      auto term_ptr = term.get();
      if (auto poly_term_ptr = dynamic_cast<gfMultiFeatureOutputTerminal*>(term_ptr)) {
        for (auto& [name, sub_term] : poly_term_ptr->sub_terminals()) {
          push_term_ref(sub_term.get());
        }
      } else {
        push_term_ref(term_ptr);
      }
    }
  }
}
void gfMultiFeatureInputTerminal::update_on_receive(bool queue) {
  rebuild_terminal_refs();
  if (parent_.update_status()) {
    if (queue && parent_.autorun)
      parent_.queue();
  }
  parent_.on_receive(*this);
}
bool gfMultiFeatureInputTerminal::has_data() const {
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
bool gfMultiFeatureInputTerminal::is_touched() {
  for (auto output_term_ : connected_outputs_){
    if (auto output_term = output_term_.lock()) {
      if (!output_term->is_touched()) {
        return true;
      }
    }
  }
  return false;
}
size_t gfMultiFeatureInputTerminal::size() const{
  if (connected_outputs_.size()==0)
    return 0;
  else
    return connected_outputs_.begin()->lock()->size();
}

void gfMultiFeatureOutputTerminal::clear() {
  // for (auto& [name, t] : terminals_) {
  //   t->clear();
  // }
  terminals_.clear();
  is_touched_ = false;
}
bool gfMultiFeatureOutputTerminal::has_data() const {
  if(terminals_.size()==0) {
    return false;
  }
  for (auto& [name, t] : terminals_) {
    if(!t->has_data())
      return false;
  }
  return true;
}
size_t gfMultiFeatureOutputTerminal::size() const {
  if (terminals_.size()==0)
    return 0;
  else
    return terminals_.begin()->second->size();
}

gfSingleFeatureOutputTerminal& gfMultiFeatureOutputTerminal::add(std::string term_name, std::type_index ttype ) {
  return add<gfSingleFeatureOutputTerminal>(term_name, {ttype}, false);
};
gfSingleFeatureOutputTerminal& gfMultiFeatureOutputTerminal::add_vector(std::string term_name, std::type_index ttype ) {
  return add<gfSingleFeatureOutputTerminal>(term_name, {ttype}, true);
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
// void Node::set_param(std::string name, Parameter param, bool quiet) {
//   if (parameters.find(name) != parameters.end()) {
//     if(parameters.at(name).index() == param.index()) {
//       parameters.emplace(name, param);
//     } else {
//       std::cout << "Incorrect datatype for parameter: '" << name <<"', node type: " << type_name << "\n";
//     }
//   } else if(!quiet) {
//     std::cout << "No such parameter in this node: '" << name <<"', node type: " << type_name << "\n";
//   }
// }
// void Node::set_params(ParameterMap new_map, bool quiet) {
//   for (auto& kv : new_map) {
//     set_param(kv.first, kv.second, quiet);
//   }
// }
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
bool Node::inputs_valid() {
  for (auto& [name,iT] : input_terminals) {
    if (!iT->has_data())
      return false;
  }
  return true;
}
bool Node::parameters_valid() {
  return true;
}
bool Node::update_status() {
  auto status_before = status_;
  if (inputs_valid())
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

std::string Node::substitute_from_term(const std::string& textt, gfMultiFeatureInputTerminal& term, const size_t& i) {
  size_t start_pos=0;
  std::string text(textt);
  while(true) {
    auto open = text.find("[[", start_pos);
    if (open==std::string::npos) break;

    auto close = text.find("]]", start_pos);
    if (close==std::string::npos) break;
    open+=2;
    auto len = close-open;
    std::string attr_name = text.substr(open, len);
    for(auto& sterm : term.sub_terminals()) {
      if (sterm->get_name() == attr_name) {
        if(sterm->accepts_type(typeid(std::string))) {
          auto& val = sterm->get<std::string>(i);
          text.replace(open-2, len+4, val);
        } else if(sterm->accepts_type(typeid(int))) {
          auto val = sterm->get<int>(i);
          text.replace(open-2, len+4, std::to_string(val));
        } else if(sterm->accepts_type(typeid(float))) {
          auto val = sterm->get<float>(i);
          text.replace(open-2, len+4, std::to_string(val));
        } else if(sterm->accepts_type(typeid(bool))) {
          auto val = sterm->get<bool>(i);
          if (val)
            text.replace(open-2, len+4, "true");
          else
            text.replace(open-2, len+4, "false");
        } else {
          // throw warning that subtitute param is not found
          start_pos = close;
        }
      }  else {
        // throw warning that subtitute param is not subtituted
        start_pos = close;
      }
    }
  }
  return text;
}

void NodeManager::queue(std::shared_ptr<Node> n) {
  node_queue.push(n);
}
size_t NodeManager::run_all(bool notify_children) {
  // disable autorun on nodes that do not have valid parameters
  for (auto& [nname, node] : nodes) {
    for (auto& [name, param] : node->parameters) {
        param->copy_value_from_master();
    }
    if(!node->parameters_valid()) {
      node->autorun = false;
      std::cout << "Not executing " << nname << std::endl;
    }
  }
  // find all root nodes with autorun enabled
  std::vector<NodeHandle> to_run;
  for (auto& [name, node] : nodes) {
    if(node->is_root() && node->autorun) {
      to_run.push_back(node);
    }
  }
  if(notify_children) {
    for (auto& node : to_run){
      node->notify_children();
    }
  }
  size_t run_count = 0;
  for (auto& node : to_run){
    run_count += run(node, notify_children);
  }
  return run_count;
}
size_t NodeManager::run(Node &node, bool notify_children) {
  std::queue<std::shared_ptr<Node>>().swap(node_queue); // clear to prevent double processing of nodes ()
  node.update_status();
  size_t run_count = 0;
  if (node.queue()) {
    if (notify_children) node.notify_children();
    while (!node_queue.empty()) {
      auto n = node_queue.front();
      node_queue.pop();
      n->status_ = GF_NODE_PROCESSING;
      // n->preprocess();
      std::cout << "P " << n->get_name() << "..." << std::flush;
      std::clock_t c_start = std::clock(); // CPU time
      // copy parameter values from master if a master is set
      for (auto& [name, param] : n->parameters) {
        param->copy_value_from_master();
      }
//      try {
        n->process();
        n->status_ = GF_NODE_DONE;
        ++run_count;
        n->propagate_outputs();
//      } catch (const gfException& e) {
//        std::cout << "ERROR: gfException -- " << e.what() << "\n" << std::flush;
//        n->status_ = GF_NODE_READY;
//      }
      std::clock_t c_end = std::clock(); // CPU time
      std::cout << 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC << "ms\n";
    }
  }
  return run_count;
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
  global_flowchart_params.clear();
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

void NodeManager::set_globals(const NodeManager& other_manager) {
  for (auto& [name, param] : other_manager.global_flowchart_params) {
    global_flowchart_params[name] = param;
  }
}

void NodeManager::json_serialise(std::ostream& json_sstream) {
  json j;
  j["globals"] = json::object();
  for (auto& [name, param] : global_flowchart_params) {
    if(param->is_type(typeid(int))) {
      j["globals"][name] = {param->get_help(), std::string("int"), param->as_json()};
    } else if(param->is_type(typeid(float))) {
      j["globals"][name] = {param->get_help(), std::string("float"), param->as_json()};
    } else if(param->is_type(typeid(bool))) {
      j["globals"][name] = {param->get_help(), std::string("bool"), param->as_json()};
    } else if(param->is_type(typeid(std::string))) {
      j["globals"][name] = {param->get_help(), std::string("str"), param->as_json()};
    }
  }
  j["nodes"] = json::object();
  for (auto& [name, node_handle] : nodes) {
    json n;
    n["type"] = {node_handle->node_register->get_name(), node_handle->get_type_name()};
    n["position"] = {node_handle->position[0], node_handle->position[1]};
    for ( auto& [pname, pvalue] : node_handle->parameters ) {
      if (pvalue->has_master())
        n["parameters"][pname] = std::string("{{" + pvalue->get_master().lock()->get_label() + "}}");
      else
        n["parameters"][pname] = pvalue->as_json();
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
    //note marked terminals
    std::vector<std::pair<std::string, bool>> marked_inputs_vec;
    for (const auto& [name, iTerm] : node_handle->input_terminals) {
      n["marked_inputs"][name] = iTerm->is_marked();
    }
    for (const auto& [name, oTerm] : node_handle->output_terminals) {
      n["marked_outputs"][name] = oTerm->is_marked();
    }
    
    j["nodes"][name] = n;
  }
  json_sstream << std::setw(2) << j << std::endl;
}
void NodeManager::dump_json(std::string filepath) {
  std::ofstream ofs(filepath);
  json_serialise(ofs);
}
std::vector<NodeHandle> NodeManager::json_unserialise(std::istream& json_sstream, bool strict) {
  json j;
  std::vector<NodeHandle> new_nodes;
  if (json_sstream.peek() == std::ifstream::traits_type::eof()) {
    std::cerr << "bad json stream\n";
    return new_nodes;
  }
  json_sstream >> j;
  for (auto& [gname, val] : j["globals"].items()) {
    // do not create globals that already exist
    if(global_flowchart_params.find(gname)!=global_flowchart_params.end())
      continue;
    try {
      // case that no help text is provided (keep compatibility with old flowchart files)
      if (val.size() == 2) {
        auto global_type = val[0].get<std::string>(); 
        auto& global_val = val[1]; 
        if(global_type=="str") {
          global_flowchart_params[gname] = std::make_shared<ParameterByValue<std::string>>(global_val.get<std::string>(), gname, "");
        } else if(global_type=="bool") {
          global_flowchart_params[gname] = std::make_shared<ParameterByValue<bool>>(global_val.get<bool>(), gname, "");
        } else if(global_type=="int") {
          global_flowchart_params[gname] = std::make_shared<ParameterByValue<int>>(global_val.get<int>(), gname, "");
        } else if(global_type=="float") {
          global_flowchart_params[gname] = std::make_shared<ParameterByValue<float>>(global_val.get<float>(), gname, "");
        }
      // case that help text is available as it should be
      } else if (val.size() == 3) {
        auto help_text = val[0].get<std::string>(); 
        auto global_type = val[1].get<std::string>(); 
        auto& global_val = val[2]; 
        if(global_type=="str") {
          global_flowchart_params[gname] = std::make_shared<ParameterByValue<std::string>>(global_val.get<std::string>(), gname, help_text);
        } else if(global_type=="bool") {
          global_flowchart_params[gname] = std::make_shared<ParameterByValue<bool>>(global_val.get<bool>(), gname, help_text);
        } else if(global_type=="int") {
          global_flowchart_params[gname] = std::make_shared<ParameterByValue<int>>(global_val.get<int>(), gname, help_text);
        } else if(global_type=="float") {
          global_flowchart_params[gname] = std::make_shared<ParameterByValue<float>>(global_val.get<float>(), gname, help_text);
        }
      }
    } catch (const std::exception& e) {
      throw(gfFlowchartError("Unable to read global " + std::string(gname)));
    }
  }
  json nodes_j = j["nodes"];
  for (auto node_j : nodes_j.items()) {
    auto tt = node_j.value().at("type").get<std::array<std::string,2>>();
    if (registers_.count(tt[0])) {
      // construct node
      std::array<float,2> pos = node_j.value().at("position");
      auto nhandle = create_node(registers_.at(tt[0]), tt[1], {pos[0], pos[1]});
      new_nodes.push_back(nhandle);
      std::string node_name = node_j.key();
      name_node(nhandle, node_name);

      // set node parameters
      if (node_j.value().count("parameters")) {
        auto params_j = node_j.value().at("parameters");
        for (auto& pel : params_j.items()) {
          if(!nhandle->parameters.count(pel.key())) {
            std::cerr << "key not found in node parameters: " << pel.key() << "\n";
            continue;
          }
          auto phandle = nhandle->parameters[pel.key()];
          // check and set master parameter 
          if (pel.value().is_string() && !phandle->is_type(typeid(std::string)) ) {
            try{
              auto mgname = get_global_name( pel.value().get<std::string>() );
              phandle->set_master(global_flowchart_params[mgname]);
            } catch (const std::exception& e) {
              std::cerr << e.what();
            }
          } else phandle->from_json(pel.value());
        }
      }
      nhandle->post_parameter_load();
      // set marked terminals
      try{
        if (node_j.value().count("marked_inputs")) {
          auto marked_iterms_j = node_j.value().at("marked_inputs");
          for (auto& it : marked_iterms_j.items()) {
            nhandle->input_terminals.at(it.key())->set_marked(it.value().get<bool>());
          }
        }
        // set marked terminals
        if (node_j.value().count("marked_outputs")) {
          auto marked_oterms_j = node_j.value().at("marked_outputs");
          for (auto& it : marked_oterms_j.items()) {
            nhandle->output_terminals.at(it.key())->set_marked(it.value().get<bool>());
          }
        }
      } catch (const std::out_of_range& oor) {
        std::cerr << "could not find one marked terminal\n";
      }
    } else {
      std::cerr << "Could not load node of type " << tt[1] << ", register not found: " << tt[0] <<"\n";
      if (strict)
        throw gfFlowchartError("Unable to load json file");
    }
  }
  // create connections
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
                  throw gfNodeTerminalError("No input terminal '" + cval[1] + "' on node '" + cval[0] + "', failed to connect.");
                nhandle->output_terminals.at(conn_j.key())->connect(*nodes.at(cval[0])->input_terminals[cval[1]]);
              } catch (const std::exception& e) {
                if(strict) {
                  throw;
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
std::vector<NodeHandle> NodeManager::load_json(std::string filepath, bool strict) {
  std::ifstream ifs(filepath);
  return json_unserialise(ifs, strict);
}


std::string NodeManager::substitute_globals(const std::string& textt) const {
  size_t start_pos=0;
  std::string text(textt);
  while(true) {
    auto open = text.find("{{", start_pos);
    if (open==std::string::npos) break;

    auto close = text.find("}}", start_pos);
    if (close==std::string::npos) break;
    open+=2;
    auto len = close-open;
    std::string global_name = text.substr(open, len);
    if (global_flowchart_params.find(global_name) != global_flowchart_params.end()) {
      if(global_flowchart_params.at(global_name)->is_type(typeid(std::string))) {
        auto* global_val = static_cast<ParameterByValue<std::string>*>(global_flowchart_params.at(global_name).get());
        text.replace(open-2, len+4, global_val->get());
      } else if(global_flowchart_params.at(global_name)->is_type(typeid(int))) {
        auto* global_val = static_cast<ParameterByValue<int>*>(global_flowchart_params.at(global_name).get());
        text.replace(open-2, len+4, std::to_string(global_val->get()));
      } else if(global_flowchart_params.at(global_name)->is_type(typeid(float))) {
        auto* global_val = static_cast<ParameterByValue<float>*>(global_flowchart_params.at(global_name).get());
        text.replace(open-2, len+4, std::to_string(global_val->get()));
      } else if(global_flowchart_params.at(global_name)->is_type(typeid(bool))) {
        auto* global_val = static_cast<ParameterByValue<bool>*>(global_flowchart_params.at(global_name).get());
        if (global_val->get())
          text.replace(open-2, len+4, "true");
        else
          text.replace(open-2, len+4, "false");
      } else {
        // throw warning that subtitute param is not found
        start_pos = close;
      }
    }  else {
      // throw warning that subtitute param is not subtituted
      start_pos = close;
    }
  }
  return text;
}

std::string geoflow::get_global_name(const std::string& text) {
  auto open = text.find("{{", 0);
  if (open==std::string::npos) throw gfFlowchartError("Can not retrive global name");

  auto close = text.find("}}", 0);
  if (close==std::string::npos) throw gfFlowchartError("Can not retrive global name");
  open+=2;
  auto len = close-open;
  return text.substr(open, len);
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
