#include<vector>
#include<memory>

namespace geoflow {

  enum node_status {
    NO_OUTPUT,
    WAITING,
    PROCESSING,
    IDLE
  };
  class Node {
    struct inputs;
    struct outputs;
    struct parameters;

    node_status status;

    void check_inputs();

    void process();

    virtual void on_process()=0;
  };

  enum connection_status {
    EMPTY,
    WAITING,
    CONSTANT,
    SET
  };
  template<typename T> class Connection {
    Node & source, target;
    connection_status status;

    std::shared_ptr<T> data_ptr;
    void set_data(T data);
    void set_status(connection_status status);
    
  };

  class NodeManager {
    std::vector<Node> nodes;
    std::vector<Connection> connections;

    void process_from_node(Node &node);
    void add_node(Node &node);
    void remove_node(Node &node);
    void add_connection(Connection &conn);
    void remove_connection(Connection &conn);
  };

}