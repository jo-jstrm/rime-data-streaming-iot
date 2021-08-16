#include "tree_builder.hpp"

#include <string>
#include "srt_node.hpp"
#include "sensor_random.hpp"

using namespace std;
using namespace caf;

using build_node = atom_constant<atom("buildnode")>;
using initial_tree = atom_constant<atom("inittree")>;
using set_level = atom_constant<atom("setlvl")>;
using register_sensor = atom_constant<atom("regsenso")>;

behavior builder_actor(event_based_actor* self) {
  return {
    /** 
     * IN: root network_manager -> (build_tree, string)
     * OUT: root network_manager -> (build_tree, string)
     */ 
    [=](build_node, string attribute, int id_counter, string ip, int port, int sampling_interval, int tier, bool am_root) {
      // Make unique id
      string tree_id;
      if( attribute == "loc" ) {        
        tree_id = "loc";
      }
      else {
        // Make id
        tree_id = ip + to_string(port) + to_string(id_counter);
      }
      auto grp = self->system().groups().get_local("trees");
      auto node = self->spawn_in_group(grp, srt_node);
      // This only works for location tree currently
      self->system().registry().put( atom("loctree"), node);            
      self->send(node, attribute, tree_id, -1, sampling_interval);      
      // Initial level = tier
      // Currently the level will not change during runtime, but the means to propagate changes are implemented
      self->send(node, set_level::value, tier);
      actor random = self->spawn(random_sensor, "data");
      self->send(node, register_sensor::value, random);

      return make_result(tree_id, node);
    },
  };
}