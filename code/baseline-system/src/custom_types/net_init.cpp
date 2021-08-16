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
  experiment_duration(cfg->experiment_duration){}