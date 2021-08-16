#ifndef _GUARD_LOCATIONSENSOR_H
#define _GUARD_LOCATIONSENSOR_H

#include <string> 
#include "caf/all.hpp"
#include "sensor_state_location.hpp"

// Sensor that periodically produces random values in a specified interval 
caf::behavior location_sensor(caf::stateful_actor<sensor_state_location>*);

#endif