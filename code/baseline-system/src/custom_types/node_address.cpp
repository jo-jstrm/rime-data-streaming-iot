#include "node_address.hpp"

using namespace std;
using namespace caf;

node_address::node_address() {}
node_address::node_address(actor handle, string ip, int port, int id) : 
  handle(handle),
  ip(ip),
  port(port),
  node_id(id) {}

bool node_address::operator==(const node_address& other) const {
  return handle == other.handle &&
          ip == other.ip &&
          port == other.port &&
          node_id == other.node_id;
}

bool node_address::operator!=(const node_address& other) const {
  return handle != other.handle ||
          ip != other.ip ||
          port != other.port ||
          node_id != other.node_id;
}
          