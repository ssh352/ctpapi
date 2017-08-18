#ifndef BACKTESTING_STRATEGY_H
#define BACKTESTING_STRATEGY_H
#include "common/api_struct.h"
#include "event.h"
#include "execution_handler.h"

class InputOrderEvent : public AbstractEvent {
 public:
  InputOrderEvent(AbstractExecutionHandler* execution_handler,
                  PositionEffect position_effect,
                  OrderDirection order_direction,
                  int qty)
      : execution_handler_(execution_handler),
        position_effect_(position_effect),
        order_direction_(order_direction),
        qty_(qty) {}

  virtual void Do() override {
    execution_handler_->HandlerInputOrder(position_effect_, order_direction_,
                                          qty_);
  }

 private:
  AbstractExecutionHandler* execution_handler_;
  PositionEffect position_effect_;
  OrderDirection order_direction_;
  int qty_;
};

class AbstractStrategy {
 public:
  virtual void HandleTick(const std::shared_ptr<Tick>& tick) = 0;
  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) = 0;
};

class MyStrategy : public AbstractStrategy {
 public:
  MyStrategy(AbstractEventFactory* event_factory)
      : event_factory_(event_factory) {}

  virtual void HandleTick(const std::shared_ptr<Tick>& tick) override {
    if (!order_) {
      event_factory_->EnqueueInputOrderEvent(PositionEffect::kOpen,
                                             OrderDirection::kBuy, 10);
      order_ = true;
    }
  }

  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) override {}

 private:
  AbstractEventFactory* event_factory_;
  bool order_ = false;
};

#endif  // BACKTESTING_STRATEGY_H
