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
    node_queue.push_back(n);
  }
  void NodeManager::connect(std::weak_ptr<Node> n1, std::weak_ptr<Node> n2, std::string s1, std::string s2) {
    n1.lock()->outputTerminals[s1]->connect(*n2.lock()->inputTerminals[s2]);
  }

  bool NodeManager::check_process(){
    std::cout << "executing check_process, node_queue.size()=" << node_queue.size() << "\n";
    //https://stackoverflow.com/questions/9927163/erase-element-in-vector-while-iterating-the-same-vector
    for (auto it = node_queue.begin(); it != node_queue.end();){
      if ((*it)->status==DONE) {
        std::cout << "\tfound a DONE node\n";
        (*it)->propagate_outputs();
        node_queue.erase(it);
      } else {
        it++;
      }
    }
    for (auto it = node_queue.begin(); it != node_queue.end();){
      if ((*it)->status==WAITING) {
        std::cout << "\tfound a WAITING node\n";
        auto t = std::thread(&NodeManager::run_node, this, std::ref(*it));
        t.detach();
        // node_queue.push_back(*it);
      }
    }
    return node_queue.size()==0;\
  }

  void NodeManager::run(Node &node){
    std::unique_lock<std::mutex> mlock(mutex);

    queue(node.get_ptr());
    check_process();
    // auto t = std::thread(&NodeManager::run_node, this, node.get_ptr());
    // t.detach();

    cv.wait(mlock, std::bind(&NodeManager::check_process, this));
  }

  void NodeManager::run_node(std::shared_ptr<Node> node){
    node->status = PROCESSING;
    node->process();
    
    node->status = DONE;
    std::cout << "\tWTF\n";
    std::lock_guard<std::mutex> guard(mutex);
    // cv.notify_one();
    
  }