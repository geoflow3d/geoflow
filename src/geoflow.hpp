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

#include <iostream>
#include <sstream>

namespace geoflow {

  class Node;
  class NodeManager;

  enum TerminalType : uint32_t{
    TT_any = 0,
    TT_float,
    TT_int
  };
  class Terminal {
    public:
    Terminal(Node& parent_gnode, TerminalType type):parent(parent_gnode), type(type) {
      std::cout<< "Constructing geoflow::Terminal " << this << "\n";
    }; 
    ~Terminal() {
      std::cout<< "Destructing geoflow::Terminal " << this << "\n";
    }

    TerminalType type;
    Node& parent;
    std::any cdata;

    virtual void push(std::any data) = 0;
    
    bool has_data() {return cdata.has_value();};
  };

  class InputTerminal : public Terminal, public std::enable_shared_from_this<InputTerminal>{

    public:
    InputTerminal(Node& parent_gnode, TerminalType type): Terminal(parent_gnode, type){};
    
    std::weak_ptr<InputTerminal> get_ptr(){
      return weak_from_this();
    }
    void push(std::any data);
    void clear();   
  };
  class OutputTerminal : public Terminal, public std::enable_shared_from_this<OutputTerminal>{
    public:
    //use a set to make sure we don't get duplicate connections
    std::set<std::weak_ptr<InputTerminal>, std::owner_less<std::weak_ptr<InputTerminal>>> connections;
    
    OutputTerminal(Node& parent_gnode, TerminalType type): Terminal(parent_gnode, type){};
    ~OutputTerminal();
    
    std::weak_ptr<OutputTerminal> get_ptr(){
      return weak_from_this();
    }

    void connect(InputTerminal& in);
    void disconnect(InputTerminal& in);
    
    void push(std::any data);
  };

  enum node_status {
    WAITING,
    READY,
    PROCESSING,
    DONE,
    ERROR
  };
  class Node : public std::enable_shared_from_this<Node>{
    public:
    Node(NodeManager& manager, std::string name) : manager(manager), name(name){
    };
    ~Node(){std::cout<< "Destructing geoflow::Node " << this << "\n";}

    std::map<std::string,std::shared_ptr<InputTerminal>> inputTerminals;
    std::map<std::string,std::shared_ptr<OutputTerminal>> outputTerminals;
    struct parameters;
    node_status status=WAITING;
    std::string name;
    NodeManager& manager;

    void add_input(std::string name, TerminalType type);
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

    virtual void process()=0;

    std::string get_info();
  };


  class NodeManager {
    public:
    NodeManager(){};
    // std::vector<std::shared_ptr<Node>> nodes;
    std::queue<std::shared_ptr<Node>> node_queue;
    std::map<std::string, std::function<std::shared_ptr<Node>(NodeManager&)>> node_register;

    template<class NodeClass> static std::shared_ptr<NodeClass> create_node(NodeManager& nm){
      return std::make_shared<NodeClass>(nm);
    }
    
    bool run(Node &node);
    void run_node(std::shared_ptr<Node> node);
    bool check_process();
    void queue(std::shared_ptr<Node> n);
    template<class NodeClass> void register_node(std::string name) {
      node_register[name] = create_node<NodeClass>;
    }
    std::shared_ptr<Node> create(std::string name);
  };

  bool connect(Node& n1, Node& n2, std::string s1, std::string s2);
  bool connect(Terminal* t1, Terminal* t2);
  void disconnect(Terminal* t1, Terminal* t2);
  bool detect_loop(Terminal* t1, Terminal* t2);
  bool detect_loop(OutputTerminal* iT, InputTerminal* oT);
}