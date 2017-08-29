#ifndef BACKTESTING_EXECUTION_HANDLER_H
#define BACKTESTING_EXECUTION_HANDLER_H
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <boost/assert.hpp>
#include "tick_series_data_base.h"
#include "common/api_struct.h"
#include "event_factory.h"

struct LimitOrder {
  std::string instrument;
  std::string order_id;
  OrderDirection direction;
  PositionEffect position_effect;
  double price;
  int qty;
};

template <class KeyCompare>
class ComparePrice {
 public:
  using is_transparent = void;

  bool operator()(const LimitOrder& l, const LimitOrder& r) const {
    return compare_(l.price, r.price);
  }

  bool operator()(double price, const LimitOrder& r) const {
    return compare_(price, r.price);
  }

  bool operator()(const LimitOrder& l, double price) const {
    return compare_(l.price, price);
  }

 private:
  KeyCompare compare_;
};

class AbstractExecutionHandler {
 public:
  virtual void HandleTick(const std::shared_ptr<TickData>& tick) = 0;

  virtual void HandlerInputOrder(const std::string& instrument,
                                 PositionEffect position_effect,
                                 OrderDirection direction,
                                 double price,
                                 int qty,
                                 TimeStamp timestamp) = 0;
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

    if (!long_limit_orders_.empty()) {
      auto end_it = long_limit_orders_.upper_bound(tick->tick->last_price);
      std::for_each(
          long_limit_orders_.begin(), end_it, [=](const LimitOrder& lo) {
            EnqueueFillEvent(lo.instrument, lo.order_id, lo.direction,
                             lo.position_effect, lo.price, lo.price, lo.qty);
          });
      long_limit_orders_.erase(long_limit_orders_.begin(), end_it);
    }

    if (!short_limit_orders_.empty()) {
      auto end_it = short_limit_orders_.upper_bound(tick->tick->last_price);
      std::for_each(
          short_limit_orders_.begin(), end_it, [=](const LimitOrder& lo) {
            EnqueueFillEvent(lo.instrument, lo.order_id, lo.direction,
                             lo.position_effect, lo.price, lo.price, lo.qty);
          });
      short_limit_orders_.erase(short_limit_orders_.begin(), end_it);
    }
  }

  virtual void HandlerInputOrder(const std::string& instrument,
                                 PositionEffect position_effect,
                                 OrderDirection direction,
                                 double price,
                                 int qty,
                                 TimeStamp timestamp) override {
    std::string order_id = boost::lexical_cast<std::string>(++order_id_seq_);
    if (direction == OrderDirection::kBuy) {
      long_limit_orders_.insert(
          {instrument, order_id, direction, position_effect, price, qty});
    } else {
      short_limit_orders_.insert(
          {instrument, order_id, direction, position_effect, price, qty});
    }
    auto order = std::make_shared<OrderField>();
    order->order_id = order_id;
    order->instrument_id = instrument;
    order->position_effect = position_effect;
    order->direction = direction;
    order->status = OrderStatus::kActive;
    order->price = price;
    order->avg_price = price;
    order->leaves_qty = qty;
    order->qty = qty;
    order->traded_qty = 0;
    event_factory_->EnqueueFillEvent(std::move(order));
    orders_csv_ << timestamp << ","
                << (position_effect == PositionEffect::kOpen ? "O" : "C") << ","
                << (direction == OrderDirection::kBuy ? "B" : "S") << ","
                << static_cast<int>(OrderStatus::kActive) << "," << order->price
                << "," << qty << "\n";
  }

  void HandleCancelOrder(const std::string& order_id) {
    auto find_it =
        std::find_if(long_limit_orders_.begin(), long_limit_orders_.end(),
                     [=](const LimitOrder& limit_order) {
                       return limit_order.order_id == order_id;
                     });

    if (find_it != long_limit_orders_.end()) {
      long_limit_orders_.erase(find_it);
      EnqueueCancelOrderEvent(*find_it);
      return;
    }

    find_it =
        std::find_if(short_limit_orders_.begin(), short_limit_orders_.end(),
                     [=](const LimitOrder& limit_order) {
                       return limit_order.order_id == order_id;
                     });
    if (find_it != short_limit_orders_.end()) {
      EnqueueCancelOrderEvent(*find_it);
      short_limit_orders_.erase(find_it);
      return;
    }

    BOOST_ASSERT(false);
    return;
  }

  void EnqueueFillEvent(const std::string& instrument,
                        const std::string& order_id,
                        OrderDirection direction,
                        PositionEffect position_effect,
                        double input_price,
                        double price,
                        int qty) {
    {
      auto order = std::make_shared<OrderField>();
      order->order_id = order_id;
      order->instrument_id = instrument;
      order->position_effect = position_effect;
      order->direction = direction;
      order->status = OrderStatus::kAllFilled;
      order->price = price;
      order->avg_price = price;
      order->leaves_qty = 0;
      order->qty = qty;
      order->traded_qty = qty;
      event_factory_->EnqueueFillEvent(std::move(order));
      orders_csv_ << current_tick_->timestamp << ","
                  << (position_effect == PositionEffect::kOpen ? "O" : "C")
                  << "," << (direction == OrderDirection::kBuy ? "B" : "S")
                  << "," << static_cast<int>(OrderStatus::kAllFilled) << ","
                  << order->price << "," << qty << "\n";
    }
  }

  void EnqueueCancelOrderEvent(const LimitOrder& limit_order) {
    auto order = std::make_shared<OrderField>();
    order->order_id = limit_order.order_id;
    order->instrument_id = limit_order.instrument;
    order->position_effect = limit_order.position_effect;
    order->direction = limit_order.direction;
    order->status = OrderStatus::kCanceled;
    order->price = limit_order.price;
    order->avg_price = limit_order.price;
    order->leaves_qty = limit_order.qty;
    order->qty = limit_order.price;
    order->traded_qty = 0;
    event_factory_->EnqueueFillEvent(std::move(order));

    orders_csv_ << current_tick_->timestamp << ","
                << (limit_order.position_effect == PositionEffect::kOpen ? "O"
                                                                         : "C")
                << ","
                << (limit_order.direction == OrderDirection::kBuy ? "B" : "S")
                << "," << static_cast<int>(OrderStatus::kCanceled) << ","
                << order->price << "," << limit_order.qty << "\n";
  }

 private:
  double GetFillPrice() const { return current_tick_->last_price; }
  std::shared_ptr<Tick> current_tick_;
  AbstractEventFactory* event_factory_;
  std::ofstream orders_csv_;
  uint64_t order_id_seq_ = 0.0;
  std::multiset<LimitOrder, ComparePrice<std::greater<double>>>
      long_limit_orders_;
  std::multiset<LimitOrder, ComparePrice<std::less<double>>>
      short_limit_orders_;
};

#endif  // BACKTESTING_EXECUTION_HANDLER_H
