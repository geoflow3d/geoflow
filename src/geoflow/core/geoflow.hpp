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

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <functional>
#include <any>
#include <optional>
#include <variant>
#include <map>
#include <unordered_map>
#include <set>
#include <queue>
#include <exception>

#include <iostream>
#include <sstream>

#include "../common.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace geoflow {

  class Exception: public std::exception
  {
  public:
      explicit Exception(const std::string& message):
        msg_(message)
        {}
      virtual const char* what() const throw (){
        return msg_.c_str();
      }

  protected:
      std::string msg_;
  };

  class Node;
  class NodeManager;
  class InputTerminal;
  class OutputTerminal;
  // typedef std::weak_ptr<InputTerminal> InputHandle;
  // typedef std::weak_ptr<OutputTerminal> OutputHandle;

  typedef std::variant<bool,int,float,std::string> Parameter;
  typedef std::unordered_map<std::string, Parameter> ParameterMap;
  typedef std::shared_ptr<Node> NodeHandle;
  
  class Terminal {
    public:
    Terminal(Node& parent_gnode, std::string name):parent(parent_gnode), name(name) {
      // std::cout<< "Constructing geoflow::Terminal " << this << "\n";
    };
    ~Terminal() {
      // std::cout<< "Destructing geoflow::Terminal " << this << "\n";
    }

    Node& parent;
    const std::string name;
    std::any cdata;

    virtual void push(std::any data) = 0;
    virtual void clear() = 0;
    
    bool has_data() {
      return cdata.has_value();
    };
    template<typename T> T get() { 
      return std::any_cast<T>(cdata); 
    };
    template<typename T> void set(T data) { 
      push(std::any(data)); 
    };

    const std::string get_name() { return name; };
  };

  class InputTerminal : public Terminal, public std::enable_shared_from_this<InputTerminal>{
    std::vector<TerminalType> types;
    public:
    TerminalType connected_type;
    InputTerminal(Node& parent_gnode, std::string name, std::initializer_list<TerminalType> types): Terminal(parent_gnode, name), types(types) {};
    
    std::weak_ptr<InputTerminal>  get_ptr(){
      return weak_from_this();
    }
    std::vector<TerminalType> get_types() { return types; };
    void push(std::any data);
    void clear();
  };
  class OutputTerminal : public Terminal, public std::enable_shared_from_this<OutputTerminal>{
    typedef std::set<std::weak_ptr<InputTerminal>, std::owner_less<std::weak_ptr<InputTerminal>>> connection_set;
    TerminalType type;
    public:
    //use a set to make sure we don't get duplicate connection
    connection_set connections;
    
    OutputTerminal(Node& parent_gnode, std::string name, TerminalType type): Terminal(parent_gnode, name), type(type){};
    ~OutputTerminal();
    
    std::weak_ptr<OutputTerminal>  get_ptr(){
      return weak_from_this();
    }

    bool is_compatible(InputTerminal& in);
    void connect(InputTerminal& in);
    void disconnect(InputTerminal& in);
    void propagate();
    connection_set& get_connections();
    
    void push(std::any data);
    void clear();
  };

  template <typename TerminalClass> class TerminalGroup {
    protected:
    std::map<std::string,std::shared_ptr<TerminalClass> > terminals;
    std::vector<TerminalType> types;
    std::string name;
    Node& parent;
    public:
    TerminalGroup(Node& parent_gnode, std::string name, std::initializer_list<TerminalType> types)
    : parent(parent_gnode), name(name), types(types) {};

    TerminalClass& term(std::string term_name) {
      return *terminals.at(term_name);
    }
    void add(std::string term_name, TerminalType type) {
      // TODO: check if term_name is unique and if type is in types
      terminals[term_name] = std::make_shared<InputTerminal>(parent, name, type);
    }
    void clear(std::string term_name) {
      terminals.erase(term_name);
    }
    void clear() {
      terminals.clear();
    }
    friend class Node;
  };

  enum node_status {
    WAITING,
    READY,
    PROCESSING,
    DONE
  };

  class Node : public std::enable_shared_from_this<Node> {
    public:

    std::map<std::string,std::shared_ptr<InputTerminal>> inputTerminals;
    std::map<std::string,std::shared_ptr<OutputTerminal>> outputTerminals;

    std::map<std::string,TerminalGroup<InputTerminal>> inputGroups;
    std::map<std::string,TerminalGroup<OutputTerminal>> outputGroups;

    ParameterMap parameters;
    ImVec2 position;

    Node(NodeManager& manager, std::string type_name): manager(manager), type_name(type_name) {};
    ~Node() {
      // std::cout<< "Destructing geoflow::Node " << type_name << " " << name << "\n";
      notify_children();
    }

    InputTerminal& input(std::string term_name) {
      if (inputTerminals.find(term_name) == inputTerminals.end()) {
        throw Exception("No such input terminal - \""+term_name+"\" in " + get_name());
      }
      return *inputTerminals[term_name];
    }
    OutputTerminal& output(std::string term_name) {
      if (outputTerminals.find(term_name) == outputTerminals.end()) {
        throw Exception("No such output terminal - \""+term_name+"\" in " + get_name());
      }
      return *outputTerminals[term_name];
    }
    template<typename T> T& param(std::string name) {
      return std::get<T>(parameters.at(name));
    }
    TerminalGroup<InputTerminal>& input_group(std::string group_name){
      return inputGroups.at(group_name)
    }
    TerminalGroup<OutputTerminal>& output_group(std::string group_name){
      return outputGroups.at(group_name)
    }

    void for_each_input(std::function<void(InputTerminal&)> f) {
      for (auto& iT : inputTerminals) {
        f(*iT.second);
      }
      for (auto& iG : inputGroups) {
        for (auto& iT : iG.terminals) {
          f(*iT.second);
        }
      }
    }
    void for_each_output(std::function<void(OutputTerminal&)> f) {
      for (auto& iT : outputTerminals) {
        f(*iT.second);
      }
      for (auto& iG : outputGroups) {
        for (auto& iT : iG.terminals) {
          f(*iT.second);
        }
      }
    }

    node_status status=WAITING;

    void add_input(std::string name, TerminalType type);
    void add_input(std::string name, std::initializer_list<TerminalType> types);
    void add_output(std::string name, TerminalType type);

    void add_input_group(std::string group_name, std::initializer_list<TerminalType> types) {
      inputGroups[group_name] = TerminalGroup<InputTerminal>(*this, group_name, types);
    }
    void add_output_group(std::string group_name, std::initializer_list<TerminalType> types) {
      outputGroups[group_name] = TerminalGroup<OutputTerminal>(*this, group_name, types);
    }

    template<typename T> void add_param(std::string name, T value) {
      parameters[name] = value;
    }
    void set_param(std::string name, Parameter param);
    void set_params(ParameterMap param_map);
    const ParameterMap&  dump_params();

    void set_position(float x, float y) {
      position.x=x;
      position.y=y;
    }
    std::pair<float,float> get_position() {
      return std::make_pair(position.x, position.y);
    }

    NodeHandle get_handle(){return shared_from_this();};

    bool update();
    void propagate_outputs();
    void notify_children();

    // private:

    virtual void init() = 0;
    virtual void process() = 0;
    virtual void gui(){};
    virtual void on_push(InputTerminal& it){};
    virtual void on_clear(InputTerminal& it){};
    virtual void on_connect(OutputTerminal& ot){};

    std::string get_info();
    const std::string get_name() { return name; };
    const std::string get_type_name() { return type_name; };
    bool set_name(std::string new_name);

    protected:
    std::string name;
    const std::string type_name; // to be managed only by node manager because uniqueness constraint (among all nodes in the manager)
    NodeManager& manager;

    friend class NodeManager;
  };

  class NodeRegister {
    // Allows us to have a register of node types. Each node type is registered using a unique string (the type_name). The type_name can be used to create a node of the corresponding type with the create function.
    public:
    NodeRegister(std::string name):name(name) {};
    std::map<std::string, std::function<NodeHandle(NodeManager&, std::string)>> node_types;

    template<class NodeClass> void register_node(std::string type_name) {
      node_types[type_name] = create_node_type<NodeClass>;
    }
    std::string get_name() {return name;}
    protected:
    template<class NodeClass> static std::shared_ptr<NodeClass> create_node_type(NodeManager& nm, std::string type_name){
      auto node = std::make_shared<NodeClass>(nm, type_name);
      node->init();
      return node;
    }
    NodeHandle create(std::string type_name, NodeManager& nm) {
      if (node_types.find(type_name) == node_types.end())
        throw Exception("No such node type - \""+type_name+"\"");

      auto f = node_types[type_name];
      auto n = f(nm, type_name);
      return n;
    }
    const std::string name;
    friend class NodeManager;
  };

  class NodeManager {
    // manages a set of nodes that form one flowchart. Every node must linked to a NodeManager.
    typedef std::vector<std::tuple<std::string, std::string, std::string, std::string>> ConnectionList;
    public:
    size_t ID=0;

    std::optional<std::array<double,3>> data_offset;
    NodeManager(){};
    std::unordered_map<std::string, NodeHandle> nodes;

    NodeHandle create_node(NodeRegister& node_register, std::string type_name);
    NodeHandle create_node(NodeRegister& node_register, std::string type_name, std::pair<float,float> pos);
    void remove_node(NodeHandle node);

    bool name_node(NodeHandle node, std::string new_name);

    std::vector<NodeHandle> dump_nodes();

    ConnectionList dump_connections();

    // load_json() {

    // }
    
    bool run(Node &node);
    bool run(NodeHandle node) {
      return run(*node);
    };
    
    protected:
    std::queue<NodeHandle> node_queue;
    void queue(NodeHandle n);
    
    friend class Node;
  };

  bool connect(OutputTerminal& oT, InputTerminal& iT);
  bool connect(Node& n1, Node& n2, std::string s1, std::string s2);
  bool connect(NodeHandle n1, NodeHandle n2, std::string s1, std::string s2);
  bool connect(Terminal& t1, Terminal& t2);
  bool is_compatible(Terminal& t1, Terminal& t2);
  void disconnect(Terminal& t1, Terminal& t2);
  bool detect_loop(Terminal& t1, Terminal& t2);
  bool detect_loop(OutputTerminal& iT, InputTerminal& oT);
}
