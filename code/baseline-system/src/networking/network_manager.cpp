#include "network_manager.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iterator>
#include <random>
#include <sstream>
#include <utility>
#include "caf/io/all.hpp"
#include "find_node_info.hpp"
#include "logger.hpp"
#include "net_init.hpp"
#include "node_address.hpp"
#include "node_info.hpp"
#include "query.hpp"
#include "results.hpp"
#include "sensor_location.hpp" 
#include "sensor_random.hpp"

#define CAF_LOG_LEVEL CAF_LOG_LEVEL_ERROR
#define CAF_LOG_COMPONENT "Networker"

/**
 * Assumptions:
 *   Roots do not fail
 *   Base(s) do not fail
 */

using namespace caf;
using namespace std;
using namespace std::chrono;

using query_atom = atom_constant<atom("queryatom")>;

using init = atom_constant<atom("init")>;

using read = atom_constant<atom("read")>;
using remote_reading = atom_constant<atom("remoter")>;
using tuples = atom_constant<atom("tuples")>;

network_state::network_state() {}

// first and second must contain "attribute"
bool comp_attr(const pair<node_info, int64_t>& first, const pair<node_info, int64_t>& second,
                const double& reference, const string& attribute)
{
  return abs(reference - first.first.sensors.find(attribute)->second.x) 
    < abs(reference - second.first.sensors.find(attribute)->second.x);
}

behavior network_manager(stateful_actor<network_state>* self, net_init net_init){
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "STARTUP");   
  
  /*-------------------------------init state------------------------------*/
  self->state.sampling_interval = net_init.sampling_interval;     
  self->state.my_info = node_info(actor_cast<actor>(self),
    net_init.my_ip,
    net_init.my_port,
    net_init.node_id,
    net_init.tier);
  self->state.sensor = self->spawn(random_sensor, "data");

  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "my_ip=" + net_init.my_ip );
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "my_port=" + to_string(net_init.my_port) );
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "my_tier=" + to_string(net_init.tier) );
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "am_first_root=" + to_string(net_init.am_first_root) );
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "parent_ip=" + net_init.relative_ip );
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "parent_port=" + to_string(net_init.relative_port) );

  // Logger
  auto log = self->system().spawn(rime::logger, net_init.experiment_duration);
  self->system().registry().put(atom("logger"), log);
  
  // If not root
  if(! net_init.am_first_root) {
    self->send(actor_cast<actor>(self), connect_atom::value, net_init.relative_ip, net_init.relative_port);    
  }

  /*---------------------------------Handlers-------------------------------*/
  
  return {
    /*---------------------------------Init---------------------------------*/
    
    /**
     * Initial connection to parent
     * IN: Constructor
     * OUT: parent nm -> (init, node_info)
     * OUT: tree -> (new_parent, node_info)
     */
    [=](connect_atom, string parent_ip, int parent_port) {      
      CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "Try to connect");  
          
      if(auto parent = self->system().middleman().remote_actor(parent_ip, parent_port) ) {                            
        self->state.parent = *parent;

        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "CONNECTED");  
      }
      else {        
        self->delayed_send(actor_cast<actor>(self), seconds(5), connect_atom::value, parent_ip, parent_port);
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "COULD NOT REACH INITIAL PARENT, RECONNECTING...");
      }      

      self->send(actor_cast<actor>(self), read::value);
    },
    /*-------------------------------data handling------------------------------*/    
    /**
     * Reads the sensor every x milliseconds
     */
    [=](read) {      
      self->send(self->state.sensor, read::value);
      self->delayed_send<message_priority::high>(actor_cast<actor>(self), 
        milliseconds(self->state.sampling_interval), read::value);
    },
    /**
     * From child
     * IN: child nm -> (value)
     */
    [=](remote_reading, value res) {     
      std::chrono::system_clock::time_point arrival_tp = std::chrono::system_clock::now();

      if(self->state.my_info.tier == 0) {
        // We want the latency only at root        
        auto latency = chrono::duration<double>(arrival_tp - res.ts).count();
        string log = to_string(arrival_tp.time_since_epoch().count()) + " DATA RECEIVED FROM " + to_string(res.node_id) 
                      + " Source-to-sink-latency [s] " + to_string(latency);
        self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), tuples::value, log);
      }
      else { 
        // Forward data to your parent (tier and level mean the same)
        self->send<message_priority::high>(self->state.parent, remote_reading::value, res);
      }
    },
    /**
     * From own sensor
     */
    [=](value res) {      
      res.node_id = self->state.my_info.node_id;
      
      if(self->state.my_info.tier > 0) {
        self->send<message_priority::high>(self->state.parent, remote_reading::value, res);        
      }
    },  
  };
}