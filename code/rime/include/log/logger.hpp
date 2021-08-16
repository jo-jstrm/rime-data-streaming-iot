#ifndef _GUARD_LOGGER_H
#define _GUARD_LOGGER_H

#include "caf/all.hpp"

#include <chrono>
#include <string>
#include <vector>

namespace rime {
  
struct logger_state {
  int experiment_duration;
  std::vector<std::string>* logs_ptr;
  std::vector<std::string>* topo_ptr;
  std::vector<std::string>* tupl_ptr;
  
  logger_state();
};

// This actor stores all messages in *logs_ptr and writes them after all benchmarks have run.
caf::behavior logger(caf::stateful_actor<rime::logger_state>*, int);

} // namespace
#endif