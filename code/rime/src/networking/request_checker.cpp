#include "request_checker.hpp"

#include <string>

using namespace std;
using namespace std::chrono;
using namespace caf;

using build_request = atom_constant<atom("build")>;

rc_state::rc_state() : open_responses(0) {}

behavior request_checker(stateful_actor<rc_state>* self) {
  return {
    /**
     * Forwards the request to all trees and awaits all results before returning
     * IN: local network_manager -> (delete_child, actor, tree_id)
     * RETURN: bool is_needed = is the actor needed in any tree?
     */
    [=](map<string, actor> trees, actor to_delete) {
      bool is_needed_from_anyone;
      actor sender = actor_cast<actor>( self->current_sender() );
      for (auto& tree : trees) {
        self->request(tree.second, seconds(2), delete_atom::value, to_delete).await(
          [=, &is_needed_from_anyone](bool needed) {
            if(needed) {
              is_needed_from_anyone = true;
            }            
            self->state.open_responses--;
          }
        );
      }     
    }
  };
}