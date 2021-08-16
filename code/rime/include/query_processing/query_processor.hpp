#ifndef _GUARD_QUERYPROCESSOR_H
#define _GUARD_QUERYPROCESSOR_H

#include <deque>
#include <map>
#include <utility>
#include <string>
#include <vector>
#include "caf/all.hpp"
#include "subtree_range.hpp"
#include "results.hpp"
#include "sensor_state.hpp"

struct ProcessorState {
  /**
   * Stores latitude and longitude
   */  
  std::pair<double, double> location;
  
  /**
   * Local sensor readings
   * key = sensor name
   * value = deque of readings
   */
  std::map<std::string, std::deque<double>> readings;  
  
  /**
   * min and max for each sensor
   * key = sensor name
   * value.first = min
   * value.second = max
   */
  std::map<std::string, std::pair<double, double>> sensor_meta;  
  
  ProcessorState();
};

// Class-based (only for the sake of practicing), event-driven, stateful actor
caf::behavior query_processor(caf::stateful_actor<ProcessorState>*, caf::actor, std::vector<std::string>);
#endif