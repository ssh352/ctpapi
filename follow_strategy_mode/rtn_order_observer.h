#ifndef FOLLOW_STRATEGY_MODE_RTN_ORDER_OBSERVER_H
#define FOLLOW_STRATEGY_MODE_RTN_ORDER_OBSERVER_H
#include "common/api_struct.h"

class RtnOrderObserver {
 public:
  virtual void RtnOrder(OrderField order) = 0;
};

#endif  // FOLLOW_STRATEGY_MODE_RTN_ORDER_OBSERVER_H
