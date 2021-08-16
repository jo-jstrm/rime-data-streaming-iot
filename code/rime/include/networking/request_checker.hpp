#ifndef _GUARD_REQUESTCHECKER_H
#define _GUARD_REQUESTCHECKER_H

#include <vector>
#include "caf/all.hpp"

struct rc_state {
  int open_responses;

  rc_state();
};

caf::behavior request_checker(caf::stateful_actor<rc_state>*);

#endif