#include "find_node_info.hpp"

#include "node_info.hpp"

using namespace std;
using namespace caf;

find_relative::find_relative(actor handle) : handle(handle) {}

bool find_relative::operator()(node_info const& info) const {
  return info.handle == handle;
}

find_rel_by_ip_port::find_rel_by_ip_port(string ip, int port) : 
  relative_ip(ip), 
  relative_port(port) {}

bool find_rel_by_ip_port::operator()(node_info const& info) const {
  return info.ip == relative_ip && info.port == relative_port;
}