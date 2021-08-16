#ifndef _GUARD_SRTCONFIG_H
#define _GUARD_SRTCONFIG_H

#include <string>
#include <list>
#include "caf/all.hpp"
#include "sensor_state.hpp"

class srt_config : public caf::actor_system_config{
public:    
  /***** Network Config *****/
  std::string base_ip;
  int base_port;
  bool base_station_mode;
  bool am_first_root;

  std::string relative_ip;
  int relative_port;

  int node_id;
  std::string my_ip;
  int my_port;  
  int tier;
  
  /**** sensors ****/
  std::list<std::string> sensors;  
  std::list<double> initial_values;

  int sampling_interval; // in [ms]
  
  // Indicates that the hardcoded query should be executed
  bool send_query;
  double query_x;
  double query_y;

  // Indicates that this node should start moving during runtime
  bool start_moving;

  bool log_tuples;
  bool log_for_failures;
  
  int experiment_duration;

  srt_config();
  srt_config(int my_port, int relative_port);
};

#endif