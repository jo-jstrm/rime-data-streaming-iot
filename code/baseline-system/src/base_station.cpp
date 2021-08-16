#include "base_station.hpp"

#include "caf/io/all.hpp"
#include "caf/logger.hpp"
#include "find_node_info.hpp"
#include "network_manager.hpp"

using namespace caf;
using namespace std;
using namespace std::chrono;

using query_atom = atom_constant<atom("queryatom")>;

#define CAF_LOG_LEVEL CAF_LOG_LEVEL_ERROR
#define CAF_LOG_COMPONENT "Base"

behavior base_station_actor(stateful_actor<BaseState>* self, string ip, int port, int id){  
  self->state.my_ip = ip;
  self->state.my_port = port;
  self->state.node_id = id;

  /*-------------------------------Message handlers------------------------------*/
  return{
    [=](query_atom, string root_ip, int root_port, double query_x, double query_y) {
      auto root = self->system().middleman().remote_actor(root_ip, root_port);

      if( ! root ) {
        CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "Could not reach root, trying again");
        self->delayed_send(actor_cast<actor>(self), seconds(5), query_atom::value, root_ip, root_port);
        return;
      }

      self->delayed_send(*root, seconds(25), query_atom::value, query_x, query_y);
      CAF_LOG_IMPL(CAF_LOG_COMPONENT, CAF_LOG_LEVEL, "! QUERY SENT TO ROOT (in 25s)");
    }
  };
}