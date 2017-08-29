#include "strategy.h"

#include "cta_transaction_series_data_base.h"
MyStrategy::MyStrategy(
    AbstractEventFactory* event_factory,
    std::vector<std::pair<std::unique_ptr<CTATransaction[]>, int64_t>>
        cta_signal_container)
    : event_factory_(event_factory),
      keep_memory_(std::move(cta_signal_container)) {
  auto null_deleter = [](CTATransaction*) {};
  for (auto& item : keep_memory_) {
    for (int i = 0; i < item.second; ++i) {
      transactions_.push_back(
          std::shared_ptr<CTATransaction>(&item.first.get()[i], null_deleter));
    }
  }
  range_beg_it_ = transactions_.begin();
}

void MyStrategy::HandleTick(const std::shared_ptr<TickData>& tick) {
  if (range_beg_it_ == transactions_.end()) {
    return;
  }

  {
    auto end_it = std::upper_bound(
        delay_input_order_.begin(), delay_input_order_.end(),
        tick->tick->timestamp - 1 * 60 * 1000,
        [](TimeStamp timestamp, const std::shared_ptr<CTATransaction>& tran) {
          return timestamp < tran->timestamp;
        });
    if (end_it != delay_input_order_.begin()) {
      std::for_each(delay_input_order_.begin(), end_it,
                    [=](const std::shared_ptr<CTATransaction>& tran) {
                      event_factory_->EnqueueInputOrderSignal(
                          *tick->instrument,
                          tran->position_effect == 0 ? PositionEffect::kOpen
                                                     : PositionEffect::kClose,
                          tran->direction == 0 ? OrderDirection::kBuy
                                               : OrderDirection::kSell,
                          tran->price, tran->qty, tick->tick->timestamp);
                    });
      delay_input_order_.erase(delay_input_order_.begin(), end_it);
    }
  }

  auto end_it =
      std::upper_bound(range_beg_it_, transactions_.end(), tick,
                       [](const std::shared_ptr<TickData>& tick,
                          const std::shared_ptr<CTATransaction>& tran) {
                         return tick->tick->timestamp < tran->timestamp;
                       });

  for (auto i = range_beg_it_; i != end_it; ++i) {
    if ((*i)->position_effect == 0) {
      // Open
      delay_input_order_.push_back((*i));
    } else {
      event_factory_->EnqueueInputOrderSignal(
          *tick->instrument,
          (*i)->position_effect == 0 ? PositionEffect::kOpen
                                     : PositionEffect::kClose,
          (*i)->direction == 0 ? OrderDirection::kBuy : OrderDirection::kSell,
          (*i)->price, (*i)->qty, tick->tick->timestamp);
    }
  }

  range_beg_it_ = end_it;

  if (!unfill_orders_.empty()) {
    auto end_it =
        unfill_orders_.upper_bound(tick->tick->timestamp - 10 * 60 * 1000);
    std::for_each(unfill_orders_.begin(), end_it,
                  [=](const std::shared_ptr<OrderField>& order) {
                    event_factory_->EnqueueCancelOrderEvent(order->order_id);
                  });
  }
}

void MyStrategy::HandleOrder(const std::shared_ptr<OrderField>& order) {
  if (order->status == OrderStatus::kActive) {
    unfill_orders_.insert(order);
  } else if (order->status == OrderStatus::kAllFilled ||
             order->status == OrderStatus::kCanceled) {
    auto find_it = unfill_orders_.find(order->order_id);
    BOOST_ASSERT(find_it != unfill_orders_.end());
    unfill_orders_.erase(find_it);
  } else {
  }
}

// Inherited via AbstractStrategy

void MyStrategy::HandleCloseMarket() {}
