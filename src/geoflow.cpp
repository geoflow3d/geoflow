#include "geoflow.hpp"
#include <iostream>

using namespace geoflow;

  // Terminal::Terminal(Node& parent_gnode) {
  //   parent = parent_gnode;
  //   // std::cout << "Terminal::Terminal(), parent.expired: " << parent_gnode.expired() << "\n";
  // };

  void InputTerminal::push(std::any data) {
    cdata = data;
    // wait_for_update = false;
    parent.update();
  }
  void InputTerminal::clear() {
    cdata.reset();
    parent.status = WAITING;
    parent.notify_children();
  }

  OutputTerminal::~OutputTerminal() {
    for(auto& conn : connections) {
      if (!conn.expired()) {
        auto in = conn.lock();
        in->clear();
      }
    }
  }
  void OutputTerminal::push(std::any data) {
    cdata = data;
  }
  void OutputTerminal::clear() {
    cdata.reset();
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
    in.clear();
    // clear data to enforce a new connection must be made to this input terminal before the parent node can use it
    std::cout << "\t #connections:" << connections.size() << "\n";
  }

  void Node::add_input(std::string name, TerminalType type) {
    inputTerminals[name] = std::make_shared<InputTerminal>(
      *this, type
    );
  }
  void Node::add_output(std::string name, TerminalType type) {
    outputTerminals[name] = std::make_shared<OutputTerminal>(
      *this, type
    );
  }
  bool Node::update() {
    std::cout << "Node::update()\n";
    for(auto &input : inputTerminals) {
      if(!input.second->has_data()) {
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
  void Node::propagate_outputs() {
    std::cout << "propagating outputs...\n";
    for (auto &oT : outputTerminals) {
      std::cout << "\tterm ["<< oT.second << "] [" << oT.second->connections.size() <<" connections]\n";
      for (auto conn = oT.second->connections.begin(); conn != oT.second->connections.end();) {
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
  void Node::notify_children() {
    std::cout << "Node::notify_children begin\n";
    for (auto& oT : outputTerminals) {
      oT.second->clear(); // clear output terminal
      for (auto conn = oT.second->connections.begin(); conn != oT.second->connections.end();) {
        std::cout << oT.second->connections.size() << "\n";
        if (conn->expired()) {
          std::cout << "expired connection\n";
          conn = oT.second->connections.erase(conn); // if the terminal on the other end no longer exist, remove this connection
          std::cout << "...resolved\n";
        } else {
          auto c = conn->lock();          
          // c->wait_for_update = true;
          c->clear(); // note: clear calls again notify_children for the child node
          ++conn;
        }
      }
    }
    std::cout << "Node::notify_children end\n";
  }
  std::string Node::get_info() {
    std::stringstream s;
    s << "status: ";
    switch (status) {
      case WAITING: s << "WAITING"; break;
      case READY: s << "READY"; break;
      case PROCESSING: s << "PROCESSING"; break;
      case DONE: s << "DONE"; break;
      case ERROR: s << "ERROR"; break;
      default: s << "UNKNOWN"; break;
    }
    s << "\n";
    return s.str();
  }

  std::shared_ptr<Node> NodeManager::create(std::string name) {
    auto f = node_register[name];
    auto n = f(*this);
    // nodes.push_back(n);
    return n;
  }
  void NodeManager::queue(std::shared_ptr<Node> n) {
    node_queue.push(n);
  }
  bool NodeManager::check_process() {
    while (!node_queue.empty()) {
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
  void NodeManager::run_node(std::shared_ptr<Node> node) {
    node->status = PROCESSING;
    node->process();
    node->status = DONE;
  }

  bool geoflow::connect(Node& n1, Node& n2, std::string s1, std::string s2) {
    auto oT = n1.outputTerminals[s1].get();
    auto iT = n2.inputTerminals[s2].get();
    if (detect_loop(oT, iT))
      return false;
    oT->connect(*iT);
    return true;
  }
  bool geoflow::connect(Terminal* t1, Terminal* t2) {
    auto oT = dynamic_cast<OutputTerminal*>(t1);
    auto iT = dynamic_cast<InputTerminal*>(t2);
    if (detect_loop(oT, iT))
      return false;
    oT->connect(*iT);
    return true;
  }
  void geoflow::disconnect(Terminal* t1, Terminal* t2) {
    auto oT = dynamic_cast<OutputTerminal*>(t1);
    auto iT = dynamic_cast<InputTerminal*>(t2);
    oT->disconnect(*iT);
  }
  bool geoflow::detect_loop(Terminal* t1, Terminal* t2) {
    auto oT = dynamic_cast<OutputTerminal*>(t1);
    auto iT = dynamic_cast<InputTerminal*>(t2);
    return detect_loop(oT, iT);
  }
  bool geoflow::detect_loop(OutputTerminal* outputT, InputTerminal* inputT) {
    auto node = inputT->parent.get_ptr();
    std::queue<std::shared_ptr<Node>> nodes_to_check;
    std::set<std::shared_ptr<Node>> visited;
    nodes_to_check.push(node);
    // visited.insert(node);
    while (!nodes_to_check.empty()) {
      auto n = nodes_to_check.front();
      nodes_to_check.pop();
      
      for (auto& oT : n->outputTerminals) {
        if (oT.second.get() == outputT){
          return true;
        }
        for (auto& c :oT.second->connections) {
          if (!c.expired()) {
            auto iT = c.lock();
            auto child_node = iT->parent.get_ptr();
            if (visited.count(child_node)==0) {
              visited.insert(child_node);
              nodes_to_check.push(child_node);
            }
          }
        }
      }
    }
    return false;
  }