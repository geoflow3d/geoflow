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
  // typedef std::weak_ptr<InputTerminal> InputHandle;
  // typedef std::weak_ptr<OutputTerminal> OutputHandle;
  
  typedef std::shared_ptr<Node> NodeHandle;
  typedef std::weak_ptr<Node> WeakNodeHandle;
  
  class gfOutputTerminal;
  class gfBasicMonoOutputTerminal;

  enum gfIO {GF_IN, GF_OUT};
  enum gfTerminalFamily {GF_UNKNOWN, GF_BASIC, GF_VECTOR, GF_POLY};
  enum gfNodeStatus {GF_NODE_WAITING, GF_NODE_READY, GF_NODE_PROCESSING, GF_NODE_DONE};

  class gfTerminal : public gfObject {
    protected:
    Node& parent_;
    std::vector<std::type_index> types_;

    public:
    gfTerminal(Node& parent_node, std::initializer_list<std::type_index> types, std::string name)
      : parent_(parent_node), types_(types), gfObject(name) {};
    gfTerminal(Node& parent_node, std::vector<std::type_index> types, std::string name)
      : parent_(parent_node), types_(types), gfObject(name) {};

    Node& get_parent() const { return parent_; };

    bool accepts_type(std::type_index type) const;
    const std::vector<std::type_index>& get_types() const { return types_; };
    const virtual gfIO get_side() = 0;
    const virtual gfTerminalFamily get_family() = 0;
    virtual bool has_data() = 0;
    virtual bool has_connection() = 0;

    friend class Node;
  };

  class gfInputTerminal : public gfTerminal, public std::enable_shared_from_this<gfInputTerminal> {
    private:
    bool is_optional_;

    protected:
    virtual void clear();
    virtual void update_on_receive(bool queue) = 0;
    virtual void connect_output(gfOutputTerminal& output_term) = 0;
    virtual void disconnect_output(gfOutputTerminal& output_term) = 0;

    public:
    gfInputTerminal(Node& parent_gnode, std::string name, std::initializer_list<std::type_index> types, bool is_optional=false)
      : gfTerminal(parent_gnode, types, name), is_optional_(is_optional)
      {};
    gfInputTerminal(Node& parent_gnode, std::string name, std::vector<std::type_index> types, bool is_optional=false)
      : gfTerminal(parent_gnode, types, name), is_optional_(is_optional)
      {};
    gfInputTerminal(Node& parent_gnode, std::string name, std::type_index type, bool is_optional=false)
      : gfInputTerminal(parent_gnode, name, {type}, is_optional)
      {};
    virtual ~gfInputTerminal() {};
    std::weak_ptr<gfInputTerminal>  get_ptr(){
      return weak_from_this();
    }
    const gfIO get_side() { return GF_IN; };
    bool is_optional() { return is_optional_; };

    friend class gfOutputTerminal;
    friend class gfBasicMonoOutputTerminal;
    friend class gfMonoOutputTerminal;
    friend class Node;
  };

  class gfMonoInputTerminal : public gfInputTerminal {
    protected:
    std::weak_ptr<gfOutputTerminal> connected_output_;
    void update_on_receive(bool queue);
    void connect_output(gfOutputTerminal& output_term);
    void disconnect_output(gfOutputTerminal& output_term);

    public:
    using gfInputTerminal::gfInputTerminal;
    ~gfMonoInputTerminal();
    bool is_connected_type(std::type_index ttype) const;
    std::type_index get_connected_type() const;
    bool has_connection();
    bool has_data();

    friend class gfMonoOutputTerminal;
  };

  typedef std::set<std::weak_ptr<gfInputTerminal>, std::owner_less<std::weak_ptr<gfInputTerminal>>> InputConnectionSet;

  class gfOutputTerminal : public gfTerminal, public std::enable_shared_from_this<gfOutputTerminal> {
    private:
    bool is_marked_;
    protected:
    InputConnectionSet connections_;

    std::set<NodeHandle> get_child_nodes();
    virtual void propagate();
    virtual void clear() = 0;

    public:
    gfOutputTerminal(Node& parent_gnode, std::string name, std::initializer_list<std::type_index> types, bool is_marked=false) 
      : gfTerminal(parent_gnode, types, name), is_marked_(is_marked) {}
    gfOutputTerminal(Node& parent_gnode, std::string name, std::vector<std::type_index> types, bool is_marked=false) 
      : gfTerminal(parent_gnode, types, name), is_marked_(is_marked) {}
    ~gfOutputTerminal();
    std::weak_ptr<gfOutputTerminal>  get_ptr(){ return weak_from_this(); }
    const InputConnectionSet& get_connections();
    const gfIO get_side() { return GF_OUT; };
    
    bool has_connection() { return connections_.size()>0; };
    bool is_compatible(gfInputTerminal& input_terminal);
    void connect(gfInputTerminal& in);
    void disconnect(gfInputTerminal& in);
    bool is_marked() { return is_marked_; }; 

    friend class Node;
    friend class gfInputTerminal;
    friend class gfGroupOutputTerminal;
    friend class gfMonoInputTerminal;
    friend class gfPolyInputTerminal;
  };

  class gfMonoOutputTerminal : public gfOutputTerminal {
    public:
    using gfOutputTerminal::gfOutputTerminal;
    const std::type_index& get_type() { return types_[0]; };
  };


  class gfBasicMonoInputTerminal : public gfMonoInputTerminal {
    public:
    using gfMonoInputTerminal::gfMonoInputTerminal;
    const gfTerminalFamily get_family() { return GF_BASIC; };
    template<typename T> const T get();
  };

  class gfBasicMonoOutputTerminal : public gfMonoOutputTerminal {
    private:
    std::any data_;
    
    protected:
    void clear();

    public:
    using gfMonoOutputTerminal::gfMonoOutputTerminal;
    const gfTerminalFamily get_family() { return GF_BASIC; };
    bool has_data();
    template<typename T> T& set(T data){
      if(!accepts_type(typeid(T)))
        throw gfException("illegal type for gfBasicMonoOutputTerminal");
      data_ = std::move(data);
      return std::any_cast<T&>(data_);
    };
    void operator=(const std::any& data) {
      data_ = data;
    }
    std::any& get_data() { return data_; };
    template<typename T> const T get() const { 
      return std::any_cast<T>(data_); 
    };

    friend class gfBasicMonoInputTerminal;
    friend class gfPolyOutputTerminal;
  };

  template<typename T> const T gfBasicMonoInputTerminal::get() {
    auto output_term = connected_output_.lock();
    auto sot = (gfBasicMonoOutputTerminal*)(output_term.get());
    return std::any_cast<T>(sot->get_data()); 
  };


  class gfVectorMonoInputTerminal : public gfMonoInputTerminal {    
    public:
    using gfMonoInputTerminal::gfMonoInputTerminal;
    const gfTerminalFamily get_family() { return GF_VECTOR; };
    template<typename T> const T get(size_t i);
    const std::vector<std::any>& get();
    size_t size();
  };

  class gfVectorMonoOutputTerminal : public gfMonoOutputTerminal {
    private:
    std::vector<std::any> data_;

    protected:
    void clear();

    public:
    using gfMonoOutputTerminal::gfMonoOutputTerminal;
    const gfTerminalFamily get_family() { return GF_VECTOR; };
    bool has_data();

    template<typename T> void push_back(T data) {
      if(!accepts_type(typeid(T)))
        throw gfException("illegal type for gfVectorMonoOutputTerminal");
      data_.push_back(std::move(data));
    };
    void push_back_any(std::any& data) {
      data_.push_back(std::move(data));
    }
    size_t size() { return data_.size(); };
    template<typename T>void resize(size_t n) {
      return data_.resize(n, T());
    };
    std::vector<std::any>& get() { return data_; };
    template<typename T> T get(size_t i) { return std::any_cast<T>(data_[i]); };

    friend class gfVectorMonoInputTerminal;
  };

  template<typename T>const T gfVectorMonoInputTerminal::get(size_t i) {
    auto output_term = connected_output_.lock();
    auto sot = (gfVectorMonoOutputTerminal*)(output_term.get());
    return sot->get<T>(i);
  }

  typedef std::set<std::weak_ptr<gfOutputTerminal>, std::owner_less<std::weak_ptr<gfOutputTerminal>>> OutputConnectionSet;
  class gfPolyInputTerminal : public gfInputTerminal {
    typedef std::vector<const gfBasicMonoOutputTerminal*> BasicRefs;
    typedef std::vector<const gfVectorMonoOutputTerminal*> VectorRefs;
    private:
    BasicRefs basic_terminals_;
    VectorRefs vector_terminals_;
    void push_term_ref(gfOutputTerminal* term_ptr);
    void rebuild_terminal_refs();

    protected:
    // void clear();
    OutputConnectionSet connected_outputs_;
    
    void clear();
    void update_on_receive(bool queue);
    void connect_output(gfOutputTerminal& output_term);
    void disconnect_output(gfOutputTerminal& output_term);
    
    public:
    using gfInputTerminal::gfInputTerminal;
    ~gfPolyInputTerminal();
    const gfTerminalFamily get_family() { return GF_POLY; };
    bool has_data();
    bool has_connection() {return connected_outputs_.size() > 0; };

    const BasicRefs& basic_terminals() { return basic_terminals_; };
    const VectorRefs& vector_terminals() { return vector_terminals_; };
  };

  class gfPolyOutputTerminal : public gfOutputTerminal {
    private:
    template<typename T> T& add(std::string term_name, std::initializer_list<std::type_index> ttype) {
      // TODO: check if term_name is unique and if type is in types
      auto t = std::make_shared<T>(get_parent(), term_name, ttype);
      terminals_[term_name] = t;
      auto term = (T*)(terminals_.at(term_name).get());
      return (*term);
    };

    protected:
    typedef std::map<std::string,std::shared_ptr<gfMonoOutputTerminal>> MonoTerminalMap;
    MonoTerminalMap terminals_;
    // bool is_propagated_=false;
    // void propagate(); ?
    void clear();

    public:
    using gfOutputTerminal::gfOutputTerminal;
    const gfTerminalFamily get_family() { return GF_POLY; };
    bool has_data();
    gfBasicMonoOutputTerminal& add(std::string term_name, std::type_index ttype ) ;
    gfVectorMonoOutputTerminal& add_vector(std::string term_name, std::type_index ttype );
    const MonoTerminalMap& get_terminals() { return terminals_; };
    template<typename T> T get_basic(std::string term_name) {
      auto tptr = (gfBasicMonoOutputTerminal*)(terminals_.at(term_name).get()); 
      return std::any_cast<T>(tptr->get_data());
    };

    void connect(gfInputTerminal& in);
    void disconnect(gfInputTerminal& in);


    friend class gfPolyInputTerminal;
  };

  template <typename T> struct get_family {
    static const gfTerminalFamily value = GF_UNKNOWN;
  };
  template<> struct get_family<gfBasicMonoInputTerminal> {
    static const gfTerminalFamily value = GF_BASIC;
  };
  template<> struct get_family<gfVectorMonoInputTerminal> {
    static const gfTerminalFamily value = GF_VECTOR;
  };
  template<> struct get_family<gfPolyInputTerminal> {
    static const gfTerminalFamily value = GF_POLY;
  };
  template<> struct get_family<gfBasicMonoOutputTerminal> {
    static const gfTerminalFamily value = GF_BASIC;
  };
  template<> struct get_family<gfVectorMonoOutputTerminal> {
    static const gfTerminalFamily value = GF_VECTOR;
  };
  template<> struct get_family<gfPolyOutputTerminal> {
    static const gfTerminalFamily value = GF_POLY;
  };

  class Node : public std::enable_shared_from_this<Node>, public gfObject {
    private:
    template<typename T> void add_input(std::string name, std::initializer_list<std::type_index> types, bool is_optional) {
      // TODO: check if name is unique key in input_terminals map
      input_terminals[name] = std::make_shared<T>(
        *this, name, types, is_optional
      );
    }
    template<typename T> void add_output(std::string name, std::initializer_list<std::type_index> types, bool is_marked=false) {
      // TODO: check if name is unique key in output_terminals map
      output_terminals[name] = std::make_shared<T>(
        *this, name, types, is_marked
      );
    }
    template<typename T> void add_input(std::string name, std::vector<std::type_index> types, bool is_optional) {
      // TODO: check if name is unique key in input_terminals map
      input_terminals[name] = std::make_shared<T>(
        *this, name, types, is_optional
      );
    }
    template<typename T> void add_output(std::string name, std::vector<std::type_index> types, bool is_marked=false) {
      // TODO: check if name is unique key in output_terminals map
      output_terminals[name] = std::make_shared<T>(
        *this, name, types, is_marked
      );
    }

    template<typename T> T& input(std::string term_name) {
      if (input_terminals.find(term_name) == input_terminals.end()) {
        throw gfException("No such input terminal - \""+term_name+"\" in " + get_name());
      }
      if (input_terminals[term_name]->get_family() != get_family<T>::value) {
        throw gfException("Illegal terminal down cast - \""+term_name+"\" in " + get_name());
      }
        
      auto input_term = (T*) (input_terminals[term_name].get());
      return *input_term;
    }
    template<typename T> T& output(std::string term_name) {
      if (output_terminals.find(term_name) == output_terminals.end()) {
        throw gfException("No such output terminal - \""+term_name+"\" in " + get_name());
      }
      if (output_terminals[term_name]->get_family() != get_family<T>::value) {
        throw gfException("Illegal terminal down cast - \""+term_name+"\" in " + get_name());
      }

      auto output_term = (T*) (output_terminals[term_name].get());
      return *output_term;
    }

    public:
    typedef std::map<std::string,std::shared_ptr<gfInputTerminal>> InputTerminalMap;
    typedef std::map<std::string,std::shared_ptr<gfOutputTerminal>> OutputTerminalMap;

    InputTerminalMap input_terminals;
    OutputTerminalMap output_terminals;

    // std::map<std::string,std::shared_ptr<InputGroup>> inputGroups;
    // std::map<std::string,std::shared_ptr<OutputGroup>> outputGroups;

    ParameterMap parameters;
    bool autorun = true;
    arr2f position;

    Node(NodeRegisterHandle node_register, NodeManager& manager, std::string type_name, std::string node_name): node_register(node_register), manager(manager), type_name(type_name), gfObject(node_name) {};
    ~Node();

    void remove_from_manager();

    gfBasicMonoInputTerminal& input(std::string term_name) {
      return input<gfBasicMonoInputTerminal>(term_name);
    }
    gfVectorMonoInputTerminal& vector_input(std::string term_name) {
      return input<gfVectorMonoInputTerminal>(term_name);
    }
    gfPolyInputTerminal& poly_input(std::string term_name) {
      return input<gfPolyInputTerminal>(term_name);
    }
    gfBasicMonoOutputTerminal& output(std::string term_name) {
      return output<gfBasicMonoOutputTerminal>(term_name);
    }
    gfVectorMonoOutputTerminal& vector_output(std::string term_name) {
      return output<gfVectorMonoOutputTerminal>(term_name);
    }
    gfPolyOutputTerminal& poly_output(std::string term_name) {
      return output<gfPolyOutputTerminal>(term_name);
    }

    void for_each_input(std::function<void(gfInputTerminal&)> f) {
      for (auto& iT : input_terminals) {
        f(*iT.second);
      }
    }
    void for_each_output(std::function<void(gfOutputTerminal&)> f) {
      for (auto& iT : output_terminals) {
        f(*iT.second);
      }
    }

    gfNodeStatus status_ = GF_NODE_WAITING;

    void add_input(std::string name, std::type_index type, bool is_optional=false) {
      add_input<gfBasicMonoInputTerminal>(name, {type}, is_optional);
    };
    void add_input(std::string name, std::initializer_list<std::type_index> types, bool is_optional=false) {
      add_input<gfBasicMonoInputTerminal>(name, types, is_optional);
    };
    void add_input(std::string name, std::vector<std::type_index> types, bool is_optional=false) {
      add_input<gfBasicMonoInputTerminal>(name, types, is_optional);
    };
    void add_vector_input(std::string name, std::type_index type, bool is_optional=false){
      add_input<gfVectorMonoInputTerminal>(name, {type}, is_optional);
    };
    void add_vector_input(std::string name, std::initializer_list<std::type_index> types, bool is_optional=false) {
      add_input<gfVectorMonoInputTerminal>(name, types, is_optional);
    };
    void add_vector_input(std::string name, std::vector<std::type_index> types, bool is_optional=false) {
      add_input<gfVectorMonoInputTerminal>(name, types, is_optional);
    };
    void add_poly_input(std::string name, std::initializer_list<std::type_index> types, bool is_optional=false) {
      add_input<gfPolyInputTerminal>(name, types, is_optional);
    };
    void add_poly_input(std::string name, std::vector<std::type_index> types, bool is_optional=false) {
      add_input<gfPolyInputTerminal>(name, types, is_optional);
    };

    void add_output(std::string name, std::type_index type, bool is_marked=false) {
      add_output<gfBasicMonoOutputTerminal>(name, {type}, is_marked);
    };
    void add_vector_output(std::string name, std::type_index type, bool is_marked=false) {
      add_output<gfVectorMonoOutputTerminal>(name, {type});
    };
    void add_poly_output(std::string name, std::initializer_list<std::type_index> types, bool is_marked=false) {
      add_output<gfPolyOutputTerminal>(name, types, is_marked);
    };
    void add_poly_output(std::string name, std::vector<std::type_index> types, bool is_marked=false) {
      add_output<gfPolyOutputTerminal>(name, types, is_marked);
    };

    std::set<NodeHandle> get_child_nodes();

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
    void set_autorun(bool b) {
      autorun = b;
    }
    bool is_root() {
      return input_terminals.size()==0;
    }
    bool is_leaf() {
      return output_terminals.size()==0;
    }

    NodeHandle get_handle(){ return shared_from_this(); };
    WeakNodeHandle get_weak_handle(){ return shared_from_this(); };

    bool queue();
    bool update_status();
    void propagate_outputs();
    void notify_children();
    // void preprocess();


    virtual void init() = 0;
    virtual void post_parameter_load() {};
    // virtual std::map<std::string,std::shared_ptr<InputTerminal>> init_inputs() {};
    // virtual std::map<std::string,std::shared_ptr<OutputTerminal>> init_outputs() {};
    // virtual ParameterMap init_parameters() {};
    virtual void process() = 0;
    virtual void gui() {};
    virtual void on_receive(gfMonoInputTerminal& it){};
    virtual void on_receive(gfPolyInputTerminal& it){};
    virtual void on_clear(gfInputTerminal& it){};
    virtual void on_connect_input(gfInputTerminal& ot){};
    virtual void on_connect_output(gfOutputTerminal& ot){};
    virtual void on_change_parameter(std::string name, ParameterVariant& param){};
    virtual void before_gui(){};
    virtual std::string info() {return std::string();};

    std::string debug_info();
    const std::string get_type_name() { return type_name; };
    const NodeRegister& get_register() { return *node_register; };
    const NodeManager& get_manager() { return manager; };

    protected:
    void set_name(std::string new_name);
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

    std::map<std::string, std::function<NodeHandle(NodeRegisterHandle, NodeManager&, std::string, std::string)>> node_types;

    template<class NodeClass> void register_node(std::string type_name) {
      node_types[type_name] = create_node_type<NodeClass>;
    }
    std::string get_name() const {return name;}
    
    protected:
    template<class NodeClass> static std::shared_ptr<NodeClass> create_node_type(NodeRegisterHandle nr, NodeManager& nm, std::string type_name, std::string node_name){
      auto node = std::make_shared<NodeClass>(nr, nm, type_name, node_name);
      node->init();
      return node;
    }
    NodeHandle create(std::string node_name, std::string type_name, NodeManager& nm) {
      if (node_types.find(type_name) == node_types.end())
        throw gfException("No such node type - \""+type_name+"\"");

      auto f = node_types[type_name];
      auto n = f(shared_from_this(), nm, type_name, node_name);
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
    private:
    NodeRegisterMap& registers_;
    std::unordered_map<std::string, NodeHandle> nodes;

    public:
    std::optional<std::array<double,3>> data_offset;
    NodeManager(NodeRegisterMap&  node_registers)
      : registers_(node_registers) {};
    
    NodeRegisterMap& get_node_registers() const { return registers_; };
    void operator= (const NodeManager& other_manager) {
      registers_ = other_manager.get_node_registers();
    };

    NodeHandle create_node(NodeRegisterHandle node_register, std::string type_name);
    NodeHandle create_node(NodeRegister& node_register, std::string type_name, std::pair<float,float> pos);
    NodeHandle create_node(NodeRegisterHandle node_register, std::string type_name, std::pair<float,float> pos);
    void remove_node(NodeHandle node);
    void clear();

    bool name_node(NodeHandle node, std::string new_name);

    std::unordered_map<std::string, NodeHandle>& get_nodes() { return nodes; };
    std::vector<NodeHandle> dump_nodes();

    std::vector<NodeHandle> load_json(std::string filepath, bool strict=false);
    void dump_json(std::string filepath);

    // load_json() {

    // }
    
    bool run();
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
  // bool connect(OutputGroup& in, InputGroup& out);
  bool is_compatible(gfTerminal& t1, gfTerminal& t2);
  void disconnect(gfTerminal& t1, gfTerminal& t2);
  bool detect_loop(gfTerminal& t1, gfTerminal& t2);

  // bool detect_loop(gfOutputTerminal& iT, gfInputTerminal& oT);
}
