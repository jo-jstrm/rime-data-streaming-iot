#ifndef _GUARD_NETWORKMANAGER_H
#define _GUARD_NETWORKMANAGER_H

struct net_init;

#include <deque>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include "caf/all.hpp"
#include "node_address.hpp"
#include "node_info.hpp"

struct network_state {
  // Base also serves as fallback address
  std::string base_ip;
  int base_port;
  
  // Stores information about self. This will be exchanged with other nodes. 
  node_info my_info;

  // Nodes that self publishes updates to
  std::list<std::pair<caf::actor, int>> subscribers;

  /**
   * Stores information about each tree.  
   * key = tree id 
   * value = corresponding handle for local tree actor
   */
  std::map<std::string, caf::actor> trees;

  /**
   * Roots store their bases for each tree here.  
   * key = tree id 
   * value = corresponding handle for corresponding base
   */
  std::map<std::string, node_address> bases;

  // Part of each tree`s id, initialized randomly and then incremented with each id creation
  // See default constructor
  int tree_id_counter;

  std::deque< std::pair<int, int> > subtree;
  int print_counter;
  
  int sampling_interval;

  bool start_moving;
  
  bool log_tuples;
  bool log_for_failures;
  
  node_info random_neighbor;
  
  network_state();
};

bool comp_attr(const std::pair<node_info, int64_t>&, 
                const std::pair<node_info, int64_t>&, 
                const double&, 
                const std::string&);

caf::behavior network_manager(caf::stateful_actor<network_state>*, net_init);

#endif