#ifndef _GUARD_NODEADDRESS_H
#define _GUARD_NODEADDRESS_H

#include <string>
#include "caf/all.hpp"

struct node_address {
  caf::actor handle;
  std::string ip;
  int port;
  int node_id;

  bool operator==(const node_address&) const;
  bool operator!=(const node_address&) const;

  node_address();
  node_address(caf::actor, std::string, int, int);
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, node_address& x){
  return f(caf::meta::type_name("node_address"),
    x.handle,
    x.ip,
    x.port,
    x.node_id);
}

#endif