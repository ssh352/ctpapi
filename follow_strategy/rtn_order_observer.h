#ifndef follow_strategy_RTN_ORDER_OBSERVER_H
#define follow_strategy_RTN_ORDER_OBSERVER_H
#include <boost/shared_ptr.hpp>
#include "common/api_struct.h"

class RtnOrderObserver {
 public:
  virtual void RtnOrder(const std::shared_ptr<const OrderField>& order) = 0;
};

#endif  // follow_strategy_RTN_ORDER_OBSERVER_H
