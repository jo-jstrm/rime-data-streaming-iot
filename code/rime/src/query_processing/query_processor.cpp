#include "query_processor.hpp"

#include <chrono>
#include "sensor_random.hpp"

using namespace caf;
using namespace std;
using namespace std::chrono;

using read = atom_constant<atom("read")>;
using update = atom_constant<atom("update")>;
using down = atom_constant<atom("down")>;
using broadcast = atom_constant<atom("broadcast")>;

double READINGS_SIZE = 20;
double INIT_MAX = 99999.9;
double INIT_MIN = 0.0;

ProcessorState::ProcessorState() {}

behavior query_processor(stateful_actor<ProcessorState>* self) {
  //Init readings, sensor_meta  
  deque<double> tmp;
  for(int i=0;i<READINGS_SIZE;i++) {
    tmp.push_back(0.0);
  }
    
  return {
    /** 
     * Received local sensor reading
     * IN sensors.cpp
     * OUT: (if): self->update
     * Appends reading to "readings"-deque
     * If necessary, updates the ranges in sensor_meta and desc_ranges[self]
     */
    [=](pair<string, double> sensor_reading){
      string& name = sensor_reading.first;
      double& val = sensor_reading.second;
      //All stored readings for sensor "sensor_name".
      deque<double>& readings = self->state.readings.find(name)->second;      
      double& cur_min = self->state.sensor_meta[name].first;
      double& cur_max = self->state.sensor_meta[name].second;
      //Indicate changes to meta data caused by sensor_reading
      bool send = false;

      //Push reading to readings[name]
      readings.push_front(sensor_reading.second);
                      
      double popped = readings.back();
      readings.pop_back();    

      /**
       * If current max or min were popped, find new max or min in readings.
       */
      if(popped == cur_min){
        double new_min = cur_max;
        for(auto reading : readings){
          if(reading < new_min){
            new_min = reading;
          }
        }
        cur_min = new_min;
        send = true;
      }
      //Note: no "else if" here -> for edge case where max and min were the same.
      if(popped == cur_max){
        double new_max = cur_min;
        for(auto reading : readings){
          if(reading > new_max){
            new_max = reading;
          }
        }
        cur_max = new_max;
        send = true;
      }      
      
      /**
       * Update own min and max readings.
       * If local min/max changes, check if min/max of your subtree change as well
       */
      //New local min   
      if(val < cur_min){
        cur_min = val;
        send = true;        
      }
      //New local max.
      if(val > cur_max){
        cur_max = sensor_reading.second;
        send = true;
      }
      /**
       * If there were changes to own range: 
       * Updates sensor_meta.min/max as well as value in desc_ranges.
       */
      if(send){        
        self->send(actor_cast<actor>(self), update::value);         
      }

      //Send result to query tracker, which will further process it.
      actor tracker = actor_cast<actor>(self->system().registry().get(atom("tracker")));
      self->send(tracker, sensor_reading);
    }
  };
}