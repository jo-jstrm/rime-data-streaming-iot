//pinger.cpp
#include "pinger.hpp"

#include <chrono>
#include "caf/io/all.hpp"

#define PING_ATTEMPTS 11

using namespace std;
using namespace caf;
using namespace std::chrono;

behavior pinger(blocking_actor* self){
  return {
    [=](actor pingee) {            
      int64_t ping = 0;
      for(short i = 0; i < PING_ATTEMPTS; i++) {
        auto begin = system_clock::now();

        self->request<message_priority::high>(pingee, seconds(1), ping_atom::value).receive(
          [=, &ping](ping_atom) {
            auto diff = (system_clock::now() - begin); 
            ping += diff.count();
          },
          [=](const error& err) {            
            return err;
          }
        );      
      }
      return ping/PING_ATTEMPTS;
    }
  };
}