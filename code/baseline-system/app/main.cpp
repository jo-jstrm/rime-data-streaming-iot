#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "base_station.hpp"
#include "net_init.hpp"
#include "network_manager.hpp"
#include "sensor_location.hpp"
#include "sensor_random.hpp"
#include "srt_config.hpp"

using namespace std;
using namespace caf;

using query_atom = atom_constant<atom("queryatom")>;

void run_base_station(actor_system& system, const srt_config& cfg){    
  size_t pos = 0;
  string command;
  string delimiter = " ";
  string raw;
  vector<string> commands;  

  auto base = system.spawn(base_station_actor, cfg.my_ip, cfg.my_port, cfg.node_id);

  auto published = io::publish(base, cfg.my_port);
  while ( ! published ) {
    CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "Publish failed: " + system.render(published.error()) );  

    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    published = io::publish(base, cfg.my_port);   
  }
  
  system.registry().put(atom("base"), base);
}

void run_node(actor_system& system, const srt_config& cfg) {  
  net_init ni = net_init(&cfg);

  /*-------------------------------init actors------------------------------*/
    
  // Init networker
  auto networker = system.spawn(network_manager, ni);
  system.registry().put(atom("networker"), networker);  
  
  /*-------------------------------Publish networker------------------------------*/
  
  auto published = io::publish(networker, cfg.my_port);
  
  while (!published){
    CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "Publish failed: " + system.render(published.error()) );  

    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    published = io::publish(networker, cfg.my_port);       
  }
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "Networker published");  
}

void caf_main(actor_system& system, const srt_config& cfg){  
  // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "CAF_Version =" + CAF_VERSION);  
  auto mode = cfg.base_station_mode ? run_base_station : run_node;    
  mode(system, cfg);
}

CAF_MAIN(io::middleman)