#include "geoflow.hpp"
#include <iostream>

using namespace geoflow;

  // Terminal::Terminal(Node& parent_gnode){
  //   parent = parent_gnode;
  //   // std::cout << "Terminal::Terminal(), parent.expired: " << parent_gnode.expired() << "\n";
  // };

  void InputTerminal::push(std::any data){
    cdata = data;
    parent.update();
  };

  void OutputTerminal::push(std::any data){
    cdata = data;
  }

  void Node::propagate_outputs(){
    for (auto &oT : outputTerminals)
      for (auto &c : oT.second->connections){
        c.lock()->push(oT.second->cdata); // c is the inputTerminal on the other end of the connection
      }
  }

  void Node::update(){
    std::cout << "Node::update()\n";
    for(auto &input : inputTerminals){
      if(!input.second->has_data()){
        std::cout << "\tDetected inputTerminal without data...\n";
        return;
        }
    }
    std::cout << "\tAll inputTerminals set, proceed to on_process()...\n";
    manager.queue(get_ptr());
  };


  void NodeManager::queue(std::shared_ptr<Node> n) {
    node_queue.push(n);
  }
  void NodeManager::connect(std::weak_ptr<Node> n1, std::weak_ptr<Node> n2, std::string s1, std::string s2) {
    n1.lock()->outputTerminals[s1]->connect(*n2.lock()->inputTerminals[s2]);
  }

  bool NodeManager::check_process(){
    while (!node_queue.empty()){
      auto n = node_queue.front();
      node_queue.pop();
      n->process();
      n->propagate_outputs();
    }
    return 1;
  }

  void NodeManager::run(Node &node){
    queue(node.get_ptr());
    check_process();
  }

  void NodeManager::run_node(std::shared_ptr<Node> node){
    node->status = PROCESSING;
    node->process();
    node->status = DONE;
  }