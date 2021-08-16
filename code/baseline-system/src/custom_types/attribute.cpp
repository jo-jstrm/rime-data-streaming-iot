#include "attribute.hpp"

using namespace std;

attribute::attribute() : 
  name("loc"),
  x(0.0),
  y(0.0) {}

attribute::attribute(string name, double x, double y) : 
  name(name),
  x(x),
  y(y) {}

  bool attribute::operator==(const attribute& other) const {
  return name == other.name &&
          x == other.x &&
          y == other.y;
}

bool attribute::operator!=(const attribute& other) const {
  return name != other.name ||
          x != other.x ||
          y != other.y;
}