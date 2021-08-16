#ifndef _GUARD_SENSORSTATE_H
#define _GUARD_SENSORSTATE_H

#include <string>
#include "caf/all.hpp"

struct sensor_state {
  std::string name;  

  bool operator==(const sensor_state&) const;

  sensor_state();
  sensor_state(std::string name);
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, sensor_state& x){
  return f(caf::meta::type_name("sensor_state"),
    x.name);    
}
#endif