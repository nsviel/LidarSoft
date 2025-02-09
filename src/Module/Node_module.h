#ifndef NODE_MODULE_H
#define NODE_MODULE_H

#include "../common.h"

class Node_engine;
class Node_operation;
class Node_interface;


class Node_module
{
public:
  //Constructor / Destructor
  Node_module(Node_engine* engine);
  ~Node_module();

public:
  //Main functions
  void load_module();
  void reset();
  void update();
  void runtime();
  void draw();
  void draw_online();
  void online(Cloud* cloud, int subset_ID);

  inline Node_engine* get_node_engine(){return node_engine;}
  inline Node_operation* get_node_ope(){return node_ope;}
  inline Node_interface* get_node_interface(){return node_interface;}

private:
  Node_engine* node_engine;
  Node_operation* node_ope;
  Node_interface* node_interface;
};

#endif
