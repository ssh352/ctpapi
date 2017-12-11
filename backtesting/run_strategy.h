#ifndef BACKTESTING_RUN_STRATEGY_H
#define BACKTESTING_RUN_STRATEGY_H
#include "caf/all.hpp"
#include "follow_strategy/product_info_manager.h"

caf::behavior RunStrategy(caf::event_based_actor* self,
                          caf::actor coor,
                          ProductInfoMananger* product_info_mananger,
                          bool cancel_limit_order_when_switch_trade_date);

#endif  // BACKTESTING_RUN_STRATEGY_H
