#include "query.hpp"

using namespace std;
using namespace caf;
using namespace std::chrono;

query::query() {}

query::query(string sensor_name, string aggregation, int duration, actor root, string root_ip, int root_port) :
  sensor_name(sensor_name),
  aggregation(aggregation),
  duration(duration),
  root(root),
  root_ip(root_ip),
  root_port(root_port){
    arrival_time = system_clock::now();
  }

bool query::operator==(const query& other) const{
  return sensor_name == other.sensor_name &&
          aggregation == other.aggregation &&
          duration == other.duration &&
          arrival_time == other.arrival_time &&
          root == other.root &&
          root_ip == other.root_ip &&
          root_port == other.root_port;
}


