#ifndef BACKTESTING_EXECUTION_HANDLER_H
#define BACKTESTING_EXECUTION_HANDLER_H
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "tick_series_data_base.h"
#include "common/api_struct.h"

class AbstractExecutionHandler {
 public:
  virtual void HandleTick(const std::shared_ptr<Tick>& tick) = 0;

  virtual void HandlerInputOrder(PositionEffect position_effect,
                                 OrderDirection direction,
                                 int qty) = 0;
};

class SimulatedExecutionHandler : public AbstractExecutionHandler {
 public:
  SimulatedExecutionHandler(AbstractEventFactory* event_factory)
      : event_factory_(event_factory) {}

  virtual void HandleTick(const std::shared_ptr<Tick>& tick) override {
    if (current_tick_ != nullptr) {
      boost::posix_time::ptime previous_pt(
          boost::gregorian::date(1970, 1, 1),
          boost::posix_time::milliseconds(current_tick_->timestamp));
      boost::posix_time::ptime pt(
          boost::gregorian::date(1970, 1, 1),
          boost::posix_time::milliseconds(tick->timestamp));
      if (previous_pt.date() != pt.date()) {
        event_factory_->EnqueueCloseMarketEvent();
      }
    }
    current_tick_ = tick;
  }

  virtual void HandlerInputOrder(PositionEffect position_effect,
                                 OrderDirection direction,
                                 int qty) override {
    auto order = std::make_shared<OrderField>();
    order->position_effect = position_effect;
    order->direction = direction;
    order->status = OrderStatus::kAllFilled;
    order->price = GetFillPrice();
    order->avg_price = order->price;
    order->leaves_qty = 0;
    order->qty = qty;
    order->traded_qty = qty;
    event_factory_->EnqueueFillEvent(std::move(order));
  }

 private:
  double GetFillPrice() const {
    return (current_tick_->ask_price1 + current_tick_->bid_price1) / 2;
  }
  std::shared_ptr<Tick> current_tick_;
  AbstractEventFactory* event_factory_;
};

#endif  // BACKTESTING_EXECUTION_HANDLER_H
