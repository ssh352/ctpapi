#ifndef BACKTESTING_RUN_STRATEGY_H
#define BACKTESTING_RUN_STRATEGY_H
#include "caf/all.hpp"

caf::behavior RunStrategy(caf::event_based_actor* self,
                          caf::actor coor,
                          bool cancel_limit_order_when_switch_trade_date);

#endif  // BACKTESTING_RUN_STRATEGY_H
