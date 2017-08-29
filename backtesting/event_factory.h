#ifndef BACKTESTING_EVENT_FACTORY_H
#define BACKTESTING_EVENT_FACTORY_H
#include <memory>
#include "common/api_struct.h"

struct Tick;
class AbstractEvent;

class AbstractEventFactory {
 public:
  virtual void EnqueueTickEvent(
      const std::shared_ptr<TickData>& tick) const = 0;

  virtual void EnqueueFillEvent(
      const std::shared_ptr<OrderField>& order) const = 0;

  virtual void EnqueueInputOrderEvent(const std::string& instrument,
                                      PositionEffect position_effect,
                                      OrderDirection order_direction,
                                      double price,
                                      int qty) const = 0;

  virtual void EnqueueInputOrderSignal(const std::string& instrument,
                                       PositionEffect position_effect,
                                       OrderDirection order_direction,
                                       double price,
                                       int qty) const = 0;

  virtual void EnqueueCloseMarketEvent() = 0;
};

#endif  // BACKTESTING_EVENT_FACTORY_H
