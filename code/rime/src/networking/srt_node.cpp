#include "srt_node.hpp"

#include <chrono>
#include <iterator>
#include <math.h>
#include <random>
#include "caf/io/all.hpp"
#include "find_node_info.hpp"
#include "node_info.hpp"
#include "results.hpp"

#define CAF_LOG_LEVEL CAF_LOG_LEVEL_ERROR
#define CAF_LOG_COMPONENT "Tree"

using namespace std;
using namespace std::chrono;
using namespace caf;

using query_atom = atom_constant<atom("queryatom")>;
using start_moving = atom_constant<atom("startmove")>;
using propagate_nodeinfo = atom_constant<atom("bcnode")>;

using collect_tree = atom_constant<atom("colltree")>;
using collected_once = atom_constant<atom("subinit")>;

using update_range = atom_constant<atom("prop")>;
using request_random_neighbor = atom_constant<atom("randneigh")>;
using get_random_child = atom_constant<atom("getrandc")>;
using random_node = atom_constant<atom("randnode")>;

using get_children = atom_constant<atom("children")>;
using get_level = atom_constant<atom("getlvl")>;
using get_subtree_range = atom_constant<atom("subtree")>;
using get_parent = atom_constant<atom("getprnt")>;

using set_level = atom_constant<atom("setlvl")>;
using propagate_level = atom_constant<atom("proplevel")>;

// Parent
using parent_level = atom_constant<atom("parlevel")>;
using find_better_parent = atom_constant<atom("findparent")>;
using new_parent = atom_constant<atom("newparent")>;
using new_backup = atom_constant<atom("newbackup")>;
using find_new_backup = atom_constant<atom("fnewbackup")>;
using root_is_parent = atom_constant<atom("rootparent")>;
using register_at_parent = atom_constant<atom("regatpar")>;
using unregister_at_parent = atom_constant<atom("unregatpar")>;
using compute_distance_to_parent = atom_constant<atom("compdist")>;
using register_at_backup = atom_constant<atom("backsub")>;

// Children
using child_registration = atom_constant<atom("childreg")>;
using delete_child = atom_constant<atom("delchild")>;

// Down handler
using node_down = atom_constant<atom("nodedown")>;

// Sensor interaction
using read = atom_constant<atom("read")>;
using state = atom_constant<atom("state")>;
using root = atom_constant<atom("root")>;

using update_subtree_range = atom_constant<atom("updsubrng")>;
using register_sensor = atom_constant<atom("regsenso")>;

behavior srt_node(stateful_actor<node_state>* self) {  
  self->state.collected_once = false;
  self->state.parent_valid = false;
  self->state.backup_valid = false;
  self->state.my_lvl = 1000;

  return {
    /*-------------------------------Init------------------------------*/  
    /**
     * Init attribute
     * IN: tree_builder -> (build_node, string, int, string, int)
     * OUT: nm ->(attribute, subtree_range, string)     
     */
    [=](string attribute, string tree_id, int level, int sampling_interval) {            
      self->state.tree_attr = attribute;
      self->state.tree_id = tree_id;
      self->state.my_lvl = level;
      self->state.sampling_interval = sampling_interval;      
      
      auto loc_sensor = self->system().registry().get(atom("locsens"));

      self->state.my_subtree_range.tree_id = self->state.tree_id;

      // Init subtree range
      self->request(actor_cast<actor>(loc_sensor), seconds(2), state::value).await([=](double lat, double lon) {
        self->state.local_att_value.x = lat;
        self->state.local_att_value.y = lon;        
        self->state.my_subtree_range.lat_min = lat;
        self->state.my_subtree_range.lat_max = lat;
        self->state.my_subtree_range.lon_min = lon;
        self->state.my_subtree_range.lon_max = lon;

        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "initial attribute range = (" + to_string(self->state.local_att_value.x) + "," + to_string(self->state.local_att_value.y) +")" );
        
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "Initial subtree range: x=(" + to_string(self->state.my_subtree_range.lat_min) +"," + to_string(self->state.my_subtree_range.lat_max) +"), y=(" + to_string(self->state.my_subtree_range.lon_min) + "," + to_string(self->state.my_subtree_range.lon_max) +  ")" );
        
        auto nm = actor_cast<actor>(self->system().registry().get(atom("networker")));
        self->send(nm, self->state.local_att_value, self->state.my_subtree_range, self->state.tree_id);
      });
    },    
    [=](register_sensor, actor sensor) {
      self->state.sensor = sensor;
    },    
    /**
     * Initialize regular behaviour, i.e. start sensor readings and find better parent,
     * after the tree was collected once
     * IN: 
     */
    [=](collected_once) {    
      // Triggered ONCE if you are not root and initial tree was collected once
      if(self->state.collected_once == false) {
        if(self->state.my_lvl > 1) {
          // For level 1, there is only root as a parent option, so no need to do it there.
          self->send<message_priority::high>(actor_cast<actor>(self), find_better_parent::value);
        }
        else if(self->state.my_lvl > 0) {
          // Root does not need to do this, because it does not have neighbors.
          self->send(actor_cast<actor>( self->system().registry().get(atom("networker"))),
                    request_random_neighbor::value);
        }
        
        self->send(actor_cast<actor>(self), read::value);
        self->state.collected_once = true;
      }
    },
    /*-------------------------------Children------------------------------*/  
    /**
     * Stores the child if it is new and requests level propagation to new child
     * Also initializes update of subtree range
     * IN: local network_manager -> (connect_atom, string, int)
     * OUT: (if) local network_manager -> (propagate_level, string, actor, int)
     */
    [=](child_registration, node_info child) {
      auto it = find( self->state.children.begin(), self->state.children.end(), child);
      if( it == self->state.children.end() ) {        
        self->state.children.push_back(child);        
        self->send(actor_cast<actor>(self), update_subtree_range::value);
        self->send(actor_cast<actor>(self->current_sender()), propagate_level::value, self->state.tree_id,
                                                                          child.handle, self->state.my_lvl);
      } 
    },
    /**
     * Checks wether to_delete is needed in this tree
     * IN: request_checker
     * RETURN: true if it is needed
     */
    [=](delete_atom, actor to_delete) {
      if(to_delete == self->state.parent.handle) {
        return true;
      }
      else {
        if( ! self->state.children.empty() ) {
          auto child_it = find_if(self->state.children.begin(), self->state.children.end(), find_relative(to_delete) );
          if( child_it == self->state.children.end() ) {
            return false;
          }
          else return true;
        }
        else return false;
      }
    },
    /**
     * Delete child and update subtree
     * IN: nm -> (delete_child, string)
     */
    [=](delete_child, actor child, int child_id) {      
      if( ! self->state.children.empty() ) {      
        auto child_it = find_if(self->state.children.begin(), self->state.children.end(), find_relative(child) );
        
        if( child_it != self->state.children.end() ) {
          self->state.children.erase(child_it);
          self->send(actor_cast<actor>(self), update_subtree_range::value);
        }
      }
    },
    /*-------------------------------Parent------------------------------*/
    /**
     * Received level from parent
     * IN: nm -> (parent_level, string, int)
     * OUT: (if) nm -> (propagate_level, string, int)
     */
    [=](parent_level, int new_parent_level) {      
      if(self->state.my_lvl != new_parent_level + 1) {
        self->state.my_lvl = new_parent_level + 1;

        if( ! self->state.children.empty() ) {
          self->send(actor_cast<actor>(self->current_sender()),
              propagate_level::value, self->state.tree_id, self->state.children, self->state.my_lvl);
        }                  
      }
    },    
    /**
     * Store new parent and compute/store distance to it
     * Register and subscribe at parent
     * IN: nm -> (connect_atom, string , int)    
     * IN: nm-> (new_parent, node_info)      
     * OUT: nm -> (register_at_parent, actor, string)
     */
    [=](new_parent, node_info new_parent) {
      auto nm = actor_cast<actor>( self->system().registry().get(atom("networker")) );
      node_info old_parent = self->state.parent;    
      node_info old_backup_parent = self->state.backup_parent;              

      self->state.backup_parent = self->state.parent;
      self->state.backup_subtree_range = self->state.parent_subtree_range;
      self->state.distance_to_backup = self->state.distance_to_parent;

      self->state.parent = new_parent;     
      self->state.parent_subtree_range = new_parent.subtree_ranges[self->state.tree_id];
      self->send(actor_cast<actor>(self), compute_distance_to_parent::value);
      
      // Two edge cases: new parent = old parent and old parent = intial parent = own networker
      if( new_parent.handle != old_parent.handle && self->state.parent_valid ) {        
        // Unregister at old parent in any case
        self->send(nm, unregister_at_parent::value, old_parent.handle, self->state.tree_id);
        
        // old backup != new backup
        // Unsubscribe from old backup
        // Do not unsubscribe from old parent, because it is the new backup
        if(old_backup_parent != self->state.backup_parent) {          
          self->send(nm, unsubscribe_atom::value, old_backup_parent.handle);
        }               
      }

      self->state.parent_valid = true;
    },
    /**
     * Store new backup     
     * No need to unsubscribe at old backup, nm does this already!
     * IN: nm-> (new_backup, node_info) 
     */
    [=](new_backup, node_info backup) {      
      auto nm = actor_cast<actor>( self->system().registry().get(atom("networker")) );
      node_info old_backup = self->state.backup_parent;
      auto& my_lat = self->state.local_att_value.x;
      auto& my_lon = self->state.local_att_value.y;
      auto& backup_lat = backup.sensors[self->state.tree_attr].x;
      auto& backup_lon = backup.sensors[self->state.tree_attr].y;
      double lat_dist = 0;
      double lon_dist = 0;
      
      self->state.backup_parent = backup;     
      self->state.backup_subtree_range = backup.subtree_ranges[self->state.tree_id];
      
      // Compute distance
      lat_dist = my_lat - backup_lat;
      lon_dist = my_lon - backup_lon;

      self->state.distance_to_backup = sqrt(lat_dist*lat_dist + lon_dist*lon_dist);

      if(self->state.backup_parent.handle != self->state.parent.handle && self->state.backup_valid) {
        self->send(nm, register_at_backup::value, self->state.backup_parent);
      }

      self->state.backup_valid = true;
    },
    [=](compute_distance_to_parent) {
      auto& my_lat = self->state.local_att_value.x;
      auto& my_lon = self->state.local_att_value.y;
      auto& parent_lat = self->state.parent.sensors[self->state.tree_attr].x;
      auto& parent_lon = self->state.parent.sensors[self->state.tree_attr].y;
      double lat_dist = 0;
      double lon_dist = 0;

      // Compute distance
      lat_dist = my_lat - parent_lat;
      lon_dist = my_lon - parent_lon;

      self->state.distance_to_parent = sqrt(lat_dist*lat_dist + lon_dist*lon_dist);
      
      if(self->state.collected_once) {
        self->send<message_priority::high>(actor_cast<actor>(self), find_better_parent::value);
      }
    },
    [=](find_better_parent) {
      auto nm = actor_cast<actor>( self->system().registry().get(atom("networker")) );          
      self->send<message_priority::high>(nm, find_better_parent::value,
                                          self->state.parent, self->state.distance_to_parent,
                                          self->state.tree_id, self->state.local_att_value);
    },
    /*-------------------------------Subtree------------------------------*/  
    /**
     * IN: self -> (delete_child, actor, int)
     * IN: self -> (child_registration, node_info)
     * IN: self -> (node_info)
     * OUT: nm -> (propagate, subtree_range sub_range, string tree_id, actor parent)
     */
    [=](update_subtree_range) {
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "update_subtree_range: begin");
      subtree_range old_range = self->state.my_subtree_range;
      
      auto& cur_lat_min = self->state.my_subtree_range.lat_min;
      auto& cur_lat_max = self->state.my_subtree_range.lat_max;
      auto& cur_lon_min = self->state.my_subtree_range.lon_min;
      auto& cur_lon_max = self->state.my_subtree_range.lon_max;
      
      bool propagate = false;
      auto& id = self->state.tree_id;
      
      // Start with local attribute value
      cur_lat_min = self->state.local_att_value.x;
      cur_lat_max = self->state.local_att_value.x;
      cur_lon_min = self->state.local_att_value.y;
      cur_lon_max = self->state.local_att_value.y;

      // Iterate over children and update accordingly
      if( ! self->state.children.empty() ) {
        for(auto& child : self->state.children) {
          if(child.subtree_ranges[id].lat_min < cur_lat_min) {
            cur_lat_min = child.subtree_ranges[id].lat_min;          
          }
          if(child.subtree_ranges[id].lat_max > cur_lat_max) {
            cur_lat_max = child.subtree_ranges[id].lat_max;          
          }
          if(child.subtree_ranges[id].lon_min < cur_lon_min) {
            cur_lon_min = child.subtree_ranges[id].lon_min;          
          }
          if(child.subtree_ranges[id].lon_max > cur_lon_max) {
            cur_lon_max = child.subtree_ranges[id].lon_max;          
          }
        }
      }
      if(! (old_range == self->state.my_subtree_range) ) {
        propagate = true;
      }

      if(propagate) {
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "new subtree range: x=(" + to_string(self->state.my_subtree_range.lat_min) +"," + to_string(self->state.my_subtree_range.lat_max) +"), y=(" + to_string(self->state.my_subtree_range.lon_min) + "," + to_string(self->state.my_subtree_range.lon_max) +  ")" );

        // Do not send this when you are root (because you have no parent)
        if(self->state.my_lvl > 0) {
          auto nw = self->system().registry().get(atom("networker"));
          self->send(actor_cast<actor>(nw), update_range::value, self->state.my_subtree_range, self->state.tree_id, 
                  self->state.parent.handle);
        }
      }  
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "new subtree-range: end");
    },    
    /**
     * Check if info is from one of your children or parent
     * If so, call update handler to check if own subtree range changes
     * IN: nw -> (node_info)
     * IN: nw -> (subtree_range, string)
     */
    [=](node_info node) {      
      if(node.handle == self->state.parent.handle) {
        self->state.parent = node;
        self->send(actor_cast<actor>(self), compute_distance_to_parent::value);
        self->send(actor_cast<actor>(self), update_subtree_range::value);        
        return;
      }
      
      // Use find_relative, because the rest of the node_info might have changed!
      if( ! self->state.children.empty() ) {
        auto it = find_if(self->state.children.begin(), self->state.children.end(), find_relative(node.handle) );
        if( it != self->state.children.end() ) {
          *it = node;
          self->send(actor_cast<actor>(self), update_subtree_range::value);
        }
      }
    },
    /*-------------------------------Sensor readings------------------------------*/  
    /**
     * Reads the sensor every x seconds
     */
    [=](read) {
      self->send(self->state.sensor, read::value);
      self->delayed_send<message_priority::high>(actor_cast<actor>(self), 
            milliseconds(self->state.sampling_interval), read::value);
    },
    /**
     * Fresh sensor reading
     */
    [=](value res) {      
      auto nm = actor_cast<actor>(self->system().registry().get(atom("networker")));          
      int& my_lvl = self->state.my_lvl;
      res.tree_id = self->state.tree_id;      
      
      if(my_lvl > 0) {
        self->send(nm, res, self->state.parent.handle, my_lvl);
      }
      else if(my_lvl == 0) {        
        self->send(nm, res, actor_cast<actor>(self), my_lvl);
      }
    },
    /*-------------------------------Query------------------------------*/  
    [=](query_atom, double qx, double qy) {
      // Query applies, if x >= qx & y >= qy
      list<node_info> receivers;
      bool applies = false;
      
      // If subtree range applies potentially
      if( self->state.my_subtree_range.lat_max >= qx && self->state.my_subtree_range.lon_max >= qy ) {
        for(auto& child : self->state.children) {
          if(child.subtree_ranges[self->state.tree_id].lat_max >= qx
              && child.subtree_ranges[self->state.tree_id].lon_max >= qy )
          {
            receivers.push_back(child);
            applies = true;
          }
        }
        /*
        if(self->state.local_att_value.x >= qx && self->state.local_att_value.y >= qy ) {
          applies = true;
        } 
        */       
      }
      
      return make_result(applies, receivers);
    },
    /*-------------------------------Movement------------------------------*/  
    [=](start_moving, int count) {      
      auto nm = actor_cast<actor>( self->system().registry().get(atom("networker")) );

      if(count > 0) {
        if(self->state.local_att_value.x < 97.0) {
          self->state.local_att_value.x += 3.0;
        }
        if(self->state.local_att_value.y < 97.0) {
          self->state.local_att_value.y += 3.0;
        }

        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "MOVED: new position x="
            + to_string(self->state.local_att_value.x)
            + ", y=" + to_string(self->state.local_att_value.y) );

        count--;
        self->send(actor_cast<actor>(self), update_subtree_range::value);        
        self->send(nm, self->state.local_att_value, self->state.my_subtree_range, self->state.tree_id);
        self->send(nm, propagate_nodeinfo::value);
        self->delayed_send(actor_cast<actor>(self), seconds(3),  start_moving::value, count);
      }
      else {
        if(self->state.my_lvl > 1) {
          self->send<message_priority::high>(actor_cast<actor>(self), find_better_parent::value);          
        }
      }
    },
    /*-------------------------------Utility------------------------------*/  
    /**
    * Remote node is down. Check if it is part of your children or parent.
    * IN: nm -> down_handler
    */
    [=](node_down, actor down) {
      auto nm = actor_cast<actor>(self->system().registry().get(atom("networker")));
      
      if(down == self->state.parent.handle) {        
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "PARENT DOWN, old parent = " + to_string(self->state.parent.node_id) + ", backup = " + to_string(self->state.backup_parent.node_id));

        // Use backup.
        self->state.parent = self->state.backup_parent;
        self->state.parent_subtree_range = self->state.backup_subtree_range;
        self->state.distance_to_parent = self->state.distance_to_backup;
        
        self->send<message_priority::high>(nm, register_at_parent::value, self->state.parent,
                                              self->state.parent.node_id, self->state.tree_id);        
        self->send<message_priority::high>(actor_cast<actor>(self), find_better_parent::value);
      }
      else if(down == self->state.backup_parent.handle) {
        self->state.backup_parent = self->state.parent;
        self->state.distance_to_backup = self->state.distance_to_parent;
        self->state.backup_subtree_range = self->state.parent_subtree_range;
        self->send<message_priority::high>(actor_cast<actor>(self), find_better_parent::value);
      }
      else {
        if( ! self->state.children.empty() ) {
          for(auto& child : self->state.children) {
            if(child.handle == down) {              
              self->state.children.remove(child);
              break;
            }
          }
        }
      }
    },
    [=](get_parent) {
      return make_result(get_parent::value, self->state.parent.handle);
    },
    [=](get_parent, get_level) {
      return make_result(get_parent::value, get_level::value, self->state.parent.handle, self->state.my_lvl);
    },   
    [=](get_children) {      
      return self->state.children;
    },
    /**
     * IN: local network_manager -> (get_level, string)
     * OUT: remote network_manager
     * RETURN: own level in tree
     */
    [=](get_level) {
      return self->state.my_lvl;
    },
    /** 
     * Set level and propagate it to children
     * IN: tree_builder -> (build_node, string, bool)
     * OUT: (if) nw -> (propagate_level, string, list, int)
     */
    [=](set_level, int new_lvl) {      
      int old_lvl = self->state.my_lvl;

      if(old_lvl != new_lvl) {
        auto nm = actor_cast<actor>(self->system().registry().get(atom("networker")));      
        self->state.my_lvl = new_lvl;
        self->send(actor_cast<actor>(nm), propagate_level::value, self->state.tree_id, self->state.children, new_lvl);
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "New level: " + to_string(self->state.my_lvl));
      }
    }, 
    /**
     * We tolerate the possibility that this returns the node that requested a random child
     */
    [=](get_random_child, int node_id) {    
      if(self->state.children.size() == 1) {
        return *(self->state.children.begin() );
      }
      else {        
        int position;
        auto it = self->state.children.begin();      
        system_clock::time_point begin = system_clock::now();      
        system_clock::duration dur;
        mt19937 gen;
        uniform_int_distribution<> dist( 0, self->state.children.size() - 1 );

        //seed the generator
        dur = system_clock::now() - begin;      
        gen.seed(dur.count());
        position = dist(gen);
        advance(it, position); 

        if(it->node_id == node_id) {
          self->delegate(actor_cast<actor>(self), get_random_child::value, node_id);
        }
        return(*it);
      }
    },
  };
}