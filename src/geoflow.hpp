#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <any>
#include <map>
#include <set>

#include <iostream>

namespace geoflow {

  class Node;

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
    
    std::set<std::weak_ptr<InputTerminal>, std::owner_less<std::weak_ptr<InputTerminal>>> connections;

    public:
    OutputTerminal(Node& parent_gnode): Terminal(parent_gnode){};
    
    std::weak_ptr<OutputTerminal> get_ptr(){
      return weak_from_this();
    }

    void connect(InputTerminal& in) {
      connections.insert(in.get_ptr());
    }
    
    void push(std::any data);
  };

  // enum node_status {
  //   NO_OUTPUT,
  //   WAITING,
  //   PROCESSING,
  //   IDLE
  // };
  class Node {
    public:
    std::map<std::string,std::shared_ptr<InputTerminal>> inputTerminals;
    std::map<std::string,std::shared_ptr<OutputTerminal>> outputTerminals;
    struct parameters;

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

    // std::get_input_terminal(std::string name){
    void check_inputs();

    // private:
    std::any get_value(std::string input_name){
      return inputTerminals[input_name]->cdata;
    }
    template<typename T> void set_value(const std::string output_name, T value){
      outputTerminals[output_name]->push(std::any(value));
    }

    // virtual void define_terminals()=0;
    virtual void process()=0;
  };


  class NodeManager {
    public:
    NodeManager(){};
    std::vector<std::shared_ptr<Node>> nodes;

    void process_from_Node(Node &Node);
    std::weak_ptr<Node> add(std::shared_ptr<Node>);
    void remove_Node(Node &Node);
    void connect(std::weak_ptr<Node> n1, std::weak_ptr<Node> n2, std::string s1, std::string s2);
    // void remove_connection(Connection &conn);
  };

}