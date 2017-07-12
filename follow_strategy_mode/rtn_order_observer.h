#ifndef FOLLOW_STRATEGY_MODE_RTN_ORDER_OBSERVER_H
#define FOLLOW_STRATEGY_MODE_RTN_ORDER_OBSERVER_H
#include "defines.h"

class RtnOrderObserver {
 public:
  virtual void RtnOrder(OrderData order) = 0;
};

#endif  // FOLLOW_STRATEGY_MODE_RTN_ORDER_OBSERVER_H
