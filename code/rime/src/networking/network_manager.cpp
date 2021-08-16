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
#include "pinger.hpp"
#include "query.hpp"
#include "request_checker.hpp"
#include "results.hpp"
#include "sensor_location.hpp" 
#include "srt_node.hpp" 
#include "subtree_range.hpp"
#include "tree_builder.hpp"

#define CAF_LOG_LEVEL CAF_LOG_LEVEL_ERROR
#define CAF_LOG_COMPONENT "Networker"

/**
 * Assumptions:
 *** Roots do not fail
 *** Base(s) do not fail
 *** All nodes have the same trees
 */

using namespace caf;
using namespace std;
using namespace std::chrono;

using query_atom = atom_constant<atom("queryatom")>;
using start_moving = atom_constant<atom("startmove")>;

using collect_tree = atom_constant<atom("colltree")>;
using print_tree = atom_constant<atom("printtree")>;

using propagate_nodeinfo = atom_constant<atom("bcnode")>;
using broadcast = atom_constant<atom("broadcast")>;

using node_down = atom_constant<atom("nodedown")>;
using subtree = atom_constant<atom("subtree")>;
using collected_once = atom_constant<atom("subinit")>;

using build_node = atom_constant<atom("buildnode")>;
using build_request = atom_constant<atom("build")>;

using new_parent = atom_constant<atom("newparent")>;
using root_is_parent = atom_constant<atom("rootparent")>;
using find_better_parent = atom_constant<atom("findparent")>;
using find_random_for_sibling = atom_constant<atom("findrands")>;
using new_backup = atom_constant<atom("newbackup")>;
using find_new_backup = atom_constant<atom("fnewbackup")>;
using parent_level = atom_constant<atom("parlevel")>;
using register_at_parent = atom_constant<atom("regatpar")>;
using unregister_at_parent = atom_constant<atom("unregatpar")>;
using register_at_backup = atom_constant<atom("backsub")>;

using get_level = atom_constant<atom("getlvl")>;
using get_relatives = atom_constant<atom("relatives")>;
using get_tier = atom_constant<atom("tier")>;
using get_children = atom_constant<atom("children")>;
using get_parent = atom_constant<atom("getprnt")>;

using init = atom_constant<atom("init")>;
using register_base = atom_constant<atom("regibase")>;

using request_random_neighbor = atom_constant<atom("randneigh")>;
using get_random_child = atom_constant<atom("getrandc")>;
using random_node = atom_constant<atom("randnode")>;

using child_registration = atom_constant<atom("childreg")>;
using delete_child = atom_constant<atom("delchild")>;

using propagate_level = atom_constant<atom("proplevel")>;
using update_range = atom_constant<atom("prop")>;

using tuples = atom_constant<atom("tuples")>;
using root = atom_constant<atom("root")>;

const int INITIAL_DISTANCE = 10000.0;
const int NUMBER_OF_MOVEMENTS = 4;

network_state::network_state() {
  // Make tree_id_counter random
  auto begin = system_clock::now();
  mt19937 gen;
  uniform_int_distribution<int> dist(1, 5000);
  
  auto dur = system_clock::now() - begin;
  gen.seed(dur.count());
  tree_id_counter = dist(gen);
}

// First and second must contain "attribute"
bool comp_attr(const pair<node_info, int64_t>& first, const pair<node_info, int64_t>& second,
                                              const double& reference, const string& attribute)
{
  return abs(reference - first.first.sensors.find(attribute)->second.x) 
    < abs(reference - second.first.sensors.find(attribute)->second.x);
}

behavior network_manager(stateful_actor<network_state>* self, net_init net_init){
  /*-------------------------------init state------------------------------*/
  self->state.base_ip = net_init.base_ip;
  self->state.base_port = net_init.base_port;  
  self->state.sampling_interval = net_init.sampling_interval; 
  self->state.start_moving = net_init.start_moving; 
  deque<pair<int,int>> d;
  self->state.subtree = d;
  self->state.random_neighbor.node_id = -1;
  self->state.log_for_failures = net_init.log_for_failures;
  self->state.log_tuples = net_init.log_tuples;

  self->state.my_info = node_info(actor_cast<actor>(self),
    net_init.my_ip,
    net_init.my_port,
    net_init.node_id,
    net_init.tier,
    net_init.sensors);

  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "node_id=" + to_string(self->state.my_info.node_id) );
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "my_ip=" + self->state.my_info.ip );
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "my_port=" + to_string(self->state.my_info.port) );
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "tier=" + to_string(self->state.my_info.tier) );
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "am_first_root=" + to_string(net_init.am_first_root) );
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "parent_ip=" + net_init.relative_ip );
  CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "parent_port=" + to_string(net_init.relative_port) );

  // Logger
  auto log = self->system().spawn(rime::logger, net_init.experiment_duration);
  self->system().registry().put(atom("logger"), log);

  // Build location tree locally
  self->send(actor_cast<actor>(self), build_request::value, "loc", net_init.am_first_root);  

  if( net_init.am_first_root) {
    // If this node is the root
    self->delayed_send(actor_cast<actor>(self), seconds(5), collect_tree::value);
  }
  else {    
    self->send(actor_cast<actor>(self), connect_atom::value, net_init.relative_ip, net_init.relative_port);    
  }
  
  // Down-handler for monitored (i.e. all) relatives.  
  self->set_down_handler(
    [=](const down_msg& dm){        
      actor down = actor_cast<actor>(dm.source);      
      auto& sub = self->state.subscribers;      
      auto sub_it = find_if(sub.begin(), sub.end(), [=,&down](pair<actor, int> sub) {
        return sub.first == down;
      });

      // Log
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " ACTOR DOWN " + to_string(down);
      CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "ACTOR DOWN" + to_string(down) );
      if( sub_it != sub.end() ) {
        log += " " + to_string(sub_it->second);
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "ACTOR DOWN" + to_string(sub_it->second) );
      }
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      
      // Search for "down" in subscribers
      // If in it, delete it       
      self->send<message_priority::high>(actor_cast<actor>(self), unsubscribe_atom::value, down);

      // Notify trees, maybe "down" was a parent or child 
      for(auto& tree : self->state.trees) {
        self->send<message_priority::high>(tree.second, node_down::value, down);
      }
    }
  );
  
  return {
    /*---------------------------------Init---------------------------------*/
    
    /**
     * Initial connection to parent
     * IN: main
     * OUT: parent nm -> (init, node_info)
     * OUT: tree -> (new_parent, node_info)
     */
    [=](connect_atom, string parent_ip, int parent_port) {      
      CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "Try to connect");  

      // Necessary, wait for tree to have read location from sensor
      if( self->state.my_info.subtree_ranges["loc"].lat_min == 0.0
          || self->state.my_info.subtree_ranges["loc"].lat_max == 0.0
          || self->state.my_info.subtree_ranges["loc"].lon_min == 0.0
          || self->state.my_info.subtree_ranges["loc"].lon_max == 0.0 )
      {
          self->delayed_send(actor_cast<actor>(self), seconds(1), connect_atom::value, parent_ip, parent_port);
          return;
      }
          
      if(auto parent = self->system().middleman().remote_actor(parent_ip, parent_port) ) {                    
        // Register at parent 
        self->send(*parent, init::value, self->state.my_info);    

        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "CONNECTED");  

        if(self->state.start_moving) {
          CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "! MOVING , starting after 30s");  
          self->delayed_send(self->state.trees["loc"], seconds(30), start_moving::value, NUMBER_OF_MOVEMENTS);  
        }
      }
      else {        
        self->delayed_send(actor_cast<actor>(self), seconds(5), connect_atom::value, parent_ip, parent_port);
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "COULD NOT REACH INITIAL PARENT, RECONNECTING...");
      }      
    },
    /**
     * IN: remote child nm -> (connect_atom, string, int)
     * OUT: local loc-tree -> (child_registraiong, actor)
     * RETURN: (init, node_info)
     */
    [=](init, node_info child) {
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: INIT ";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE: INIT");
      
      self->send(child.handle, new_parent::value, self->state.my_info, "loc");      
      
      self->send(child.handle, new_backup::value, self->state.my_info, "loc");
    },
    /**
     * Register base for a tree
     * IN: base
     */
    [=](register_base, node_address base) {      
      self->state.bases["loc"] = base;
    },
    /**
     * IN: tree -> (parent_level, int) if you have children
     * IN: tree -> (set_level, int) if initial root
     * OUT: remote nm -> (parent_level, string, int)
     */
    [=](propagate_level, string tree_id, list<node_info> children, int level) {
      if( ! children.empty() ) {
        auto& tree = self->state.trees[tree_id];      
        for(auto& child : children) {
          self->send(child.handle, parent_level::value, tree_id, level);          
        }
      }      
    },
    /**
     * Sends a new child your level
     * IN: tree -> (child_registration, node_info)
     * OUT: remote nm -> (parent_level, string, int)
     */
    [=](propagate_level, string tree_id, actor child, int level) {
      self->send(child, parent_level::value, tree_id, level);
    },
    /**
     * IN: parent nm -> (propagate_level, string, list, int)
     * OUT: tree -> (parent_level, int)
     */
    [=](parent_level, string tree_id, int level) {      
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);      

      self->send(self->state.trees[tree_id], parent_level::value, level);
    },    
    /**
     * Location tree building. Only a new root receives this.     
     * IN: base -> (build_request, string, string, int)   
     * OUT: remote network_manager ->
     * @param attribute = the attriubte that the tree is built over
     * @param base = base that issued the build request (aka sender)
     */ 
    [=](build_request, string attribute, bool am_root) {
      actor builder = self->system().spawn(builder_actor);      
      
      // Get root node + tree_id
      self->request(builder, seconds(5), build_node::value, 
                                attribute, 
                                self->state.tree_id_counter, 
                                self->state.my_info.ip, 
                                self->state.my_info.port,
                                self->state.sampling_interval,
                                self->state.my_info.tier,
                                am_root)
      .await([=](string id, actor tree) {
          // Increment counter for future id generation.
          self->state.tree_id_counter++;
          self->state.trees[id] = tree;          
        }
      );     
    },
    /**
     * Request ranom neighbor from parent
     * IN: nm -> (random_node, node_info)
     * OUT: remote nm -> (get_random_child, node_info)
     */
    [=](request_random_neighbor) {
      self->request(self->state.trees["loc"], seconds(2), get_parent::value).then(
        [=](get_parent, actor parent) {          
          self->send(parent, get_random_child::value, self->state.my_info);  
        }
      );
    },
    /**
     * IN: child nm -> (request_random_neighbor)
     * OUT: child nm -> (random_node, node_info)
     */
    [=](get_random_child, node_info sender) {
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE");
      
      self->request(self->state.trees["loc"], seconds(2), get_random_child::value, sender.node_id).then(
        [=](node_info random_child) {
          self->send(sender.handle, random_node::value, random_child);
        }
      );
    },
    /**
     * Received random neighbor, i.e. your sibling from parent
     * IN: remote nm -> (get_random_child, node_info)
     * OUT: self -> (request_random_neigbor)     
     */
    [=](random_node, node_info random_neighbor) {      
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: random_node";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE: random_node");
      
      // Unsub at old random
      if(self->state.random_neighbor.node_id != -1) {
        self->send(self->state.random_neighbor.handle, unsubscribe_atom::value, actor_cast<actor>(self) );
      }
      
      self->state.random_neighbor = random_neighbor;

      if( auto rand = self->system().middleman().remote_actor(random_neighbor.ip, random_neighbor.port) ) {
        self->state.random_neighbor.handle = *rand;

        self->send(*rand, subscribe_atom::value, self->state.my_info.handle, self->state.my_info.node_id);
      }
      else {
        // nop
      }

      self->delayed_send(actor_cast<actor>(self), seconds(10), request_random_neighbor::value);
    },
    /*-------------------------------Relatives------------------------------*/    

    /**
     * Handles arrival of new node infos.     * 
     * IN: other node's networker-> (init)     
     */
    [=](node_info node) {
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE");      
              
      // Send the subtree_range to trees
      for(auto& tree : self->state.trees) {
        self->send(tree.second, node);
      }      
    },    
    /**
     * Remote node subscribes to your info
     */
    [=](subscribe_atom, actor node, int node_id) {           
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: subscribe";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);      
      
      if(node_id != self->state.my_info.node_id) {
        auto it = find_if(self->state.subscribers.begin(), self->state.subscribers.end(),
          [=, &node]( pair<actor, int> sub ) {
            return sub.first == node;
        });
        
        if( it == self->state.subscribers.end() ) {
          self->state.subscribers.push_back(make_pair(node, node_id) );     
        }
      }
    },
    /**
     * Delete node from subscribers
     * IN: tree -> (new_parent, node_info, bool)
     */
    [=](unsubscribe_atom, actor to_delete) {
      self->state.subscribers.remove_if([=, &to_delete]( pair<actor, int> sub ) {
        return sub.first == to_delete;
      });      
    },
    /**
     * Broadcast own info
     * IN: self -> (attribute, subtree_range, string) OR self->(new_parent)
     * OUT: all relatives' networkers->(node_info)
     */
    [=](propagate_nodeinfo) {      
      if(! self->state.subscribers.empty()) {
        for(auto& sub : self->state.subscribers){
          self->send(sub.first, self->state.my_info);
        }
      }      
    },
    /*-------------------------------Parent Selection------------------------------*/  
    /**
     * 1) Start request for better parent 
     * IN: tree -> (find_better_parent, bool)
     * OUT: parent nm -> (find_better_parent, int, string, double, attribute)
     * @param only_backup indicates if a new backup is needed instead of a better parent
     */
    [=](find_better_parent, node_info current_parent, double distance_to_current_parent, 
          string tree_id, attribute local_att_value) {
      self->send<message_priority::high>(current_parent.handle, find_better_parent::value, self->state.my_info.node_id,
            self->state.my_info.tier, tree_id, distance_to_current_parent, local_att_value);      
    },
    /**
     * 2) Forward request from child for better parent to your parent
     * IN: child nm -> (find_better_parent, node_info, double, string, attribute)
     * OUT: parent nm -> (find_better_parent, )
     */
    [=](find_better_parent, int child_id, int child_tier, string tree_id, double distance_to_current_parent,
                                                                                  attribute child_att_value) {
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: find_better_parent at parent";
      self->send<message_priority::high>(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE: find_better_parent parent");

      actor sender = actor_cast<actor>( self->current_sender() );
      self->request(self->state.trees[tree_id], seconds(2), get_parent::value).then(
        [=](get_parent, actor parent) {          
          self->send<message_priority::high>(parent, find_better_parent::value, sender, child_id, child_tier,
                      tree_id, distance_to_current_parent, child_att_value);            
        }
      );
    },
    /**
     * 3) Search in your children for node with shorter distance than distance_to_current_parent
     * IN: child nm -> (find_better_parent, string, double, attribute)
     * OUT: (if) grandchild nm -> (new_parent, actor, string, double)
     */
    [=](find_better_parent, actor grandchild, int grandchild_id, int grandchild_tier, string tree_id,
                                      double distance_to_current_parent, attribute child_att_value) {      
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: find_better_parent grandparent";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE: find_better_parent grandparent");      
      
      self->request(self->state.trees[tree_id], seconds(2), get_children::value).then(
        [=](list<node_info> children) {
          if( ! children.empty() ) {
            double dist = INITIAL_DISTANCE;
            double best_dist = INITIAL_DISTANCE;
            double second_best_dist = INITIAL_DISTANCE;      
            node_info best_info;
            node_info second_best_info;

            for(auto& node : children) {                                
              // Compute euclidean distance
              dist = sqrt( pow( node.sensors[child_att_value.name].x - child_att_value.x, 2 )
                        + pow( node.sensors[child_att_value.name].y - child_att_value.y, 2 ) );              

              if(dist < best_dist) {
                second_best_dist = best_dist;
                second_best_info = best_info;

                best_dist = dist;
                best_info = node;
              }            
              else if(dist < second_best_dist) {
                second_best_dist = dist;
                second_best_info = node;
              }
            }

            if( self->state.random_neighbor.node_id != -1 && self->state.my_info.tier > 1) {
              // Take random neighbor into account
              self->request<message_priority::high>(self->state.random_neighbor.handle, seconds(2), 
                find_random_for_sibling::value, distance_to_current_parent, tree_id).then(
                  [=, &second_best_dist, &second_best_info, &best_dist, &best_info](node_info random_parent, double random_parent_distance){                           
                    if(random_parent_distance < second_best_dist) {
                      second_best_dist = random_parent_distance;
                      second_best_info = random_parent;
                    }              
                    if(random_parent_distance < best_dist) {
                        second_best_dist = best_dist;
                        second_best_info = best_info;

                        best_dist = random_parent_distance;
                        best_info = random_parent;
                    }

                    if(best_dist < distance_to_current_parent) {
                      // No matter the new_backup flag, if you find a better parent, send it!
                      // Always include a new backup (if there is a backup)
                      self->send<message_priority::high>(grandchild, new_parent::value, best_info, tree_id);                      
                      
                      if(second_best_dist != INITIAL_DISTANCE) {
                        self->send<message_priority::high>(grandchild, new_backup::value, second_best_info, tree_id);
                      }                          
                    }                        
                    else if (best_dist == distance_to_current_parent && second_best_dist != INITIAL_DISTANCE) {
                      // If a backup was requested, and it was not better than the actual parent, send it
                      self->send<message_priority::high>(grandchild, new_backup::value, second_best_info, tree_id);                      
                    }
                    else {
                      // best > current or (best == current and no second best)
                      self->send<message_priority::high>(grandchild, new_backup::value, best_info, tree_id);                      
                    }                     
                  }
                );
            }            
            else {
              if(best_dist < distance_to_current_parent) {
                // No matter the new_backup flag, if you find a better parent, send it!
                // Always include a new backup (if there is a backup)
                self->send<message_priority::high>(grandchild, new_parent::value, best_info, tree_id);                
                
                if(second_best_dist != INITIAL_DISTANCE) {
                  self->send(grandchild, new_backup::value, second_best_info, tree_id);                  
                }                          
              }                        
              else if (best_dist == distance_to_current_parent && second_best_dist != INITIAL_DISTANCE){                
                // If a backup was requested, and it was not better than the actual parent, send it
                self->send<message_priority::high>(grandchild, new_backup::value, second_best_info, tree_id);                
              }
              else {
                // best > current or (best == current and no second best)
                self->send<message_priority::high>(grandchild, new_backup::value, best_info, tree_id);                
              }
            }

          }        
        }
      );
    },
    /**
     * IN: sibling nm -> (find-better_parent, actor, int, int, string, double, attribute)
     * OUT: same
     * RETURN: best fiiting node for dist_to_current_parent form own children
     */
    [=](find_random_for_sibling, double dist_to_current_parent, attribute node_att_val, string tree_id) {
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: find_random_for_sibling";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);      
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE: find_random_for_sibling");
      
      auto res_info = self->make_response_promise<node_info>();
      auto res_dist = self->make_response_promise<double>();
      node_info rand_info;
      double rand_distance;
      
      self->request(self->state.trees[tree_id], seconds(2), get_children::value).then(
        [=, &rand_info, &rand_distance](list<node_info> children) mutable {
          if( ! children.empty() ) {
            double dist = INITIAL_DISTANCE;
            double best_dist = INITIAL_DISTANCE;               
            node_info best_info;

            for(auto& node : children) {                                
              // Compute euclidean distance
              dist = sqrt( pow( node.sensors[node_att_val.name].x - node_att_val.x, 2 )
                        + pow( node.sensors[node_att_val.name].y - node_att_val.y, 2 ) );              

              if(dist < best_dist) {
                best_dist = dist;
                best_info = node;
              }
            }

            res_info.deliver(best_info);
            res_dist.deliver(best_dist);
          }
        }
      );

      return res_info;
    },
    /**
     * IN: remote nm -> (find_better_parent, actor, int, int, string, double, attribute, bool)
     * OUT: tree -> (new_parent, node_info, bool)
     * @param requested indicates if the new parent was requested by the node
     */
    [=](new_parent, node_info new_parent, string tree_id) {
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: new_parent";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE: new_parent");      

      if(new_parent.tier != self->state.my_info.tier - 1) {
        return;
      }
      
      auto test_parent = self->system().middleman().remote_actor(new_parent.ip, new_parent.port);

      if(test_parent) {
        self->send(*test_parent, child_registration::value, self->state.my_info, tree_id);        
      }
      self->send(self->state.trees[tree_id], new_parent::value, new_parent);      
      self->monitor(new_parent.handle);
      self->send(actor_cast<actor>(self), subscribe_atom::value, new_parent.handle, new_parent.node_id);
      self->send(new_parent.handle, child_registration::value, self->state.my_info, tree_id);      
    },
    /**
     * IN: remote nm -> (find_better_parent, actor, int, int, string, double, attribute, bool)
     * OUT: tree -> (new_backup, node_info, bool)     
     */
    [=](new_backup, node_info new_backup, string tree_id) {
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: new_backup";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE: new_backup");      
      
      self->send(actor_cast<actor>(self), subscribe_atom::value, new_backup.handle, new_backup.node_id);
      self->send(self->state.trees[tree_id], new_backup::value, new_backup);      
    },
    /*-------------------------------children------------------------------*/    
    /**
     * Register at your new parent
     * IN: tree -> (new_parent, node_info, bool)
     * OUT: parent nm -> (new_parent, string)
     */
    [=](register_at_parent, node_info new_parent, int new_parent_id, string tree_id) {               
        self->send(new_parent.handle, child_registration::value, self->state.my_info, tree_id);
        
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "MESSAGE SENT: registration at parent " + to_string(new_parent_id) );  

        self->send<message_priority::high>(actor_cast<actor>(self), subscribe_atom::value, new_parent.handle, new_parent_id);
    },
    /**
     * IN: child nm -> (register_at_parent, actor, string)
     * OUT: tree -> (child_registration, node_info9)
     */
    [=](child_registration, node_info new_child, string tree_id) {
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: child_registration from " + to_string(new_child.node_id);
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE: child_registration from "
      //               + to_string(new_child.node_id) );
      
      self->send(self->state.trees[tree_id], child_registration::value, new_child);
      self->monitor(new_child.handle);
      self->send(actor_cast<actor>(self), subscribe_atom::value, new_child.handle, new_child.node_id);   
    },
    /**
     * Unregister at old parent of tree tree_id
     * Does not check if other trees have it as parent
     * IN: tree -> (new_parent, node_info)
     * OUT: parent nm -> (delete_child, node_info) 
     */
    [=](unregister_at_parent, actor parent, string tree_id) {           
      self->send(parent, delete_child::value, self->state.my_info.node_id, tree_id);
      self->demonitor(parent);               
    },
    /**
     * IN: local tree -> (new_backup, node_info)
     * OUT: remote nm -> (subscribe, actor, int)
     */
    [=](register_at_backup, node_info backup) {      
      self->send(backup.handle, subscribe_atom::value, actor_cast<actor>(self), self->state.my_info.node_id); 
    },
    /**
     * Delete sender of this message in tree with tree_id
     * IN: remote nm -> (delete_child, actor, string)
     * OUT: tree -> (delete_child, actor)
     */
    [=](delete_child, int child_node_id, string tree_id) {
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: delete_child";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE: delete_child");
      
      auto& sub = self->state.subscribers;
      auto sender = actor_cast<actor>( self->current_sender() );
      
      if( ! self->state.subscribers.empty() ) {
        auto sub_it = find_if(sub.begin(), sub.end(),  [=, &sender]( pair<actor, int> sub ) {
            return sub.first == sender;
        });              
        if( sub_it != sub.end() ) {          
          sub.erase(sub_it);
        }
      }      
      self->send(self->state.trees[tree_id], delete_child::value, actor_cast<actor>(self->current_sender()),
                child_node_id );
    },
    /*-------------------------------query handling------------------------------*/
    [=](query_atom, double qx, double qy) {      
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " ! RECEIVED QUERY";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "! RECEIVED QUERY");      

      self->request(self->state.trees["loc"], seconds(2), query_atom::value, qx, qy).then(
        [=](bool applies_to_subtree, list<node_info> children) {
          auto ts = system_clock().now().time_since_epoch().count();
          string log = to_string(ts);

          if(applies_to_subtree) {
            log += " ! QUERY APPLIES TO SUBTREE";
            // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "! QUERY APPLIES TO SUBTREE");
            for(auto& child : children) {
              self->send(child.handle, query_atom::value, qx, qy);
            }
          }
          else {
            log += " ! QUERY DOES NOT APPLY TO SUBTREE";
            // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "! QUERY DOES NOT APPLY TO SUBTREE");
          }
          self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
        }
      );
    },
    /**
     * IN: child nm -> (value)
     */
    [=](value res) {     
      // The code below is for logging sensor readings that arrive at root and the other nodes
      // Enable or disable based on your needs through the flags. (or just comment out)            
      system_clock::time_point arrival_tp = system_clock::now();      

      if(self->state.my_info.tier == 0 && self->state.log_tuples && ! self->state.log_for_failures) {
        // We want the latency only at root and we do not want it in the "failure" experiment
        auto latency = chrono::duration<double>(arrival_tp - res.ts).count();
        string log = to_string(arrival_tp.time_since_epoch().count()) + " DATA RECEIVED FROM " + to_string(res.node_id) 
                      + " Source-to-sink-latency [s] " + to_string(latency);
        self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), tuples::value, log);
      }
      else if (self->state.log_for_failures
                && res.node_id != self->state.my_info.node_id
                && self->state.my_info.tier != 0)
      { 
        // Parent nodes must log all tuples for the failure experiment        
        string log = to_string(arrival_tp.time_since_epoch().count()) + " DATA RECEIVED FROM " + to_string(res.node_id);
        self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), tuples::value, log);
      }
      self->request<message_priority::high>(self->state.trees[res.tree_id], seconds(2), 
                get_parent::value,
                get_level::value)
        .then([=](get_parent, get_level, actor parent, int my_level)
        {
          if(my_level > 0) {
            self->send(parent, res);            
          }
        }
      );
    },
    /**
     * Reading from own sensor
     * IN: Local SRT -> (value)
     */
    [=](value res, actor parent, int my_lvl) {
      system_clock::time_point arrival_tp = system_clock::now();      
      res.node_id = self->state.my_info.node_id;

      if(my_lvl > 0) {
        self->send(parent, res);         
      }
      else if (my_lvl == 0 && self->state.log_tuples) {
        // If root, log your own local readings
        auto latency = chrono::duration<double>(arrival_tp - res.ts).count();
        string log = to_string(arrival_tp.time_since_epoch().count()) + " DATA RECEIVED FROM " + to_string(res.node_id) 
                      + " Source-to-sink-latency [s] " + to_string(latency);
        self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), tuples::value, log);
      }
    },
    /*-------------------------------attribute meta data handling------------------------------*/
    /**
     * Update my_info with subtree range and local attribute measurement
     * Send updated my_info to all subscibers     
     * IN: local tree -> (string, string)
     * OU: remote nm -> (node_info)
     */
    [=](attribute local_att, subtree_range sub_range, string tree_id) {      
      self->state.my_info.sensors[tree_id] = local_att;
      self->state.my_info.subtree_ranges[tree_id] = sub_range;
      self->send(actor_cast<actor>(self), propagate_nodeinfo::value);      
    },
    /**
     * Own subtree range changed. Store it, propagate node_info to subscribers and notify parent
     * IN: tree ->update_atom, subtree_range)
     * OUT: self -> propagate_nodeinfo
     */
    [=](update_range, subtree_range sub_range, string tree_id, actor parent) {
      self->state.my_info.subtree_ranges[tree_id] = sub_range;
      self->send(actor_cast<actor>(self), propagate_nodeinfo::value);     
    },   
    /*-------------------------------utility------------------------------*/
    /**
     * from parent
     */
    [=](collect_tree) {
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: collect_tree from parent";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE: collect_tree from parent");
      
      self->request(self->state.trees["loc"], seconds(2), get_children::value).then(
        [=](list<node_info> children) {
          self->state.print_counter = children.size();

          // Self has children
          if(children.size() > 0) {
            for(auto& child : children) {
              self->send(child.handle, collect_tree::value);
            }
          }
          // Self has no children
          else {
            deque<pair<int, int>> d;
            self->send(actor_cast<actor>(self), collect_tree::value, d);
            self->state.print_counter = 1;
          }
        }
      );
    },
    /**
     * Subtree from one child
     * IN: Child NM
     */
    [=](collect_tree, deque<pair<int, int>> subtree) {      
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " RECEIVED MESSAGE: collect_tree from child";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "RECEIVED MESSAGE: collect_tree from child");

      self->state.print_counter--;      
      self->state.subtree.insert(self->state.subtree.end(), subtree.begin(), subtree.end());
      // Decrement counter for each received message, until it is 0
      if(self->state.print_counter == 0) {        
        self->request(self->state.trees["loc"], seconds(2), get_level::value).then(
          [=](int my_level) {
            self->state.subtree.push_front( make_pair(self->state.my_info.node_id, my_level) );
            
            if(my_level == 0) {
              // If root
              self->send(actor_cast<actor>(self), print_tree::value, self->state.subtree);

              // Start normal behavior only after intial tree was collected once
              self->send(self->state.trees["loc"], collected_once::value);

              // Get the current tree every x seconds
              self->delayed_send(actor_cast<actor>(self), seconds(15), collect_tree::value);
            }
            else {      
              // if not root        
              self->request(self->state.trees["loc"], seconds(2), get_parent::value).then(
                [=](get_parent, actor parent) {                
                  self->send(parent, collect_tree::value, self->state.subtree);
                  // Start normal behavior only after intial tree was collected once
                  self->send(self->state.trees["loc"], collected_once::value);                  
                  // This can be used if you want every node to log its subtree
                  self->send(actor_cast<actor>(self), print_tree::value, self->state.subtree);
                }
              );
            }
          }
        );
      }
      else ; // nop      
    },
    /**
     * Collected your whole subtree
     * Format of deque element: (first=node_id, second=level)
     */
    [=](print_tree, deque<pair<int, int>> tree) {      
      auto ts = system_clock().now().time_since_epoch().count();
      string log = to_string(ts) + " CURRENT TREE";
      self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), print_tree::value, log);
      // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "CURRENT TREE:");  
      
      string spaces = "";
      int c;
      for(auto& node : tree) {
        for(c=node.second; c >= 0; c--) {
          spaces = spaces + "--";
        }
        log = to_string(ts) + spaces + to_string(node.first);
        self->send(actor_cast<actor>(self->system().registry().get(atom("logger"))), print_tree::value, log);      
        // CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, spaces + to_string(node.first) );
        spaces = "";        
      }      
      deque<pair<int,int>> d;
      self->state.subtree = d;
    },
  };
}