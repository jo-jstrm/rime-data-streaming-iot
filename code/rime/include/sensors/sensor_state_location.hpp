#ifndef _GUARD_SENSORSTATELOCATION_H
#define _GUARD_SENSORSTATELOCATION_H

#include <string>
#include "caf/all.hpp"

struct sensor_state_location {
  std::string name;
  double lat;
  double lon; 

  bool operator==(const sensor_state_location&) const;

  sensor_state_location();
  sensor_state_location(std::string name);
  sensor_state_location(std::string name, double, double);
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, sensor_state_location& x) {
  return f(caf::meta::type_name("sensor_state_location"),
    x.name,
    x.lat,
    x.lon);    
}
#endif