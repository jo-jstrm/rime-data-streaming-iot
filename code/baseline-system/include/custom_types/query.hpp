#ifndef _GUARD_CUSTOMTYPES_H
#define _GUARD_CUSTOMTYPES_H

#include <string>
#include "caf/all.hpp"

struct query{
  //Attribute/sensor to be queried.
  std::string sensor_name;

  //Type of query. Possibilities: {'range', 'val', 'min', 'max', 'avg'}
  std::string aggregation;

  /**
   * Duration of the query. Possibilities: 
   * 'time' translates to [0,1,2,...]
   *  ad-hoc query for duration=0
   */
  int duration;

  //Time that the query was injected at. Also unique id.
  std::chrono::system_clock::time_point arrival_time;

  //Source of this query and data sink
  caf::actor root;
  std::string root_ip;
  int root_port;

  bool operator==(const query&) const;

  query();
  query(std::string sensor_name,
    std::string aggregation,
    int duration,
    caf::actor root,
    std::string root_ip,
    int root_port);
};
template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, query& x){
  return f(caf::meta::type_name("query"),
    x.sensor_name,
    x.aggregation,
    x.duration,
    x.root_ip,
    x.root_port);
}
#endif