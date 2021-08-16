#include "logger.hpp"

#include <chrono>

#define CAF_LOG_LEVEL CAF_LOG_LEVEL_ERROR
#define CAF_LOG_COMPONENT "Logger"

using namespace std;
using namespace caf;

using write_logs = atom_constant<atom("writelog")>;
using print_tree = atom_constant<atom("printtree")>;
using tuples = atom_constant<atom("tuples")>;

rime::logger_state::logger_state() {  
  logs_ptr = new vector<string>;
  topo_ptr = new vector<string>;
  tupl_ptr = new vector<string>;
};

behavior rime::logger(stateful_actor<rime::logger_state>* self, int experiment_duration) {
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "LOGGER STARTUP");
  
  self->state.experiment_duration = experiment_duration;
  self->delayed_send(actor_cast<actor>(self), chrono::seconds(experiment_duration), write_logs::value);
  return {
    [=](string log) {
      self->state.logs_ptr->push_back(log);
    },
    [=](print_tree, string log) {
      self->state.topo_ptr->push_back(log);
    },
    [=](tuples, string log) {
      self->state.tupl_ptr->push_back(log);
    },
    [=](write_logs) {
      // Will put a heavy load on disk I/O. The experiment should be over before you use this.
      for(const string& log : *(self->state.tupl_ptr)) {
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, log);
      }
      for(const string& log : *(self->state.logs_ptr)) {
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, log);
      }
      for(const string& log : *(self->state.topo_ptr)) {
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, log);
      }
    },
  };
}