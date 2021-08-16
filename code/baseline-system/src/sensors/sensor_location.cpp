#include "sensor_location.hpp"

#include <random>
#include <chrono>
#include <utility>

using namespace caf;
using namespace std;
using namespace std::chrono;

using read = atom_constant<atom("read")>;
using state = atom_constant<atom("state")>;

behavior location_sensor(stateful_actor<sensor_state_location>* self) {  
  self->state.name = "loc";
    
  // Generate random latitude and longitude
  system_clock::time_point begin = system_clock::now();      
  system_clock::duration dur;
  mt19937 gen;  
  uniform_real_distribution<double> dist(0, 100);  

  dur = system_clock::now() - begin;      
  gen.seed(dur.count());

  self->state.lat = dist(gen);

  dur = system_clock::now() - begin;
  gen.seed(dur.count());
  
  self->state.lon = dist(gen);  

  return {    
    [=](read) {
      // Generate random latitude and longitude
      system_clock::time_point begin = system_clock::now();      
      system_clock::duration dur;
      mt19937 gen;  
      uniform_real_distribution<double> dist(0, 100);  

      dur = system_clock::now() - begin;      
      gen.seed(dur.count());

      self->state.lat = dist(gen);

      dur = system_clock::now() - begin;
      gen.seed(dur.count());
      
      self->state.lon = dist(gen);

      return make_result(self->state.name, self->state.lat, self->state.lon);      
    },
    /*-------------------------------Testing------------------------------*/
    [=](state) {
      return make_result(self->state.lat, self->state.lon);
    }    
  };
 }