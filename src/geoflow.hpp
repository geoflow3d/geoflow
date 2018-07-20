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
    Terminal(Node& parent_gnode, TerminalType type):parent(parent_gnode), type(type){};   

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
    
  };
  class OutputTerminal : public Terminal, public std::enable_shared_from_this<OutputTerminal>{
    public:
    std::set<std::weak_ptr<InputTerminal>, std::owner_less<std::weak_ptr<InputTerminal>>> connections;

    
    OutputTerminal(Node& parent_gnode, TerminalType type): Terminal(parent_gnode, type){};
    
    std::weak_ptr<OutputTerminal> get_ptr(){
      return weak_from_this();
    }

    void connect(InputTerminal& in) {
      connections.insert(in.get_ptr());
    }
    
    void push(std::any data);
  };

  enum node_status {
    WAITING,
    PROCESSING,
    DONE
  };
  class Node : public std::enable_shared_from_this<Node>{
    public:
    Node(NodeManager& manager, std::string name) : manager(manager), name(name){
    };

    std::map<std::string,std::shared_ptr<InputTerminal>> inputTerminals;
    std::map<std::string,std::shared_ptr<OutputTerminal>> outputTerminals;
    struct parameters;
    node_status status=WAITING;
    std::string name;
    NodeManager& manager;

    void add_input(std::string name, TerminalType type){
      inputTerminals[name] = std::make_shared<InputTerminal>(
        *this, type
      );
    }
    void add_output(std::string name, TerminalType type){
      outputTerminals[name] = std::make_shared<OutputTerminal>(
        *this, type
      );
    }

    std::shared_ptr<Node> get_ptr(){return shared_from_this();};

    void update();
    void propagate_outputs();

    // private:
    std::any get_value(std::string input_name){
      return inputTerminals[input_name]->cdata;
    }
    template<typename T> void set_value(const std::string output_name, T value){
      outputTerminals[output_name]->push(std::any(value));
    }

    virtual void process()=0;
  };


  class NodeManager {
    public:
    NodeManager(){};
    std::vector<std::shared_ptr<Node>> nodes;
    std::queue<std::shared_ptr<Node>> node_queue;

    template<class NodeClass> static std::shared_ptr<NodeClass> create_node(NodeManager& nm){
      return std::make_shared<NodeClass>(nm);
    }
    
    std::map<std::string, std::function<std::shared_ptr<Node>(NodeManager&)>> node_register;

    void run(Node &node);
    void run_node(std::shared_ptr<Node> node);
    bool check_process();
    void queue(std::shared_ptr<Node> n);
    template<class NodeClass> void register_node(std::string name){
        // node_register[name] = std::make_shared<NodeClass>;
        node_register[name] = create_node<NodeClass>;
        // auto n = std::make_shared<NodeClass>(*this);
      };
    std::weak_ptr<Node> add(std::string name){
        auto f = node_register[name];
        auto n = f(*this);
        nodes.push_back(n);
        return n;
      };
    void remove_Node(Node &Node);
    void connect(std::weak_ptr<Node> n1, std::weak_ptr<Node> n2, std::string s1, std::string s2);
  };

}