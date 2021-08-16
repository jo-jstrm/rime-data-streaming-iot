#include "sensor_state_location.hpp"

#include <string>

using namespace std;

sensor_state_location::sensor_state_location() :
  name("default"),
  lat(0.0),
  lon(0.0) {}

sensor_state_location::sensor_state_location(string name) : 
  name(name),
  lat(0.0),
  lon(0.0) {}

sensor_state_location::sensor_state_location(string name, double lat, double lon) : 
  name(name),
  lat(lat),
  lon(lon) {}

bool sensor_state_location::operator==(const sensor_state_location& other) const {
  return name == other.name &&
          lat == other.lat &&
          lon == other.lon;
}