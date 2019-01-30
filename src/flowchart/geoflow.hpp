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

namespace geoflow {

  struct ConnectionException : public std::exception
  {
    const char * what () const throw ()
      {
        return "Terminals have incompatible types!";
      }
  };

  class Node;
  class NodeManager;
  class InputTerminal;
  class OutputTerminal;
  // typedef std::weak_ptr<InputTerminal> InputHandle;
  // typedef std::weak_ptr<OutputTerminal> OutputHandle;

  typedef std::variant<bool,int,double,std::string> Parameter;
  typedef std::unordered_map<std::string, Parameter> ParameterMap;
  typedef std::shared_ptr<Node> NodeHandle;
  
  enum TerminalType : uint32_t {
    TT_any = 0,
    TT_float,
    TT_int,
    TT_vec1ui,
    TT_vec1i,
    TT_vec1f,
    TT_vec1b,
    TT_vec2f,
    TT_vec3f,
    TT_vec6f,
    TT_vec_float,
    TT_colmap,
    TT_geometry,
    TT_feature,
    TT_point_collection, 
    TT_triangle_collection,
    TT_line_string_collection,
    TT_linear_ring_collection,
    TT_attribute_map_f
  };
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

  enum node_status {
    WAITING,
    READY,
    PROCESSING,
    DONE
  };

  class Node : public std::enable_shared_from_this<Node> {
    public:

    std::map<std::string,std::shared_ptr<InputTerminal> > inputTerminals;
    std::map<std::string,std::shared_ptr<OutputTerminal>> outputTerminals;

    ParameterMap parameters;

    Node(NodeManager& manager, std::string type_name): manager(manager), type_name(type_name) {};
    ~Node() {
      std::cout<< "Destructing geoflow::Node " << type_name << " " << name << "\n";
      notify_children();
    }

    InputTerminal& inputs(std::string name) {
      return *inputTerminals.at(name);
    }
    OutputTerminal& outputs(std::string name) {
      return *outputTerminals.at(name);
    }
    template<typename T> T& params(std::string name) {
      return std::get<T>(parameters.at(name));
    }

    node_status status=WAITING;

    void add_input(std::string name, TerminalType type);
    void add_input(std::string name, std::initializer_list<TerminalType> types);
    void add_output(std::string name, TerminalType type);

    void load_params(ParameterMap param_map);
    const ParameterMap&  dump_params();

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
    NodeRegister(){};
    std::map<std::string, std::function<NodeHandle(NodeManager&, std::string)>> node_type_register;

    template<class NodeClass> void register_node(std::string type_name) {
      node_type_register[type_name] = create_node_type<NodeClass>;
    }
    NodeHandle create(std::string type_name, NodeManager& nm) {
      auto f = node_type_register[type_name];
      auto n = f(nm, type_name);
      return n;
    }
    protected:
    template<class NodeClass> static std::shared_ptr<NodeClass> create_node_type(NodeManager& nm, std::string type_name){
      auto node = std::make_shared<NodeClass>(nm, type_name);
      node->init();
      return node;
    }
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
    void remove_node(NodeHandle node);

    bool name_node(NodeHandle node, std::string new_name);

    std::vector<NodeHandle> dump_nodes();

    ConnectionList dump_connections();

    // load_json() {

    // }
    
    bool run(Node &node);
    
    protected:
    std::queue<NodeHandle> node_queue;
    void queue(NodeHandle n);
    
    friend class Node;
  };

  bool connect(Node& n1, Node& n2, std::string s1, std::string s2);
  bool connect(NodeHandle n1, NodeHandle n2, std::string s1, std::string s2);
  bool connect(Terminal& t1, Terminal& t2);
  bool is_compatible(Terminal& t1, Terminal& t2);
  void disconnect(Terminal& t1, Terminal& t2);
  bool detect_loop(Terminal& t1, Terminal& t2);
  bool detect_loop(OutputTerminal& iT, InputTerminal& oT);
}
