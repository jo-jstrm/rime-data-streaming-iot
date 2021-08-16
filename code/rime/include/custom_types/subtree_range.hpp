#ifndef _GUARD_ATTRIBUTEMETA_H
#define _GUARD_ATTRIBUTEMETA_H

#include <string>
#include "caf/all.hpp"

//Stores meta data about the attribute "tree_id"
struct subtree_range{
  std::string tree_id;

  double lat_min;
  double lat_max;
  
  double lon_min;  
  double lon_max;

  bool operator==(const subtree_range&) const;

  subtree_range();
  subtree_range(std::string);  
  subtree_range(std::string, double, double, double, double);
};
template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, subtree_range& x){
  return f(caf::meta::type_name("subtree_range"),
    x.tree_id,
    x.lat_min,
    x.lat_max,
    x.lon_min,
    x.lon_max);    
}
#endif