#ifndef _GUARD_QUERYMANAGER_H
#define _GUARD_QUERYMANAGER_H

#include <string>
#include <vector>
#include "caf/all.hpp"
#include "query.hpp"

struct query_managerState{
  std::vector<query> queries;

  /**
   * Stores the current ranges of each sensor.
   * key = sensor name
   * value = [min, max]
   */
  std::map<std::string, std::pair<double, double>> sensor_range;
};

caf::behavior query_manager(caf::stateful_actor<query_managerState>*);

caf::behavior tracker_timer(caf::event_based_actor*);
#endif