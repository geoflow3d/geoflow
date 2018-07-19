#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <any>
#include <map>
#include <set>
#include <queue>

#include <iostream>

namespace geoflow {

  class Node;
  class NodeManager;

  class Terminal {
    public:
    Terminal(Node& parent_gnode):parent(parent_gnode){};   

    Node& parent;
    std::any cdata;

    virtual void push(std::any data) = 0;
    
    bool has_data() {return cdata.has_value();};
  };

  class InputTerminal : public Terminal, public std::enable_shared_from_this<InputTerminal>{

    public:
    InputTerminal(Node& parent_gnode): Terminal(parent_gnode){};
    
    std::weak_ptr<InputTerminal> get_ptr(){
      return weak_from_this();
    }
    void push(std::any data);
    
  };
  class OutputTerminal : public Terminal, public std::enable_shared_from_this<OutputTerminal>{
    public:
    std::set<std::weak_ptr<InputTerminal>, std::owner_less<std::weak_ptr<InputTerminal>>> connections;

    
    OutputTerminal(Node& parent_gnode): Terminal(parent_gnode){};
    
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
    Node(NodeManager& manager) : manager(manager){
    };

    std::map<std::string,std::shared_ptr<InputTerminal>> inputTerminals;
    std::map<std::string,std::shared_ptr<OutputTerminal>> outputTerminals;
    struct parameters;
    node_status status=WAITING;
    // std::string name;
    NodeManager& manager;

    void add_input(std::string name){
      inputTerminals[name] = std::make_shared<InputTerminal>(
        *this
      );
    }
    void add_output(std::string name){
      outputTerminals[name] = std::make_shared<OutputTerminal>(
        *this
      );
    }

    std::shared_ptr<Node> get_ptr(){return shared_from_this();};

    // std::get_input_terminal(std::string name){
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

    void run(Node &node);
    void run_node(std::shared_ptr<Node> node);
    bool check_process();
    void queue(std::shared_ptr<Node> n);
    template<class NodeClass> std::weak_ptr<Node> add(){
        auto n = std::make_shared<NodeClass>(*this);
        nodes.push_back(n);
        return n;
      };
    void remove_Node(Node &Node);
    void connect(std::weak_ptr<Node> n1, std::weak_ptr<Node> n2, std::string s1, std::string s2);
  };

}