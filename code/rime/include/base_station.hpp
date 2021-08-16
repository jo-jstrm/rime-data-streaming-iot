#ifndef _GUARD_BASESTATIONACTOR_H
#define _GUARD_BASESTATIONACTOR_H

#include <string>
#include <vector>
#include "caf/all.hpp"
#include "node_info.hpp"
#include "query.hpp"
#include "results.hpp"

using parser_actor = caf::typed_actor<
  caf::replies_to<std::string,std::string>::with<query> >;

/**
 * Base Station for the network, interface to the user, talks directly to roots
 * Assumption: Never fails
 */
struct BaseState{
  std::string my_ip;
  int my_port;
  int node_id;

  std::vector<query> queries;
  std::vector<value> results;
  std::vector<node_info> roots;
};

caf::behavior base_station_actor(caf::stateful_actor<BaseState>*, std::string, int, int);

#endif