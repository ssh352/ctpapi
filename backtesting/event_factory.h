#ifndef BACKTESTING_EVENT_FACTORY_H
#define BACKTESTING_EVENT_FACTORY_H
#include <memory>
#include "common/api_struct.h"

class AbstractEventFactory {
 public:
  virtual void EnqueueTickEvent(
      const std::shared_ptr<TickData>& tick) const = 0;

  virtual void EnqueueRtnOrderEvent(
      const std::shared_ptr<OrderField>& order) const = 0;

  virtual void EnqueueInputOrderEvent(const InputOrder& input_order) const = 0;

  virtual void EnqueueInputOrderSignal(const InputOrder& input_order) const = 0;

  virtual void EnqueueCancelOrderEvent(const std::string& order_id) = 0;

  virtual void EnqueueCloseMarketEvent() = 0;
};

#endif  // BACKTESTING_EVENT_FACTORY_H
