#ifndef _GUARD_RESULTS_H
#define _GUARD_RESULTS_H

#include <string>
#include <chrono>
#include "caf/all.hpp"

struct value{
  double result;
  int node_id;
  std::string tree_id;
  std::chrono::system_clock::time_point ts; 
    
  value();
  value(double);
  value(double, std::string, int);
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, value& x){
  return f(caf::meta::type_name("value"),
    x.result,
    x.node_id,
    x.tree_id,
    x.ts);
}

#endif