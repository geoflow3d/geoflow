#pragma once

#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <functional>
#include <any>
#include <map>
#include <set>
#include <queue>
#include <optional>
#include <unordered_map>
#include <exception>
#include <optional>

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

  enum gfGeometryType {points, lines, line_strip, line_loop, triangles};

  typedef std::vector< vec3f > Geometry;
  typedef std::unordered_map< std::string, std::vector<float>> AttributeMap;
  struct Feature {
      gfGeometryType type;
      Geometry geom;
      AttributeMap attr;
  };

  // this is deprecated:
  enum gfGeometryFormat {simple, count, index_count};
  struct gfGeometry3D {
    gfGeometryType type;
    gfGeometryFormat format;
    Box bounding_box;
    vec3f vertices;
    vec3f normals;
    vec1ui indices;
    vec1ui counts;
    vec1ui firsts;
  };
  
  enum TerminalType : uint32_t{
    TT_any = 0,
    TT_float,
    TT_int,
    TT_vec1ui,
    TT_vec1i,
    TT_vec1f,
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
    TT_linear_ring_collection
  };
  class Terminal {
    public:
    Terminal(Node& parent_gnode):parent(parent_gnode) {
      // std::cout<< "Constructing geoflow::Terminal " << this << "\n";
    }; 
    ~Terminal() {
      // std::cout<< "Destructing geoflow::Terminal " << this << "\n";
    }

    Node& parent;
    std::any cdata;

    virtual void push(std::any data) = 0;
    virtual void clear() = 0;
    
    bool has_data() {return cdata.has_value();};
    template<typename T> T get_data() { return std::any_cast<T>(cdata); };
  };

  class InputTerminal : public Terminal, public std::enable_shared_from_this<InputTerminal>{
    std::vector<TerminalType> types;
    public:
    TerminalType connected_type;
    InputTerminal(Node& parent_gnode, std::initializer_list<TerminalType> types): Terminal(parent_gnode), types(types) {};
    
    std::weak_ptr<InputTerminal> get_ptr(){
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
    
    OutputTerminal(Node& parent_gnode, TerminalType type): Terminal(parent_gnode), type(type){};
    ~OutputTerminal();
    
    std::weak_ptr<OutputTerminal> get_ptr(){
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
  class Node : public std::enable_shared_from_this<Node>{
    public:
    Node(NodeManager& manager, std::string name) : manager(manager), type_name(name){
    };
    ~Node(){
      // std::cout<< "Destructing geoflow::Node " << this << "\n";
      notify_children();
    }

    std::map<std::string,std::shared_ptr<InputTerminal>> inputTerminals;
    std::map<std::string,std::shared_ptr<OutputTerminal>> outputTerminals;

    node_status status=WAITING;
    const std::string type_name;
    NodeManager& manager;

    void add_input(std::string name, TerminalType type);
    void add_input(std::string name, std::initializer_list<TerminalType> types);
    void add_output(std::string name, TerminalType type);

    std::shared_ptr<Node> get_ptr(){return shared_from_this();};

    bool update();
    void propagate_outputs();
    void notify_children();

    // private:
    std::any get_value(std::string input_name){
      return inputTerminals[input_name]->cdata;
    }
    template<typename T> void set_value(const std::string output_name, T value){
      outputTerminals[output_name]->push(std::any(value));
    }

    virtual void on_push(InputTerminal& it){};
    virtual void on_clear(InputTerminal& it){};
    virtual void on_connect(OutputTerminal& ot){};
    virtual void process(){};
    virtual void gui(){};

    std::string get_info();
  };

  class ObjectNode {};


  class NodeManager {
    public:
    std::optional<std::array<double,3>> data_offset;
    NodeManager(){};
    std::queue<std::shared_ptr<Node>> node_queue;
    std::map<std::string, std::function<std::shared_ptr<Node>(NodeManager&)>> node_register;

    template<class NodeClass> void register_node(std::string name) {
      node_register[name] = create_node<NodeClass>;
    }
    template<class NodeClass> static std::shared_ptr<NodeClass> create_node(NodeManager& nm){
      return std::make_shared<NodeClass>(nm);
    }
    std::shared_ptr<Node> create(std::string name) {
      auto f = node_register[name];
      auto n = f(*this);
      return n;
    }
    
    bool run(Node &node);
    void queue(std::shared_ptr<Node> n);
    
  };

  bool connect(Node& n1, Node& n2, std::string s1, std::string s2); // connect only node of the same nodemanager?
  bool connect(Terminal& t1, Terminal& t2);
  bool is_compatible(Terminal& t1, Terminal& t2);
  void disconnect(Terminal& t1, Terminal& t2);
  bool detect_loop(Terminal& t1, Terminal& t2);
  bool detect_loop(OutputTerminal& iT, InputTerminal& oT);
}
