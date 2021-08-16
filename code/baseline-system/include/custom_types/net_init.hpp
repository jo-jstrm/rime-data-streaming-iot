#ifndef _GUARD_NETINIT_H
#define _GUARD_NETINIT_H

class srt_config;

#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "caf/all.hpp"
#include "attribute.hpp"

//Derived from contents of srt_config. Needed for serialization, because srt_config is not serializable.
struct net_init {
  std::string base_ip;
  int base_port;
  bool am_first_root;

  std::string relative_ip;
  int relative_port;

  int node_id;
  std::string my_ip;
  int my_port;  
  int tier;

  int sampling_interval;

  int experiment_duration;
  
  net_init(const srt_config*);  
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, net_init& x){
  return f(caf::meta::type_name("net_init"),
    x.base_ip,
    x.base_port,
    x.am_first_root,
    x.relative_ip,
    x.relative_port,
    x.node_id,
    x.my_ip,
    x.my_port,
    x.tier,  
    x.sampling_interval,
    x.experiment_duration);
}
#endif