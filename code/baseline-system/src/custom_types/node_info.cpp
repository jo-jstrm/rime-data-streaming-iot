#include "node_info.hpp"

using namespace std;
using namespace caf;

node_info::node_info() : 
  node_address(),
  tier(1) {    
    this->sensors["loc"] = attribute("loc", 0.0, 0.0);
  }

node_info::node_info(actor handle, string ip, int port,int id, int t) : 
  node_address(handle, ip, port, id),
  tier(t) {    
    this->sensors["loc"] = attribute("loc", 0.0, 0.0);    
  }

node_info::node_info(actor handle, string ip, int port,int id, int t, map<string, attribute> sensors) : 
  node_address(handle, ip, port, id),
  tier(t),
  sensors(sensors) {}

bool node_info::operator==(const node_info& other) const {
  return handle == other.handle &&
          ip == other.ip &&
          port == other.port &&
          node_id == other.node_id &&
          tier == other.tier &&
          sensors == other.sensors;
}

bool node_info::operator!=(const node_info& other) const {
  return handle != other.handle ||
          ip != other.ip ||
          port != other.port ||
          node_id != other.node_id ||
          tier != other.tier ||
          sensors != other.sensors;
}