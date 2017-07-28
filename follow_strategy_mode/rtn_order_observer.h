#ifndef FOLLOW_STRATEGY_MODE_RTN_ORDER_OBSERVER_H
#define FOLLOW_STRATEGY_MODE_RTN_ORDER_OBSERVER_H
#include <boost/shared_ptr.hpp>
#include "common/api_struct.h"

class RtnOrderObserver {
 public:
  virtual void RtnOrder(const boost::shared_ptr<const OrderField>& order) = 0;
};

#endif  // FOLLOW_STRATEGY_MODE_RTN_ORDER_OBSERVER_H
