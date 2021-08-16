#include "sensor_random.hpp"

#include <random>
#include <chrono>
#include <utility>
#include "results.hpp"

using namespace caf;
using namespace std;
using namespace std::chrono;

using read = atom_constant<atom("read")>;
using state = atom_constant<atom("state")>;

behavior random_sensor(stateful_actor<sensor_state>* self, string name) {  
  self->state.name = name;  
  
  return {
    // RETURN: (string sensor_name, double sensor reading)     
    [=](read) {
      double sensor_reading;      

      system_clock::time_point begin = system_clock::now();      
      system_clock::duration dur;
      mt19937 gen;
      uniform_real_distribution<double> dist(0, 100);

      //seed the generator
      dur = system_clock::now() - begin;      
      gen.seed(dur.count());

      sensor_reading = dist(gen);
      
      return value(sensor_reading);      
    },
    /*-------------------------------Testing------------------------------*/
    [=](state) {
      return self->state.name;
    }    
  };
 }