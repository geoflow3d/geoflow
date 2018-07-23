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
  }
  void InputTerminal::clear() {
    cdata.reset();
    parent.status = WAITING;
  }

  OutputTerminal::~OutputTerminal(){
    for(auto& conn : connections) {
      if (!conn.expired()) {
        auto in = conn.lock();
        in->clear();
        in->parent.notify_children();
      }
    }
  }
  void OutputTerminal::push(std::any data) {
    cdata = data;
  }
  void OutputTerminal::connect(InputTerminal& in) { 
    std::cout << "OutputTerminal.connect\n";
    std::cout << "\t parent:" << parent.name << "\n";
    std::cout << "\t &this terminal:" << this << "\n";
    std::cout << "\t &input terminal:" << &in << "\n";
    connections.insert(in.get_ptr());
    if (has_data()) {
      in.push(cdata);
    }
    std::cout << "\t #connections:" << connections.size() << "\n";
  }
  void OutputTerminal::disconnect(InputTerminal& in) { 
    std::cout << "OutputTerminal.disconnect\n";
    std::cout << "\t parent:" << parent.name << "\n";
    std::cout << "\t &this terminal:" << this << "\n";
    std::cout << "\t &input terminal:" << &in << "\n";
    connections.erase(in.get_ptr());
    // clear data to enforce a new connection must be made to this input terminal before the parent node can use it
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
          conn = oT.second->connections.erase(conn); // if the terminal on the other end no longer exist, remove this connection
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
        status = WAITING;
        return false;
        }
    }
    std::cout << "\tAll inputTerminals set, node is READY...\n";
    manager.queue(get_ptr());
    status = READY;
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
      n->status = PROCESSING;
      n->process();
      n->status = DONE;
      n->propagate_outputs();
    }
    return 1;
  }

  bool NodeManager::run(Node &node) {
    std::queue<std::shared_ptr<Node>>().swap(node_queue); // clear to prevent double processing of nodes ()
    if (node.update()) {
      node.notify_children();
      check_process();
      return true;
    } 
    return false;
  }

  void Node::notify_children() {
    std::cout << "Node::notify_children begin\n";
    for (auto& oT : outputTerminals) {
      for (auto conn = oT.second->connections.begin(); conn != oT.second->connections.end();){
        std::cout << oT.second->connections.size() << "\n";
        if (conn->expired()) {
          std::cout << "expired connection\n";
          conn = oT.second->connections.erase(conn); // if the terminal on the other end no longer exist, remove this connection
          std::cout << "...resolved\n";
        } else {
          auto c = conn->lock();
          c->wait_for_update = true;
          c->clear();
          c->parent.notify_children();
          ++conn;
        }
      }
    }
    std::cout << "Node::notify_children end\n";
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