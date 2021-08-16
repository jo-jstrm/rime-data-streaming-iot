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

  int sampling_interval; // in [ms]

  int experiment_duration; // in [s]

  srt_config();
  srt_config(int my_port, int relative_port);
};

#endif