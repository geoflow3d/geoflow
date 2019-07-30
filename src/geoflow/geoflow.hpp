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
#include <unordered_set>
#include <set>
#include <queue>
#include <typeinfo>
#include <typeindex>

#include <iostream>
#include <sstream>

#include "common.hpp"
#include "parameters.hpp"

namespace geoflow {

  class Node;
  class NodeManager;
  class NodeRegister;
  typedef std::shared_ptr<NodeRegister> NodeRegisterHandle;
  class InputTerminal;
  class OutputTerminal;
  class InputGroup;
  class OutputGroup;
  // typedef std::weak_ptr<InputTerminal> InputHandle;
  // typedef std::weak_ptr<OutputTerminal> OutputHandle;
  
  typedef std::shared_ptr<Node> NodeHandle;
  
  class gfOutputTerminal;
  class gfSingleOutputTerminal;

  enum gfIO {GF_IN, GF_OUT};
  enum gfTerminalFamily {GF_SINGLE, GF_MULTI, GF_GROUP};
  enum gfNodeStatus {GF_NODE_WAITING, GF_NODE_READY, GF_NODE_PROCESSING, GF_NODE_DONE};

  class gfTerminal : public gfObject {
    protected:
    Node& parent_;
    std::vector<std::type_index> types_;

    public:
    gfTerminal(Node& parent_node, std::initializer_list<std::type_index> types, std::string name)
      : parent_(parent_node), types_(types), gfObject(name) {};

    Node& get_parent() { return parent_; };

    bool accepts_type(std::type_index type);
    const std::vector<std::type_index>& get_types() { return types_; };
    const virtual gfIO get_side() = 0;
    virtual bool has_data() = 0;

    friend class Node;
  };

  class gfInputTerminal : public gfTerminal, public std::enable_shared_from_this<gfInputTerminal> {
    private:
    bool is_optional_;

    protected:
    std::weak_ptr<gfOutputTerminal> connected_output_;
    std::weak_ptr<gfInputTerminal>  get_ptr(){
      return weak_from_this();
    }
    void clear();
    void update_on_receive(bool queue);

    public:
    gfInputTerminal(Node& parent_gnode, std::string name, std::initializer_list<std::type_index> types, bool is_optional=false)
      : gfTerminal(parent_gnode, types, name), is_optional_(is_optional)
      {};
    gfInputTerminal(Node& parent_gnode, std::string name, std::type_index type, bool is_optional=false)
      : gfInputTerminal(parent_gnode, name, {type}, is_optional)
      {};
    const gfIO get_side() { return GF_IN; };
    const virtual gfTerminalFamily get_family() = 0;
    bool is_optional() { return is_optional_; };
    bool connected_type(std::type_index ttype);

    friend class gfOutputTerminal;
    friend class gfSingleOutputTerminal;
    friend class Node;
  };

  class gfSingleInputTerminal : public gfInputTerminal {
    private:
    std::type_index connected_type_ = typeid(void);

    public:
    using gfInputTerminal::gfInputTerminal;
    const gfTerminalFamily get_family() { return GF_SINGLE; };
    std::type_index get_connected_type() { return connected_type_; };
    template<typename T> T get();
    bool has_data();
  };

  typedef std::set<std::weak_ptr<gfInputTerminal>, std::owner_less<std::weak_ptr<gfInputTerminal>>> InputConnectionSet;

  class gfOutputTerminal : public gfTerminal, public std::enable_shared_from_this<gfOutputTerminal> {
    protected:
    std::weak_ptr<gfOutputTerminal>  get_ptr(){
      return weak_from_this();
    }
    InputConnectionSet connections_;

    virtual void propagate();
    virtual std::set<NodeHandle> get_child_nodes();
    virtual void clear() = 0;

    public:
    gfOutputTerminal(Node& parent_gnode, std::string name, std::type_index type) 
      : gfTerminal(parent_gnode, {type}, name)
    {}
    ~gfOutputTerminal();
    virtual bool is_compatible(gfInputTerminal& input_terminal);
    const InputConnectionSet& get_connections();
    const std::type_index& get_type() { return types_[0]; };
    const gfIO get_side() { return GF_OUT; };
    const virtual gfTerminalFamily get_family() = 0;
    void connect(gfInputTerminal& in);
    void disconnect(gfInputTerminal& in);

    friend class Node;
    friend class gfInputTerminal;
  };

  class gfSingleOutputTerminal : public gfOutputTerminal {
    private:
    std::any data_;
    protected:
    void clear();
    std::any& get_data() { return data_; };

    public:
    using gfOutputTerminal::gfOutputTerminal;
    const gfTerminalFamily get_family() { return GF_SINGLE; };
    void disconnect();
    bool has_data();
    template<typename T> void set(T data){
      data_ = std::move(data);
    };
    template<typename T> T get() { return std::any_cast<T>(data_); };

    friend class gfSingleInputTerminal;
  };

  template<typename T> T gfSingleInputTerminal::get() {
    auto output_term = connected_output_.lock();
    auto sot = (gfSingleOutputTerminal*)(output_term.get());
    return std::any_cast<T>(sot->get_data()); 
  };

  class Node : public std::enable_shared_from_this<Node> {
    public:

    std::map<std::string,std::shared_ptr<gfInputTerminal>> input_terminals;
    std::map<std::string,std::shared_ptr<gfOutputTerminal>> output_terminals;

    // std::map<std::string,std::shared_ptr<InputGroup>> inputGroups;
    // std::map<std::string,std::shared_ptr<OutputGroup>> outputGroups;

    ParameterMap parameters;
    arr2f position;

    Node(NodeRegisterHandle node_register, NodeManager& manager, std::string type_name): node_register(node_register), manager(manager), type_name(type_name) {};
    ~Node();

    void remove_from_manager();

    gfSingleInputTerminal& input(std::string term_name) {
      if (input_terminals.find(term_name) == input_terminals.end()) {
        throw gfException("No such input terminal - \""+term_name+"\" in " + get_name());
      }
      auto input_term = (gfSingleInputTerminal*) (input_terminals[term_name].get());
      return *input_term;
    }
    gfSingleOutputTerminal& output(std::string term_name) {
      if (output_terminals.find(term_name) == output_terminals.end()) {
        throw gfException("No such output terminal - \""+term_name+"\" in " + get_name());
      }
      auto output_term = (gfSingleOutputTerminal*) (output_terminals[term_name].get());
      return *output_term;
    }
    // template<typename T> T& param(std::string name) {
    //   return std::get<T>(parameters.at(name));
    // }
    // InputGroup& input_group(std::string group_name){
    //   return *inputGroups.at(group_name);
    // }
    // OutputGroup& output_group(std::string group_name){
    //   return *outputGroups.at(group_name);
    // }

    void for_each_input(std::function<void(gfInputTerminal&)> f) {
      for (auto& iT : input_terminals) {
        f(*iT.second);
      }
      // for (auto& iG : inputGroups) {
      //   for (auto& iT : iG.second->terminals) {
      //     f(*iT.second);
      //   }
      // }
    }
    void for_each_output(std::function<void(gfOutputTerminal&)> f) {
      for (auto& iT : output_terminals) {
        f(*iT.second);
      }
      // for (auto& iG : outputGroups) {
      //   for (auto& iT : iG.second->terminals) {
      //     f(*iT.second);
      //   }
      // }
    }

    gfNodeStatus status_=GF_NODE_WAITING;

    void add_input(std::string name, std::type_index type);
    void add_input(std::string name, std::initializer_list<std::type_index> types);
    void add_output(std::string name, std::type_index type);

    // void add_input_group(std::string group_name, std::initializer_list<std::type_index> types) {
    //   inputGroups.emplace(
    //     std::make_pair(group_name, std::make_shared<InputGroup>(*this, group_name, types))
    //   );
    // }
    // void add_output_group(std::string group_name, std::initializer_list<std::type_index> types) {
    //   outputGroups.emplace(
    //     std::make_pair(group_name, std::make_shared<OutputGroup>(*this, group_name, types))
    //   );
    // }

    template<typename T> void add_param(std::string name, T value) {
      parameters.emplace(name, value);
    }
    void set_param(std::string name, ParameterVariant param, bool quiet=false);
    void set_params(ParameterMap param_map, bool quiet=false);
    const ParameterMap&  dump_params();

    void set_position(float x, float y) {
      position[0]=x;
      position[1]=y;
    }
    std::pair<float,float> get_position() {
      return std::make_pair(position[0], position[1]);
    }

    NodeHandle get_handle(){return shared_from_this();};

    bool queue();
    bool update_status();
    void propagate_outputs();
    void notify_children();
    // void preprocess();

    // private:

    virtual void init() = 0;
    // virtual std::map<std::string,std::shared_ptr<InputTerminal>> init_inputs() {};
    // virtual std::map<std::string,std::shared_ptr<OutputTerminal>> init_outputs() {};
    // virtual ParameterMap init_parameters() {};
    virtual void process() = 0;
    virtual void gui() {};
    virtual void on_receive(gfInputTerminal& it){};
    virtual void on_clear(InputTerminal& it){};
    virtual void on_waiting(gfInputTerminal& it){};
    virtual void on_connect_input(InputTerminal& ot){};
    virtual void on_connect_output(gfOutputTerminal& ot){};
    virtual void on_change_parameter(std::string name, ParameterVariant& param){};
    virtual void before_gui(){};
    virtual std::string info() {return std::string();};

    std::string debug_info();
    const std::string get_name() { return name; };
    const std::string get_type_name() { return type_name; };
    const NodeRegister& get_register() { return *node_register; };
    bool set_name(std::string new_name);

    protected:
    std::string name;
    const std::string type_name; // to be managed only by node manager because uniqueness constraint (among all nodes in the manager)
    NodeManager& manager;
    NodeRegisterHandle node_register;

    friend class NodeManager;
  };

  class NodeRegister : public std::enable_shared_from_this<NodeRegister> {
    // Allows us to have a register of node types. Each node type is registered using a unique string (the type_name). The type_name can be used to create a node of the corresponding type with the create function.
    // private:
    public:
    NodeRegister(const std::string& name) : name(name) {};
    NodeRegister();

    template<typename ... T> static NodeRegisterHandle create(T&& ... t) {
      return std::shared_ptr<NodeRegister>(new NodeRegister(std::forward<T>(t)...));
    }

    std::map<std::string, std::function<NodeHandle(NodeRegisterHandle, NodeManager&, std::string)>> node_types;

    template<class NodeClass> void register_node(std::string type_name) {
      node_types[type_name] = create_node_type<NodeClass>;
    }
    std::string get_name() const {return name;}
    protected:
    template<class NodeClass> static std::shared_ptr<NodeClass> create_node_type(NodeRegisterHandle nr, NodeManager& nm, std::string type_name){
      auto node = std::make_shared<NodeClass>(nr, nm, type_name);
      node->init();
      return node;
    }
    NodeHandle create(std::string type_name, NodeManager& nm) {
      if (node_types.find(type_name) == node_types.end())
        throw gfException("No such node type - \""+type_name+"\"");

      auto f = node_types[type_name];
      auto n = f(shared_from_this(), nm, type_name);
      return n;
    }
    std::string name;
    friend class NodeManager;
  };
  typedef std::unordered_map<std::string, NodeRegisterHandle> NodeRegisterMap_;
  class NodeRegisterMap : public NodeRegisterMap_ {
    private:
    using NodeRegisterMap_::emplace;
    public:;
    using NodeRegisterMap_::NodeRegisterMap_;
    NodeRegisterMap(std::initializer_list<NodeRegisterHandle> registers) {
      for (auto& r : registers) {
        emplace(r);
      }
    }
    std::pair<NodeRegisterMap_::iterator,bool> emplace(NodeRegisterHandle reg) {
      return emplace(reg->get_name(), reg);
    }
  };

  class NodeManager {
    // manages a set of nodes that form one flowchart. Every node must linked to a NodeManager.
    public:
    size_t ID=0;

    std::optional<std::array<double,3>> data_offset;
    NodeManager(){};
    std::unordered_map<std::string, NodeHandle> nodes;

    NodeHandle create_node(NodeRegister& node_register, std::string type_name);
    NodeHandle create_node(NodeRegisterHandle node_register, std::string type_name);
    NodeHandle create_node(NodeRegister& node_register, std::string type_name, std::pair<float,float> pos);
    NodeHandle create_node(NodeRegisterHandle node_register, std::string type_name, std::pair<float,float> pos);
    void remove_node(NodeHandle node);
    void clear();

    bool name_node(NodeHandle node, std::string new_name);

    std::vector<NodeHandle> dump_nodes();


    std::vector<NodeHandle> load_json(std::string filepath, NodeRegisterMap& registers);
    void dump_json(std::string filepath);

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

  typedef std::vector<std::tuple<std::string, std::string, std::string, std::string>> ConnectionList;
  ConnectionList dump_connections(std::vector<NodeHandle>);
  bool connect(gfOutputTerminal& oT, gfInputTerminal& iT);
  bool connect(Node& n1, Node& n2, std::string s1, std::string s2);
  bool connect(NodeHandle n1, NodeHandle n2, std::string s1, std::string s2);
  bool connect(gfTerminal& t1, gfTerminal& t2);
  bool connect(OutputGroup& in, InputGroup& out);
  bool is_compatible(gfTerminal& t1, gfTerminal& t2);
  void disconnect(gfTerminal& t1, gfTerminal& t2);
  bool detect_loop(gfTerminal& t1, gfTerminal& t2);

  bool detect_loop(gfOutputTerminal& iT, gfInputTerminal& oT);
}
