#include "srt_config.hpp"

#include <deque>
#include <map>
#include <utility>
#include "caf/io/all.hpp"
#include "attribute.hpp"
#include "node_info.hpp"
#include "query.hpp"
#include "results.hpp"

using namespace std;
using namespace caf;

srt_config::srt_config(){  
  /***** Network Config *****/
  base_ip = "localhost";
  base_port = 3331;
  base_station_mode = false;

  relative_ip = "localhost";
  relative_port = 8080;

  node_id=0;
  my_ip = "localhost";
  my_port = 3333;  
  tier = 1;
  am_first_root = false;
  
  // In [ms]. Interval between two sensor readings at this node.
  sampling_interval = 500;
  experiment_duration = 60;
  
  /***** Custom Message Types *****/
  add_message_type<attribute>("attribute");
  add_message_type<node_address>("node_address");
  add_message_type<node_info>("node_info");
  add_message_type<query>("query");
  add_message_type<sensor_state>("sensor_state");
  add_message_type<value>("value");

  add_message_type<pair<int, actor>>("pair_int_actor");
  add_message_type<pair<int, int>>("pair_int_int");
  add_message_type<map<string, pair<string, double>>>("sensor_map");  
  add_message_type<map<string,attribute>>("map_attribute");  
  add_message_type< deque< pair<int,int> > >("deque_pair_int");
  
  /***** Custom CLI arguments *****/
  opt_group{custom_options_, "global"}
    .add(base_ip, "base_ip","set base station's address (default = 'localhost')")
    .add(base_port, "base_port","set base station's port")
    .add(base_station_mode, "base-station-mode,b","enable base station mode (default = false)")
    .add(am_first_root, "am_first_root","flag for the first node that is spawned")
    .add(relative_ip,"relative_ip", "set first tree's root ip")
    .add(relative_port,"relative_port", "set first tree's root port")    
    .add(node_id, "node_id", "set this node's id")
    .add(my_ip, "my_ip,i", "set your own network address")
    .add(my_port, "my_port,p", "set port (obligatory)")
    .add(tier, "tier", "Define the tier of this node in the fog hierarchy")
    .add(sampling_interval, "sampling_interval", "In [ms]. Time between two sensor readings.")
    .add(experiment_duration, "experiment_duration", "In [s]. Delay after which the experiment is over and the logs can be written.");
}

srt_config::srt_config(int my_port, int relative_port) : srt_config() {  
  this->my_port = my_port;  
  this->relative_port = relative_port;
}