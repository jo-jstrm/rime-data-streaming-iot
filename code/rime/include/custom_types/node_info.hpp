#ifndef _GUARD_NODEINFO_H
#define _GUARD_NODEINFO_H

#include <string>
#include <list>
#include <map>
#include <utility>
#include "caf/all.hpp"
#include "attribute.hpp"
#include "node_address.hpp"
#include "subtree_range.hpp"

//Stores meta data about a node that can be exchanged between nodes.
struct node_info : node_address {
  // Tier within the fog topology
  int tier;
  
  // Names of the sensors and their local values
  std::map<std::string, attribute> sensors;

  // subtree ranges for all trees
  // key = tree_id
  // value = subtree range
  std::map<std::string, subtree_range> subtree_ranges;

  bool operator==(const node_info&) const;
  bool operator!=(const node_info&) const;

  node_info();  
  node_info(caf::actor handle, std::string ip, int port, int id, int tier);  
  node_info(caf::actor handle, std::string ip, int port, int id, int tier, 
            std::map<std::string, attribute> sensors);
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, node_info& x){
  return f(caf::meta::type_name("node_info"),
    x.handle,
    x.ip,
    x.port,
    x.node_id,
    x.tier,
    x.sensors,
    x.subtree_ranges);
}

#endif