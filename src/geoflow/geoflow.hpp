// This file is part of Geoflow
// Copyright (C) 2018-2022 Ravi Peters

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

#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include)
  #if __has_include(<filesystem>)
    #define GHC_USE_STD_FS
    #include <filesystem>
    namespace fs = std::filesystem;
  #endif
#endif
#ifndef GHC_USE_STD_FS
  #include <ghc/filesystem.hpp>
  namespace fs = ghc::filesystem;
#endif

#include "common.hpp"
#include "parameters.hpp"

namespace geoflow {

  class gfObject {
    protected:
    std::string name_;

    public:
    gfObject(std::string name) : name_(name) {};
    const std::string& get_name() const { return name_; };
    const std::string* get_name_ptr() const { return &name_; };
  };

  // This is the base class for all Geoflow exceptions.
  // More specific exceptions are derived from this class.
  // This class is also used when all Geoflow exception need to be caught.
  class gfException: public std::exception
  {
  public:
    explicit gfException(const std::string& message):
      msg_("Error: " + message)
      {}
    virtual const char* what() const throw (){
      return msg_.c_str();
    }

  protected:
      std::string msg_;
  };

  // Thrown when there are issues with the flowchart (e.g. it cannot be read).
  class gfFlowchartError: public gfException
  {
  public:
    using gfException::gfException;
  };

  // Thrown at a broken input (e.g. invalid file paths).
  class gfIOError: public gfException
  {
  public:
    using gfException::gfException;
  };

  // Thrown when the data on the inputs of a node has the right type but
  // inappropriate data (e.g. two input vectors should be of the same
  // length).
  class gfNodeInputDataError: public gfException
  {
  public:
    using gfException::gfException;
  };

  // Thrown when the data on the inputs of a node has an inappropriate type
  // or logic (e.g. a loop).
  class gfNodeTerminalError: public gfException
  {
  public:
    using gfException::gfException;
  };

  class Node;
  class NodeManager;
  class NodeRegister;
  typedef std::shared_ptr<NodeRegister> NodeRegisterHandle;
  // typedef std::weak_ptr<InputTerminal> InputHandle;
  // typedef std::weak_ptr<OutputTerminal> OutputHandle;
  
  typedef std::shared_ptr<Node> NodeHandle;
  typedef std::weak_ptr<Node> WeakNodeHandle;
  
  class gfOutputTerminal;
  class gfSingleFeatureOutputTerminal;

  enum gfIO {GF_IN, GF_OUT};
  // enum gfTerminalFamily {GF_UNKNOWN, GF_BASIC, GF_VECTOR, GF_POLY};
  enum gfTerminalFamily {GF_UNKNOWN, GF_SINGLE_FEATURE, GF_MULTI_FEATURE};
  enum gfNodeStatus {GF_NODE_WAITING, GF_NODE_READY, GF_NODE_PROCESSING, GF_NODE_DONE};

  class gfTerminal : public gfObject {
    private:
    const bool supports_multiple_elements_;
    bool is_marked_=false;
    protected:
    Node& parent_;
    std::vector<std::type_index> types_;

    public:
    gfTerminal(Node& parent_node, std::initializer_list<std::type_index> types, std::string name, bool supports_multiple_elements)
      : parent_(parent_node), types_(types), gfObject(name), supports_multiple_elements_(supports_multiple_elements) {};
    gfTerminal(Node& parent_node, std::vector<std::type_index> types, std::string name, bool supports_multiple_elements)
      : parent_(parent_node), types_(types), gfObject(name), supports_multiple_elements_(supports_multiple_elements) {};

    Node& get_parent() const { return parent_; };

    bool accepts_type(std::type_index type) const;
    bool supports_multiple_elements() const { return supports_multiple_elements_; };
    std::string get_full_name() const;
    const std::vector<std::type_index>& get_types() const { return types_; };
    const virtual gfIO get_side() = 0;
    const virtual gfTerminalFamily get_family() = 0;
    virtual bool has_data() const = 0;
    virtual bool has_connection() = 0;
    virtual bool is_touched() = 0;

    void set_marked(bool is_marked) { is_marked_ = is_marked; };
    bool& is_marked() { return is_marked_; };

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
    gfInputTerminal(Node& parent_gnode, std::string name, std::initializer_list<std::type_index> types, bool is_optional, bool supports_multiple_elements)
      : gfTerminal(parent_gnode, types, name, supports_multiple_elements), is_optional_(is_optional)
      {};
    gfInputTerminal(Node& parent_gnode, std::string name, std::vector<std::type_index> types, bool is_optional, bool supports_multiple_elements)
      : gfTerminal(parent_gnode, types, name, supports_multiple_elements), is_optional_(is_optional)
      {};
    gfInputTerminal(Node& parent_gnode, std::string name, std::type_index type, bool is_optional, bool supports_multiple_elements)
      : gfInputTerminal(parent_gnode, name, {type}, is_optional, supports_multiple_elements)
      {};
    virtual ~gfInputTerminal() {};
    std::weak_ptr<gfInputTerminal>  get_ptr(){
      return weak_from_this();
    }
    const gfIO get_side() { return GF_IN; };
    bool is_optional() { return is_optional_; };
    virtual size_t size() const = 0;

    friend class gfOutputTerminal;
    friend class gfSingleFeatureOutputTerminal;
    friend class Node;
  };

  class gfSingleFeatureInputTerminal : public gfInputTerminal {
    protected:
    std::weak_ptr<gfOutputTerminal> connected_output_;
    void update_on_receive(bool queue);
    void connect_output(gfOutputTerminal& output_term);
    void disconnect_output(gfOutputTerminal& output_term);

    public:
    using gfInputTerminal::gfInputTerminal;
    ~gfSingleFeatureInputTerminal();
    bool is_connected_type(std::type_index ttype) const;
    std::type_index get_connected_type() const;
    bool has_connection();
    bool has_data() const;
    bool is_touched();

    // single element
    // const gfTerminalFamily get_family() { return GF_BASIC; };
    template<typename T> const T get();

    // multi element (vector)
    const gfTerminalFamily get_family() { return GF_SINGLE_FEATURE; };
    template<typename T> const T get(size_t i);
    const std::vector<std::any>& get_data_vec() const;
    size_t size() const;

    friend class gfSingleFeatureOutputTerminal;
  };


  typedef std::set<std::weak_ptr<gfInputTerminal>, std::owner_less<std::weak_ptr<gfInputTerminal>>> InputConnectionSet;

  class gfOutputTerminal : public gfTerminal, public std::enable_shared_from_this<gfOutputTerminal> {
    protected:
    InputConnectionSet connections_;
    bool is_touched_=false;

    std::set<NodeHandle> get_child_nodes();
    virtual void propagate();
    virtual void clear() = 0;

    public:
    gfOutputTerminal(Node& parent_gnode, std::string name, std::initializer_list<std::type_index> types, bool supports_multiple_elements) 
      : gfTerminal(parent_gnode, types, name, supports_multiple_elements) {}
    gfOutputTerminal(Node& parent_gnode, std::string name, std::vector<std::type_index> types, bool supports_multiple_elements) 
      : gfTerminal(parent_gnode, types, name, supports_multiple_elements) {}
    ~gfOutputTerminal();
    std::weak_ptr<gfOutputTerminal>  get_ptr(){ return weak_from_this(); }
    const InputConnectionSet& get_connections();
    const gfIO get_side() { return GF_OUT; };
    
    bool has_connection() { return connections_.size()>0; };
    bool is_compatible(gfInputTerminal& input_terminal);
    void connect(gfInputTerminal& in);
    void disconnect(gfInputTerminal& in);

    virtual size_t size() const=0;
    void set_type(std::type_index type) {types_ = {type}; }

    void touch() { is_touched_=true; };
    bool is_touched() { return is_touched_; };

    friend class Node;
    friend class gfInputTerminal;
    friend class gfGroupOutputTerminal;
    friend class gfSingleFeatureInputTerminal;
    friend class gfMultiFeatureInputTerminal;
  };

  class gfSingleFeatureOutputTerminal : public gfOutputTerminal {
    // private:
    // std::any data_;
    private:
    std::vector<std::any> data_;
    
    protected:
    // void clear();
    void clear();

    public:
    using gfOutputTerminal::gfOutputTerminal;
    const std::type_index& get_type() const { return types_[0]; };

    // single element
    const gfTerminalFamily get_family() { return GF_SINGLE_FEATURE; };
    bool has_data() const;
    void push_back_any(const std::any& data) {
      data_.push_back(data);
    }
    template<typename T> void push_back(T data) {
      if(!accepts_type(typeid(T)))
        throw gfException("illegal type for gfSingleFeatureOutputTerminal");
      data_.push_back(std::move(data));
      touch();
    };
    template<typename T> T& set(T data){
      if(!accepts_type(typeid(T)))
        throw gfException("illegal type for gfSingleFeatureOutputTerminal");
      data_.clear();
      push_back(data);
      return std::any_cast<T&>(data_[0]);
    };
    void set_from_any(const std::any& data) {
      data_.clear();
      data_.resize(1);
      data_[0] = data;
      touch();
    }
    void operator=(const std::vector<std::any>& data_vec) {
      data_.clear();
      data_ = data_vec;
      touch();
    }

    bool has_value(size_t i=0) {
      return !data_[i].has_value();
    }

    // multi element
    size_t size() const { return data_.size(); };
    template<typename T>void resize(size_t n) {
      return data_.resize(n, T());
    };
    std::any& get_data() { return data_[0]; };
    const std::any& get_data() const { return data_[0]; };
    std::vector<std::any>& get_data_vec() { return data_; };
    const std::vector<std::any>& get_data_vec() const { return data_; };
    template<typename T> T get(size_t i) { 
      return std::any_cast<T>(data_[i]); 
    };
    template<typename T> const T get(size_t i) const { 
      return std::any_cast<T>(data_[i]); 
    };
    template<typename T> T get() { 
      return get<T>(0); 
    };
    template<typename T> const T get() const { 
      return get<T>(0); 
    };


    friend class gfSingleFeatureInputTerminal;
    friend class gfMultiFeatureOutputTerminal;
  };

  template<typename T>const T gfSingleFeatureInputTerminal::get(size_t i) {
    auto output_term = connected_output_.lock();
    auto sot = (gfSingleFeatureOutputTerminal*)(output_term.get());
    return sot->get<T>(i);
  }
  template<typename T> const T gfSingleFeatureInputTerminal::get() {
    return get<T>(0);
  };

  typedef std::set<std::weak_ptr<gfOutputTerminal>, std::owner_less<std::weak_ptr<gfOutputTerminal>>> OutputConnectionSet;
  
  class gfMultiFeatureInputTerminal : public gfInputTerminal {
    typedef std::vector<const gfSingleFeatureOutputTerminal*> SubTermRefs;
    // typedef std::vector<const gfSingleFeatureOutputTerminal*> BasicRefs;
    // typedef std::vector<const gfSingleFeatureOutputTerminal*> VectorRefs;
    private:
    // BasicRefs basic_terminals_;
    // VectorRefs vector_terminals_;
    SubTermRefs sub_terminals_;
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
    ~gfMultiFeatureInputTerminal();
    const gfTerminalFamily get_family() { return GF_MULTI_FEATURE; };
    bool has_data() const;
    bool is_touched();
    bool has_connection() {return connected_outputs_.size() > 0; };
    size_t size() const;

    const SubTermRefs& sub_terminals() { return sub_terminals_; };
    // const BasicRefs& basic_terminals() { return basic_terminals_; };
    // const VectorRefs& vector_terminals() { return vector_terminals_; };
  };

  class gfMultiFeatureOutputTerminal : public gfOutputTerminal {
    private:
    template<typename T> T& add(std::string term_name, std::initializer_list<std::type_index> ttype, bool supports_multiple_elements) {
      // TODO: check if term_name is unique and if type is in types
      auto t = std::make_shared<T>(get_parent(), term_name, ttype, supports_multiple_elements);
      terminals_[term_name] = t;
      auto term = (T*)(terminals_.at(term_name).get());
      return (*term);
    };

    protected:
    typedef std::map<std::string,std::shared_ptr<gfSingleFeatureOutputTerminal>> SFOTerminalMap;
    SFOTerminalMap terminals_;
    // bool is_propagated_=false;
    // void propagate(); ?
    void clear();

    public:
    using gfOutputTerminal::gfOutputTerminal;
    const gfTerminalFamily get_family() { return GF_MULTI_FEATURE; };
    bool has_data() const;
    size_t size() const;

    // note these 2 are almost the same now:
    gfSingleFeatureOutputTerminal& add(std::string term_name, std::type_index ttype ) ;
    gfSingleFeatureOutputTerminal& add_vector(std::string term_name, std::type_index ttype );

    const SFOTerminalMap& sub_terminals() { return terminals_; };
    gfSingleFeatureOutputTerminal& sub_terminal(std::string term_name) {
      return *terminals_.at(term_name).get();
    };

    void operator=(gfMultiFeatureInputTerminal& gfMFInput) {
      clear();
      for(const auto& iterm : gfMFInput.sub_terminals()) {
        auto& oterm = add_vector(iterm->get_name(), iterm->get_type());
        oterm = iterm->get_data_vec();
      }
      touch();
    }

    // void connect(gfInputTerminal& in);
    // void disconnect(gfInputTerminal& in);


    friend class gfMultiFeatureInputTerminal;
  };

  template <typename T> struct get_family {
    static const gfTerminalFamily value = GF_UNKNOWN;
  };
  template<> struct get_family<gfSingleFeatureInputTerminal> {
    static const gfTerminalFamily value = GF_SINGLE_FEATURE;
  };
  template<> struct get_family<gfMultiFeatureInputTerminal> {
    static const gfTerminalFamily value = GF_MULTI_FEATURE;
  };
  template<> struct get_family<gfSingleFeatureOutputTerminal> {
    static const gfTerminalFamily value = GF_SINGLE_FEATURE;
  };
  template<> struct get_family<gfMultiFeatureOutputTerminal> {
    static const gfTerminalFamily value = GF_MULTI_FEATURE;
  };

  class Node : public std::enable_shared_from_this<Node>, public gfObject {
    private:
    template<typename T> T& add_input(std::string name, std::initializer_list<std::type_index> types, bool is_optional, bool supports_multiple_elements) {
      // TODO: check if name is unique key in input_terminals map
      auto term_handle = std::make_shared<T>(
        *this, name, types, is_optional, supports_multiple_elements
      );
      input_terminals[name] = term_handle;
      return *term_handle;
    }
    template<typename T> T& add_output(std::string name, std::initializer_list<std::type_index> types, bool supports_multiple_elements) {
      // TODO: check if name is unique key in output_terminals map
      auto term_handle  = std::make_shared<T>(
        *this, name, types, supports_multiple_elements
      );
      output_terminals[name]= term_handle;
      return *term_handle;
    }
    template<typename T> T& add_input(std::string name, const std::vector<std::type_index> types, bool is_optional, bool supports_multiple_elements) {
      // TODO: check if name is unique key in input_terminals map
      auto term_handle = std::make_shared<T>(
        *this, name, types, is_optional, supports_multiple_elements
      );
      input_terminals[name] = term_handle;
      return *term_handle;
    }
    template<typename T> T& add_output(std::string name, const std::vector<std::type_index> types, bool supports_multiple_elements) {
      // TODO: check if name is unique key in output_terminals map
      auto term_handle  = std::make_shared<T>(
        *this, name, types, supports_multiple_elements
      );
      output_terminals[name]= term_handle;
      return *term_handle;
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

    gfSingleFeatureInputTerminal& input(std::string term_name) {
      return input<gfSingleFeatureInputTerminal>(term_name);
    }
    gfSingleFeatureInputTerminal& vector_input(std::string term_name) {
      return input<gfSingleFeatureInputTerminal>(term_name);
    }
    gfMultiFeatureInputTerminal& poly_input(std::string term_name) {
      return input<gfMultiFeatureInputTerminal>(term_name);
    }
    gfSingleFeatureOutputTerminal& output(std::string term_name) {
      return output<gfSingleFeatureOutputTerminal>(term_name);
    }
    gfSingleFeatureOutputTerminal& vector_output(std::string term_name) {
      return output<gfSingleFeatureOutputTerminal>(term_name);
    }
    gfMultiFeatureOutputTerminal& poly_output(std::string term_name) {
      return output<gfMultiFeatureOutputTerminal>(term_name);
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

    gfSingleFeatureInputTerminal& add_input(std::string name, std::type_index type, bool is_optional=false) {
      return add_input<gfSingleFeatureInputTerminal>(name, {type}, is_optional, false);
    };
    gfSingleFeatureInputTerminal& add_input(std::string name, std::initializer_list<std::type_index> types, bool is_optional=false) {
      return add_input<gfSingleFeatureInputTerminal>(name, types, is_optional, false);
    };
    gfSingleFeatureInputTerminal& add_input(std::string name, std::vector<std::type_index> types, bool is_optional=false) {
      return add_input<gfSingleFeatureInputTerminal>(name, types, is_optional, false);
    };
    gfSingleFeatureInputTerminal& add_vector_input(std::string name, std::type_index type, bool is_optional=false){
      return add_input<gfSingleFeatureInputTerminal>(name, {type}, is_optional, true);
    };
    gfSingleFeatureInputTerminal& add_vector_input(std::string name, std::initializer_list<std::type_index> types, bool is_optional=false) {
      return add_input<gfSingleFeatureInputTerminal>(name, types, is_optional, true);
    };
    gfSingleFeatureInputTerminal& add_vector_input(std::string name, std::vector<std::type_index> types, bool is_optional=false) {
      return add_input<gfSingleFeatureInputTerminal>(name, types, is_optional, true);
    };
    gfMultiFeatureInputTerminal& add_poly_input(std::string name, std::initializer_list<std::type_index> types, bool is_optional=false) {
      return add_input<gfMultiFeatureInputTerminal>(name, types, is_optional, true);
    };
    gfMultiFeatureInputTerminal& add_poly_input(std::string name, std::vector<std::type_index> types, bool is_optional=false) {
      return add_input<gfMultiFeatureInputTerminal>(name, types, is_optional, true);
    };

    gfSingleFeatureOutputTerminal& add_output(std::string name, std::type_index type) {
      return add_output<gfSingleFeatureOutputTerminal>(name, {type}, false);
    };
    gfSingleFeatureOutputTerminal& add_output(std::string name, const std::vector<std::type_index> types) {
      return add_output<gfSingleFeatureOutputTerminal>(name, types, false);
    };
    gfSingleFeatureOutputTerminal& add_vector_output(std::string name, std::type_index type) {
      return add_output<gfSingleFeatureOutputTerminal>(name, {type}, true);
    };
    gfMultiFeatureOutputTerminal& add_poly_output(std::string name, std::initializer_list<std::type_index> types) {
      return add_output<gfMultiFeatureOutputTerminal>(name, types, true);
    };
    gfMultiFeatureOutputTerminal& add_poly_output(std::string name, std::vector<std::type_index> types) {
      return add_output<gfMultiFeatureOutputTerminal>(name, types, true);
    };

    std::set<NodeHandle> get_child_nodes();

    template<typename T> void add_param(T parameter) {
      parameters.emplace(parameter.get_label(), std::make_shared<T>(parameter));
    }
    // void set_param(std::string name, Parameter param, bool quiet=false);
    // void set_params(ParameterMap param_map, bool quiet=false);
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
    virtual bool inputs_valid();
    virtual bool parameters_valid();
    virtual void process() = 0;
    virtual void gui() {};
    virtual void on_receive(gfSingleFeatureInputTerminal& it){};
    virtual void on_receive(gfMultiFeatureInputTerminal& it){};
    virtual void on_clear(gfInputTerminal& it){};
    virtual void on_connect_input(gfInputTerminal& ot){};
    virtual void on_connect_output(gfOutputTerminal& ot){};
    virtual void on_change_parameter(std::string name, Parameter& param){};
    virtual void before_gui(){};
    virtual std::string info() {return std::string();};

    std::string debug_info();
    const std::string get_type_name() { return type_name; };
    const NodeRegister& get_register() { return *node_register; };
    const NodeManager& get_manager() { return manager; };
    
    std::string substitute_from_term(const std::string& textt, gfMultiFeatureInputTerminal& term, const size_t& i=0);

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
    typedef std::unordered_map<std::string, std::string> string_map;

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
    string_map& get_plugin_info() {return plugin_info;}
    
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
    string_map plugin_info;
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
    // global flowchart parameters

    public:
    std::map<std::string, std::shared_ptr<Parameter>> global_flowchart_params;
    std::optional<std::array<double,3>> data_offset;
    fs::path flowchart_path{};
    NodeManager(NodeRegisterMap&  node_registers)
      : registers_(node_registers) {};
    NodeManager(NodeManager&  other_node_manager)
      : registers_(other_node_manager.registers_) {
        std::stringstream ss;
        other_node_manager.json_serialise(ss);
        set_globals(other_node_manager);
        json_unserialise(ss);
        data_offset = *other_node_manager.data_offset;
      };
    
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

    NodeHandle& get_node(std::string& node_name) { return nodes[node_name]; };
    std::unordered_map<std::string, NodeHandle>& get_nodes() { return nodes; };
    std::vector<NodeHandle> dump_nodes();

    std::vector<NodeHandle> load_json(std::string filepath, bool strict=false);
    void dump_json(std::string filepath);

    std::vector<NodeHandle> json_unserialise(std::istream& json_sstream, bool strict=false);
    void json_serialise(std::ostream& json_sstream);

    void set_globals(const NodeManager& other_manager);

    std::string substitute_globals(const std::string& text) const;
    
    size_t run_all(bool notify_children=true);
    size_t run(Node &node, bool notify_children=true);
    size_t run(NodeHandle node, bool notify_children=true) {
      return run(*node, notify_children);
    };
    
    protected:
    std::queue<NodeHandle> node_queue;
    void queue(NodeHandle n);
    
    friend class Node;
  };

  std::string get_global_name(const std::string& text);

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
