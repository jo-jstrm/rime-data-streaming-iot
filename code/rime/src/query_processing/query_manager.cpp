#include "query_manager.hpp"

#include "results.hpp"

using namespace caf;
using namespace std;
using namespace std::chrono;

using timer = atom_constant<atom("timer")>;
using fired = atom_constant<atom("fired")>;
using broadcast = atom_constant<atom("broadcast")>;

behavior query_manager(stateful_actor<query_managerState>* self){
  return{
    /**
     * New query 
     * IN Networker->(query)
     * OUT:
     *    (if new) tracker_timer->(query)
     *    (if new) networker->(broadcast,query)
     *    (if new) processor->(string,timepoint)
     */
    [=](query query){     
      auto& queries = self->state.queries;      
      auto it = find(queries.begin(), queries.end(), query);
      
      //If new query, store/track it and tell networker to broadcast it.
      if(it == queries.end()){
        queries.push_back(query);      
        actor timer = self->system().spawn(tracker_timer);
        self->send(timer, query);        
        actor processor = actor_cast<actor>(self->system().registry().get(atom("processor")));
        self->send(actor_cast<actor>(self->current_sender()),broadcast::value, query);       
      }      
    },
    /**
     * query duration is over, so delete it.
     * IN tracker_timer->(time, int, timepoint)
     * OUT: -
     */
    [=](fired, query over){
      auto& queries = self->state.queries;

      //TODO continue here
      auto it = find(queries.begin(), queries.end(), over);
      queries.erase(it);
    },
    /**
     * When a reading arrives from query processor, generate a result for each query that needs it
     * and send it to networker.
     * IN query_processor->(pair<string, double>)
     * OUT: networker->(value)
     * message: pair<sensor-name,sensor-reading>
     */
    [=](pair<string,double> sensor_reading){
      auto& queries = self->state.queries;
      actor nw = actor_cast<actor>(self->system().registry().get(atom("networker")));
      actor slf = actor_cast<actor>(self);

      for(auto q : queries){
        if(q.sensor_name == sensor_reading.first){
          
          //self->send(nw, broadcast::value, value(sensor_reading.second,sensor_reading.first,slf,q.root,q.arrival_time));
        }
      }
    }   
  };
}

behavior tracker_timer(event_based_actor* self){
  return{
    [=](query query){
      auto duration = milliseconds(query.duration);
      
      auto delay = duration - ( system_clock::now() - query.arrival_time );

      self->delayed_send(actor_cast<actor>(self->current_sender()), delay, fired::value, query);      
    }
  };
}