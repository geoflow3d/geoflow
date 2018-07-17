#include "geoflow.hpp"
#include <iostream>

using namespace geoflow;

  // Terminal::Terminal(Node& parent_gnode){
  //   parent = parent_gnode;
  //   // std::cout << "Terminal::Terminal(), parent.expired: " << parent_gnode.expired() << "\n";
  // };

  void InputTerminal::push(std::any data){
    cdata = data;
    parent.check_inputs();
  };

  void OutputTerminal::push(std::any data){
    cdata = data;
    for (auto &c : connections){
      c.lock()->push(data);
    }

  }

  void Node::check_inputs(){
    std::cout << "Node::check_inputs()\n";
    for(auto &input : inputTerminals){
      if(!input.second->has_data()){
        std::cout << "\tDetected inputTerminal without data...\n";
        return;
        }
    }
    std::cout << "\tAll inputTerminals set, proceed to on_process()...\n";
    process();
  };


  std::weak_ptr<Node> NodeManager::add(std::shared_ptr<Node> n){
    std::weak_ptr<Node> handle = n;
    nodes.push_back(n);
    return n;
  }

  void NodeManager::connect(std::weak_ptr<Node> n1, std::weak_ptr<Node> n2, std::string s1, std::string s2) {
    n1.lock()->outputTerminals[s1]->connect(*n2.lock()->inputTerminals[s2]);
  };