#ifndef BACKTESTING_EXECUTION_HANDLER_H
#define BACKTESTING_EXECUTION_HANDLER_H
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assert.hpp>
#include <set>
#include <boost/unordered_set.hpp>
#include <functional>
#include "common/api_struct.h"
#include "hpt_core/tick_series_data_base.h"
#include "hpt_core/order_util.h"

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
template <class MailBox>
class SimulatedExecutionHandler {
 public:
  SimulatedExecutionHandler(MailBox* mail_box) : mail_box_(mail_box) {
    mail_box_->Subscribe(&SimulatedExecutionHandler::HandlerInputOrder, this);
    mail_box_->Subscribe(&SimulatedExecutionHandler::HandleTick, this);
    mail_box_->Subscribe(&SimulatedExecutionHandler::HandleCancelOrder, this);
    mail_box_->Subscribe(&SimulatedExecutionHandler::HandleActionOrder, this);
    mail_box_->Subscribe(&SimulatedExecutionHandler::BeforeTrading, this);
  }

  void BeforeTrading(const BeforeTradingAtom&,
                     const TradingTime& trading_time) {
    // TODO:Debug
    if (!long_limit_orders_.empty() || !short_limit_orders_.empty()) {
      int i = 0;
    }
    long_limit_orders_.clear();
    short_limit_orders_.clear();
  }

  void HandleTick(const std::shared_ptr<TickData>& tick) {
    current_tick_ = tick->tick;

    if (!long_limit_orders_.empty()) {
      auto end_it = long_limit_orders_.upper_bound(tick->tick->last_price);
      std::for_each(
          long_limit_orders_.begin(), end_it, [=](const LimitOrder& lo) {
            EnqueueRtnOrderEvent(lo, OrderStatus::kAllFilled,
                                 std::min(lo.price, tick->tick->ask_price1), 0,
                                 lo.qty);
          });
      long_limit_orders_.erase(long_limit_orders_.begin(), end_it);
    }

    if (!short_limit_orders_.empty()) {
      auto end_it = short_limit_orders_.upper_bound(tick->tick->last_price);
      std::for_each(
          short_limit_orders_.begin(), end_it, [=](const LimitOrder& lo) {
            EnqueueRtnOrderEvent(lo, OrderStatus::kAllFilled,
                                 std::max(lo.price, tick->tick->bid_price1), 0,
                                 lo.qty);
          });
      short_limit_orders_.erase(short_limit_orders_.begin(), end_it);
    }
  }

  void HandlerInputOrder(const InputOrderSignal& input_order) {
    std::string order_id = input_order.order_id;
    if (input_order.direction == OrderDirection::kBuy) {
      long_limit_orders_.insert(
          {input_order.instrument, order_id, input_order.direction,
           input_order.position_effect, input_order.price, input_order.qty});
    } else {
      short_limit_orders_.insert(
          {input_order.instrument, order_id, input_order.direction,
           input_order.position_effect, input_order.price, input_order.qty});
    }

    auto order = std::make_shared<OrderField>();
    order->order_id = order_id;
    // order->strategy_id = input_order.strategy_id;

    order->instrument_id = input_order.instrument;
    order->position_effect = input_order.position_effect;
    order->direction = input_order.direction;
    order->position_effect_direction = AdjustDirectionByPositionEffect(
        input_order.position_effect, input_order.direction);
    order->status = OrderStatus::kActive;
    order->input_price = input_order.price;
    order->avg_price = input_order.price;
    order->leaves_qty = input_order.qty;
    order->qty = input_order.qty;
    order->trading_qty = 0;
    // TODO:
    // order->input_timestamp = input_order.timestamp_;
    // order->update_timestamp = input_order.timestamp_;
    order->input_timestamp =
        (current_tick_ != nullptr ? current_tick_->timestamp : 0);
    order->update_timestamp =
        (current_tick_ != nullptr ? current_tick_->timestamp : 0);
    orders_.insert(order);
    mail_box_->Send(std::move(order));
  }

  void HandleCancelOrder(const CancelOrder& cancel_order) {
    const std::string& order_id = cancel_order.order_id;
    auto find_it =
        std::find_if(long_limit_orders_.begin(), long_limit_orders_.end(),
                     [=](const LimitOrder& limit_order) {
                       return limit_order.order_id == order_id;
                     });

    if (find_it != long_limit_orders_.end()) {
      EnqueueRtnOrderEvent(*find_it, OrderStatus::kCanceled, find_it->price,
                           find_it->qty, 0);
      long_limit_orders_.erase(find_it);
      return;
    }

    find_it =
        std::find_if(short_limit_orders_.begin(), short_limit_orders_.end(),
                     [=](const LimitOrder& limit_order) {
                       return limit_order.order_id == order_id;
                     });
    if (find_it != short_limit_orders_.end()) {
      EnqueueRtnOrderEvent(*find_it, OrderStatus::kCanceled, find_it->price,
                           find_it->qty, 0);
      short_limit_orders_.erase(find_it);
      return;
    }

    auto it = orders_.find(cancel_order.order_id, OrderFieldHask(),
                           OrderFieldEqualTo());

    BOOST_ASSERT(it != orders_.end());

    //
    // std::string instrument;
    // std::string order_id;
    // std::string strategy_id;
    // orderDirection position_effect_direction;
    // positionEffect position_effect;
    // double price;
    // int qty;

    EnqueueRtnOrderEvent(
        {(*it)->instrument_id, (*it)->order_id, (*it)->direction,
         (*it)->position_effect, (*it)->input_price, (*it)->qty},
        OrderStatus::kCancelRejected, (*it)->input_price, (*it)->qty, 0);

    return;
  }

  void SimulatedExecutionHandler::HandleActionOrder(
      const OrderAction& action_order) {
    const std::string& order_id = action_order.order_id;
    auto find_it =
        std::find_if(long_limit_orders_.begin(), long_limit_orders_.end(),
                     [=](const LimitOrder& limit_order) {
                       return limit_order.order_id == order_id;
                     });

    if (find_it != long_limit_orders_.end()) {
      LimitOrder lo = *find_it;
      if (action_order.new_price != action_order.old_price) {
        lo.price = action_order.new_price;
      }

      if (action_order.old_qty != action_order.new_qty) {
        lo.qty = action_order.new_qty;
      }

      EnqueueRtnOrderEvent(lo, OrderStatus::kActive, lo.price, lo.qty, 0);

      long_limit_orders_.erase(find_it);
      long_limit_orders_.insert(std::move(lo));
      return;
    }

    find_it =
        std::find_if(short_limit_orders_.begin(), short_limit_orders_.end(),
                     [=](const LimitOrder& limit_order) {
                       return limit_order.order_id == order_id;
                     });
    if (find_it != short_limit_orders_.end()) {
      LimitOrder lo = *find_it;
      if (action_order.new_price != action_order.old_price) {
        lo.price = action_order.new_price;
      }

      if (action_order.old_qty != action_order.new_qty) {
        lo.qty = action_order.new_qty;
      }

      EnqueueRtnOrderEvent(lo, OrderStatus::kActive, lo.price, lo.qty, 0);

      short_limit_orders_.erase(find_it);
      short_limit_orders_.insert(std::move(lo));
      return;
    }

    auto it = orders_.find(action_order.order_id, OrderFieldHask(),
                           OrderFieldEqualTo());

    BOOST_ASSERT(it != orders_.end());

    EnqueueRtnOrderEvent(
        {(*it)->instrument_id, (*it)->order_id,
         (*it)->position_effect_direction, (*it)->position_effect,
         (*it)->input_price, (*it)->qty},
        OrderStatus::kActionRejected, (*it)->input_price, (*it)->qty, 0);
  }

  void EnqueueRtnOrderEvent(const LimitOrder& limit_order,
                            OrderStatus order_status,
                            double price,
                            int leaves_qty,
                            int traded_qty) {
    auto order = std::make_shared<OrderField>();
    order->order_id = limit_order.order_id;
    order->instrument_id = limit_order.instrument;
    order->position_effect = limit_order.position_effect;
    order->direction = limit_order.direction;
    order->position_effect_direction = AdjustDirectionByPositionEffect(
        limit_order.position_effect, limit_order.direction);
    order->status = order_status;
    order->input_price = price;
    order->avg_price = price;
    order->leaves_qty = leaves_qty;
    order->qty = limit_order.qty;
    order->trading_qty = traded_qty;
    order->update_timestamp = current_tick_->timestamp;

    orders_.insert(order);

    mail_box_->Send(std::move(order));
  }

 private:
  double GetFillPrice() const { return current_tick_->last_price; }

  struct OrderFieldHask {
    using is_transparent = void;
    size_t operator()(const std::shared_ptr<OrderField>& order) const {
      return std::hash<std::string>()(order->order_id);
    }

    size_t operator()(const std::string& order_id) const {
      return std::hash<std::string>()(order_id);
    }
  };

  struct OrderFieldEqualTo {
    using is_transparent = void;
    bool operator()(const std::shared_ptr<OrderField>& l,
                    const std::shared_ptr<OrderField>& r) const {
      return l->order_id == r->order_id;
    }

    bool operator()(const std::string& l,
                    const std::shared_ptr<OrderField>& r) const {
      return l == r->order_id;
    }
  };

  std::shared_ptr<Tick> current_tick_;
  MailBox* mail_box_;
  uint64_t order_id_seq_ = 0.0;
  std::multiset<LimitOrder, ComparePrice<std::greater<double>>>
      long_limit_orders_;
  std::multiset<LimitOrder, ComparePrice<std::less<double>>>
      short_limit_orders_;

  boost::unordered_set<std::shared_ptr<OrderField>,
                       OrderFieldHask,
                       OrderFieldEqualTo>
      orders_;
};

#endif  // BACKTESTING_EXECUTION_HANDLER_H
