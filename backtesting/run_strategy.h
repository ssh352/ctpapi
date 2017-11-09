#ifndef BACKTESTING_RUN_STRATEGY_H
#define BACKTESTING_RUN_STRATEGY_H
#include "caf/all.hpp"

caf::behavior RunStrategy(caf::event_based_actor* self, caf::actor coor);

#endif  // BACKTESTING_RUN_STRATEGY_H
