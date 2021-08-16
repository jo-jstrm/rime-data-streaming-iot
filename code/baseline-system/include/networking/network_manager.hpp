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
  caf::actor parent;

  //Stores information about self. This will be exchanged with other nodes. 
  node_info my_info;  
  
  caf::actor sensor;
  int sampling_interval;
  
  network_state();
};

bool comp_attr(const std::pair<node_info, int64_t>&, 
                const std::pair<node_info, int64_t>&, 
                const double&, 
                const std::string&);

caf::behavior network_manager(caf::stateful_actor<network_state>*, net_init);

#endif