#include "sensor_state.hpp"

#include <string>

using namespace std;

sensor_state::sensor_state() : name("default") {}
sensor_state::sensor_state(string name) : 
  name(name) {}

bool sensor_state::operator==(const sensor_state& other) const {
  return name == other.name;
}