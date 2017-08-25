#ifndef BACKTESTING_EXECUTION_HANDLER_H
#define BACKTESTING_EXECUTION_HANDLER_H
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include "tick_series_data_base.h"
#include "common/api_struct.h"

class AbstractExecutionHandler {
 public:
  virtual void HandleTick(const std::shared_ptr<TickData>& tick) = 0;

  virtual void HandlerInputOrder(const std::string& instrument,
                                 PositionEffect position_effect,
                                 OrderDirection direction,
                                 double price,
                                 int qty) = 0;
};

class SimulatedExecutionHandler : public AbstractExecutionHandler {
 public:
  SimulatedExecutionHandler(AbstractEventFactory* event_factory)
      : event_factory_(event_factory), orders_csv_("orders.csv") {}

  virtual void HandleTick(const std::shared_ptr<TickData>& tick) override {
    if (current_tick_ != nullptr) {
      boost::posix_time::ptime previous_pt(
          boost::gregorian::date(1970, 1, 1),
          boost::posix_time::milliseconds(current_tick_->timestamp));
      boost::posix_time::ptime pt(
          boost::gregorian::date(1970, 1, 1),
          boost::posix_time::milliseconds(tick->tick->timestamp));
      if (previous_pt.date() != pt.date()) {
        event_factory_->EnqueueCloseMarketEvent();
      }
    }
    current_tick_ = tick->tick;
  }

  virtual void HandlerInputOrder(const std::string& instrument,
                                 PositionEffect position_effect,
                                 OrderDirection direction,
                                 double price,
                                 int qty) override {
    std::string order_id = boost::lexical_cast<std::string>(++order_id_seq_);
    {
      auto order = std::make_shared<OrderField>();
      order->order_id = order_id;
      order->instrument_id = instrument;
      order->position_effect = position_effect;
      order->direction = direction;
      order->status = OrderStatus::kActive;
      order->price = GetFillPrice();
      order->avg_price = order->price;
      order->leaves_qty = qty;
      order->qty = qty;
      order->traded_qty = 0;
      event_factory_->EnqueueFillEvent(std::move(order));
    }
    {
      auto order = std::make_shared<OrderField>();
      order->order_id = order_id;
      order->instrument_id = instrument;
      order->position_effect = position_effect;
      order->direction = direction;
      order->status = OrderStatus::kAllFilled;
      order->price = GetFillPrice();
      order->avg_price = order->price;
      order->leaves_qty = 0;
      order->qty = qty;
      order->traded_qty = qty;
      event_factory_->EnqueueFillEvent(std::move(order));
      boost::posix_time::ptime pt(
          boost::gregorian::date(1970, 1, 1),
          boost::posix_time::milliseconds(current_tick_->timestamp));

      orders_csv_ << current_tick_->timestamp << ","
                  << boost::posix_time::to_simple_string(pt) << ","
                  << (position_effect == PositionEffect::kOpen ? "O" : "C")
                  << "," << (direction == OrderDirection::kBuy ? "B" : "S")
                  << "," << order->price << "," << qty << "\n";
    }
  }

 private:
  double GetFillPrice() const { return current_tick_->last_price; }
  std::shared_ptr<Tick> current_tick_;
  AbstractEventFactory* event_factory_;
  std::ofstream orders_csv_;
  uint64_t order_id_seq_ = 0.0;
};

#endif  // BACKTESTING_EXECUTION_HANDLER_H
