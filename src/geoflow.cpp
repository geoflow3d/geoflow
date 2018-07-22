#include "geoflow.hpp"
#include <iostream>

using namespace geoflow;

  // Terminal::Terminal(Node& parent_gnode){
  //   parent = parent_gnode;
  //   // std::cout << "Terminal::Terminal(), parent.expired: " << parent_gnode.expired() << "\n";
  // };

  void InputTerminal::push(std::any data) {
    cdata = data;
    wait_for_update = false;
    parent.update();
  };

  void OutputTerminal::push(std::any data) {
    cdata = data;
  }
  void OutputTerminal::connect(InputTerminal& in) { 
    std::cout << "OutputTerminal.connect\n";
    std::cout << "\t parent:" << parent.name << "\n";
    std::cout << "\t &this terminal:" << this << "\n";
    std::cout << "\t &input terminal:" << &in << "\n";
    connections.insert(in.get_ptr());
    std::cout << "\t #connections:" << connections.size() << "\n";
  }
  void OutputTerminal::disconnect(InputTerminal& in) { 
    std::cout << "OutputTerminal.disconnect\n";
    std::cout << "\t parent:" << parent.name << "\n";
    std::cout << "\t &this terminal:" << this << "\n";
    std::cout << "\t &input terminal:" << &in << "\n";
    auto it = connections.find(in.get_ptr());
    connections.erase(it);
    if(!it->expired())
      it->lock()->cdata.reset();// clear data to enforce a new connection must be made to this input terminal before the parent node can use it
    std::cout << "\t #connections:" << connections.size() << "\n";
  }

  void Node::propagate_outputs() {
    std::cout << "propagating outputs...\n";
    for (auto &oT : outputTerminals) {
      std::cout << "\tterm ["<< oT.second << "] [" << oT.second->connections.size() <<" connections]\n";
      for (auto conn = oT.second->connections.begin(); conn != oT.second->connections.end();){
        std::cout << "\t\tconnection...";
        if (conn->expired()) {
          std::cout << "expired\n";
          oT.second->connections.erase(conn); // if the terminal on the other end no longer exist, remove this connection
        } else {
          std::cout << "pushing data\n";
          conn->lock()->push(oT.second->cdata); // conn is the inputTerminal on the other end of the connection
          ++conn;
        }
      }
    }
  }

  bool Node::update() {
    std::cout << "Node::update()\n";
    for(auto &input : inputTerminals) {
      if(!input.second->has_data() || input.second->wait_for_update) {
        std::cout << "\tDetected inputTerminal that is not ready...\n";
        return false;
        }
    }
    std::cout << "\tAll inputTerminals set, proceed to on_process()...\n";
    manager.queue(get_ptr());
    return true;
  };


  void NodeManager::queue(std::shared_ptr<Node> n) {
    node_queue.push(n);
  }
  // void NodeManager::connect(InputTerminal& t1, Terminal& t2) {
  //   t1.connect(t2);
  // }

  bool NodeManager::check_process(){
    while (!node_queue.empty()){
      auto n = node_queue.front();
      node_queue.pop();
      n->process();
      n->propagate_outputs();
    }
    return 1;
  }

  bool NodeManager::run(Node &node) {
    if (node.update()) {
      notify_children(node);
      check_process();
      return true;
    } 
    return false;
  }

  void NodeManager::notify_children(Node &node) {
    std::queue<std::shared_ptr<Node>> nodes_to_notify;
    std::set<std::shared_ptr<Node>> visited;
    nodes_to_notify.push(node.get_ptr());
    visited.insert(node.get_ptr());
    while(!nodes_to_notify.empty()) {
      auto n = nodes_to_notify.front();
      nodes_to_notify.pop();
      
      for (auto& oT : n->outputTerminals) {
        for (auto& c :oT.second->connections) {
          auto iT = c.lock();
          iT->wait_for_update = true;
          auto child_node = iT->parent.get_ptr();
          if (visited.count(child_node)==0) {
            visited.insert(child_node);
            nodes_to_notify.push(child_node);
          }
        }
      }
    }
  }

  void NodeManager::run_node(std::shared_ptr<Node> node){
    node->status = PROCESSING;
    node->process();
    node->status = DONE;
  }

  void geoflow::connect(Node& n1, Node& n2, std::string s1, std::string s2){
    n1.outputTerminals[s1]->connect(*n2.inputTerminals[s2]);
  };
  void geoflow::connect(Terminal* t1, Terminal* t2){
    auto oT = dynamic_cast<OutputTerminal*>(t1);
    auto iT = dynamic_cast<InputTerminal*>(t2);
    oT->connect(*iT);
  };
  void geoflow::disconnect(Terminal* t1, Terminal* t2){
    auto oT = dynamic_cast<OutputTerminal*>(t1);
    auto iT = dynamic_cast<InputTerminal*>(t2);
    oT->disconnect(*iT);
  }