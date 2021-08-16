#ifndef _GUARD_SRTNODE_H
#define _GUARD_SRTNODE_H

#include <list>
#include <string>
#include <map>
#include <vector>
#include "caf/all.hpp"
#include "attribute.hpp"
#include "node_info.hpp"
#include "subtree_range.hpp"

struct node_state {
  std::string tree_id;  
  // Attribute over which this tree is built.
  std::string tree_attr;

  node_info parent;
  subtree_range parent_subtree_range;
  double distance_to_parent;
  bool parent_valid;
  
  node_info backup_parent;
  subtree_range backup_subtree_range;
  double distance_to_backup;
  bool backup_valid;
  
  std::list<node_info> children;  
  
  // Own level in this tree.
  int my_lvl;

  // Local attribute value
  attribute local_att_value;

  // Stores this node's subtree range (which includes own range) for this tree.
  subtree_range my_subtree_range;  

  // Indicates that the tree was initialized successfully
  // Used to start parent selection etc.  
  bool collected_once;

  // The sensor that routes its data through this tree
  caf::actor sensor;

  int sampling_interval;
};

caf::behavior srt_node(caf::stateful_actor<node_state>*);

#endif