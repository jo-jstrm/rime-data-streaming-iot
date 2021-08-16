#ifndef _GUARD_CONSTATTRIBUTE_H
#define _GUARD_CONSTATTRIBUTE_H

#include <string>
#include "caf/all.hpp"

struct attribute {
  std::string name;
  double x;
  double y;

  bool operator==(const attribute&) const;
  bool operator!=(const attribute&) const;

  attribute();
  attribute(std::string, double, double);
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, attribute& z){
  return f(caf::meta::type_name("attribute"),    
    z.name,
    z.x,
    z.y);
}

#endif