#include "net_init.hpp"

#include "srt_config.hpp"

net_init::net_init(const srt_config* cfg):
  base_ip(cfg->base_ip),
  base_port(cfg->base_port),
  am_first_root(cfg->am_first_root),
  relative_ip(cfg->relative_ip),
  relative_port(cfg->relative_port),
  node_id(cfg->node_id),
  my_ip(cfg->my_ip),
  my_port(cfg->my_port),  
  tier(cfg->tier),
  sampling_interval(cfg->sampling_interval),
  start_moving(cfg->start_moving),  
  initial_values(cfg->initial_values),
  log_tuples(cfg->log_tuples),
  log_for_failures(cfg->log_for_failures),
  experiment_duration(cfg->experiment_duration) {
    auto si = cfg->sensors.begin();    
    auto ii = cfg->initial_values.begin();

    // Attention: inital_values must have twice number the values of sensors
    while( si != cfg->sensors.end() ) {      
      double x = *ii;
      ii++;
      double y = *ii;
      
      this->sensors[*si] = attribute(*si, x, y );

      si++;      
      ii++;  
    }

    // Purpose: Convert list to vector
    for(auto e : cfg->sensors) {
      this->sensor_names.push_back(e);
    }
  }