#ifndef BACKTESTING_STRATEGY_H
#define BACKTESTING_STRATEGY_H
#include <boost/format.hpp>
#include <list>
#include <set>
#include "common/api_struct.h"

template <class MailBox>
class MyStrategy {
 public:
  MyStrategy(MailBox* mail_box,
             std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>
                 cta_signal_container,
             int delayed_input_order_minute,
             int cancel_order_after_minute,
             int backtesting_position_effect)
      : mail_box_(mail_box),
        keep_memory_(std::move(cta_signal_container)),
        delayed_input_order_minute_(delayed_input_order_minute),
        cancel_order_after_minute_(cancel_order_after_minute),
        backtesting_position_effect_(backtesting_position_effect) {
    auto null_deleter = [](CTATransaction*) {};
    for (auto& item : keep_memory_) {
      for (int i = 0; i < item.second; ++i) {
        transactions_.push_back(std::shared_ptr<CTATransaction>(
            &item.first.get()[i], null_deleter));
      }
    }
    range_beg_it_ = transactions_.begin();

    mail_box_->Subscribe(&MyStrategy::HandleTick, this);
    mail_box_->Subscribe(&MyStrategy::HandleOrder, this);
  }

  void HandleTick(const std::shared_ptr<TickData>& tick) {
    // TODO:Do
    if (range_beg_it_ == transactions_.end()) {
      return;
    }

    {
      auto end_it = std::upper_bound(
          delay_input_order_.begin(), delay_input_order_.end(),
          tick->tick->timestamp - delayed_input_order_minute_ * 1000,
          [](TimeStamp timestamp, const std::shared_ptr<CTATransaction>& tran) {
            return timestamp < tran->timestamp;
          });
      if (end_it != delay_input_order_.begin()) {
        std::for_each(delay_input_order_.begin(), end_it,
                      [=](const std::shared_ptr<CTATransaction>& tran) {
                        mail_box_->Send(InputOrderBacktesting{
                            *tick->instrument,
                            tran->position_effect == 0 ? PositionEffect::kOpen
                                                       : PositionEffect::kClose,
                            tran->direction == 0 ? OrderDirection::kBuy
                                                 : OrderDirection::kSell,
                            tran->price, tran->qty, tick->tick->timestamp});
                      });
        delay_input_order_.erase(delay_input_order_.begin(), end_it);
      }
    }

    auto end_it =
        std::find_if(range_beg_it_, transactions_.end(),
                     [=](const std::shared_ptr<CTATransaction>& tran) {
                       return tick->tick->timestamp < tran->timestamp;
                     });

    for (auto i = range_beg_it_; i != end_it; ++i) {
      if ((*i)->position_effect == backtesting_position_effect_) {
        delay_input_order_.push_back((*i));
      } else {
        mail_box_->Send(InputOrderBacktesting{
            *tick->instrument,
            (*i)->position_effect == 0 ? PositionEffect::kOpen
                                       : PositionEffect::kClose,
            (*i)->direction == 0 ? OrderDirection::kBuy : OrderDirection::kSell,
            (*i)->price, (*i)->qty, tick->tick->timestamp});
      }
    }

    range_beg_it_ = end_it;

    if (!unfill_orders_.empty()) {
      auto end_it = unfill_orders_.upper_bound(
          tick->tick->timestamp - cancel_order_after_minute_ * 1000);
      std::for_each(unfill_orders_.begin(), end_it,
                    [=](const std::shared_ptr<OrderField>& order) {
                      mail_box_->Send(CancelOrder{order->order_id});
                    });
    }
  }

  void HandleOrder(const std::shared_ptr<OrderField>& order) {
    OrderStatus staus = OrderStatus::kActive;
    if (order->status == staus) {
      unfill_orders_.insert(order);
    } else if (order->status == OrderStatus::kAllFilled ||
               order->status == OrderStatus::kCanceled) {
      auto find_it = unfill_orders_.find(order->order_id);
      BOOST_ASSERT(find_it != unfill_orders_.end());
      unfill_orders_.erase(find_it);
    } else {
    }
  }

 private:
  class CompareOrderId {
   public:
    using is_transparent = void;
    bool operator()(const std::shared_ptr<OrderField>& l,
                    const std::shared_ptr<OrderField>& r) const {
      return l->order_id < r->order_id;
    }

    bool operator()(const std::string& order_id,
                    const std::shared_ptr<OrderField>& r) const {
      return order_id < r->order_id;
    }

    bool operator()(const std::shared_ptr<OrderField>& l,
                    const std::string& order_id) const {
      return l->order_id < order_id;
    }
    bool operator()(const std::shared_ptr<OrderField>& l,
                    TimeStamp timestamp) const {
      return l->input_timestamp < timestamp;
    }
    bool operator()(TimeStamp timestamp,
                    const std::shared_ptr<OrderField>& l) const {
      return timestamp < l->input_timestamp;
    }
  };
  MailBox* mail_box_;
  std::list<std::shared_ptr<CTATransaction>> transactions_;
  std::list<std::shared_ptr<CTATransaction>>::iterator range_beg_it_;
  std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>> keep_memory_;

  std::list<std::shared_ptr<CTATransaction>> delay_input_order_;

  std::set<std::shared_ptr<OrderField>, CompareOrderId> unfill_orders_;

  int delayed_input_order_minute_ = 0;
  int cancel_order_after_minute_ = 0;
  int backtesting_position_effect_ = 0;
};

#endif  // BACKTESTING_STRATEGY_H
