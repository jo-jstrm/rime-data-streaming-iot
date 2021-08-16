#ifndef _GUARD_SENSORS_H
#define _GUARD_SENSORS_H

#include <string> 
#include "caf/all.hpp"
#include "sensor_state.hpp"

//Sensor that periodically produces random values
caf::behavior random_sensor(caf::stateful_actor<sensor_state>*, std::string name);

#endif