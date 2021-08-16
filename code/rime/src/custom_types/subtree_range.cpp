#include "subtree_range.hpp"

using namespace std;
using namespace caf;

subtree_range::subtree_range() : 
  tree_id("default"),
  lat_min(99999.9),
  lat_max(0.0),
  lon_min(99999.9),
  lon_max(0.0) {}

subtree_range::subtree_range(string attrName) : subtree_range() {
  tree_id = attrName;
}

subtree_range::subtree_range(string tree_id, double lat_min, double lat_max, double lon_min, double lon_max) :
  tree_id(tree_id),
  lat_min(lat_min),
  lat_max(lat_max),
  lon_min(lon_min),
  lon_max(lon_max) {}

bool subtree_range::operator==(const subtree_range& other) const{
  return tree_id == other.tree_id &&
          lat_min == other.lat_min &&
          lat_max == other.lat_max &&
          lon_min == other.lon_min &&
          lon_max == other.lon_max;          
}