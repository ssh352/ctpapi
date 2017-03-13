#include "instrument_follow.h"

InstrumentFollow::InstrumentFollow() {
  order_direction_ = kODUnkown;
}

void InstrumentFollow::HandleOrderRtnForTrader(
    const OrderRtnData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  if (order.order_status == OrderStatus::kOSOpening) {
    if (order_direction_ != kODUnkown &&
        order.order_direction != order_direction_) {
      // Open Reverse Order
      int volume = CalcOrderReverseVolume(order.volume);

      if (volume > 0) {
        enter_order->order_no = order.order_no;
        enter_order->instrument = order.instrument;
        enter_order->order_direction = order.order_direction;
        enter_order->order_price = order.order_price;
        enter_order->volume = volume;
        enter_order->action = EnterOrderAction::kEOAOpen;
        order_follows_.push_back(
            OrderFollow{order.order_no, order.volume, order.order_direction});
      }
    } else {
      enter_order->order_no = order.order_no;
      enter_order->instrument = order.instrument;
      enter_order->order_direction = order.order_direction;
      enter_order->order_price = order.order_price;
      enter_order->volume = order.volume;
      enter_order->action = EnterOrderAction::kEOAOpen;
      order_follows_.push_back(
          OrderFollow{order.order_no, order.volume, order.order_direction});
    }
  } else if (order.order_status == OrderStatus::kOSOpened) {
    auto it = std::find_if(
        order_follows_.begin(), order_follows_.end(),
        [&](auto follow) { return follow.trade_order_no() == order.order_no; });
    if (it != order_follows_.end()) {
      it->FillOpenOrderForTrade(order.volume);
    } else {
      // ASSERT(FALSE)
    }
    ResetOrderDirectionIfNeed(order);
  } else if (order.order_status == OrderStatus::kOSCloseing) {
    int outstanding_close_volume = order.volume;
    int close_volume = 0;
    for (auto& follow : order_follows_) {
      int follow_close_volume = 0;
      bool cancel_order = false;
      ;
      outstanding_close_volume =
          follow.ProcessCloseOrder(order.order_no, outstanding_close_volume,
                                   &follow_close_volume, &cancel_order);
      if (cancel_order) {
        cancel_order_no_list->push_back(follow.follow_order_no());
      }
      close_volume += follow_close_volume;
    }
    if (close_volume > 0) {
      enter_order->order_no = order.order_no;
      enter_order->instrument = order.instrument;
      enter_order->order_direction = order.order_direction;
      enter_order->order_price = order.order_price;
      enter_order->volume = close_volume;
      enter_order->action = EnterOrderAction::kEOAClose;
    }
  } else if (order.order_status == OrderStatus::kOSCanceling) {
    cancel_order_no_list->push_back(order.order_no);
  } else if (order.order_status == OrderStatus::kOSClosed) {
  } else {
  }
}

void InstrumentFollow::HandleOrderRtnForFollow(
    const OrderRtnData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  auto it = std::find_if(
      order_follows_.begin(), order_follows_.end(),
      [&](auto forder) { return forder.follow_order_no() == order.order_no; });

  // ASSERT(it != order_follows_.end());
  if (it == order_follows_.end()) {
    return;
  }
  if (order.order_status == OrderStatus::kOSOpened) {
    it->FillOpenOrderForFollow(order.volume);
  }
}

int InstrumentFollow::CalcOrderReverseVolume(int order_volume) const {
  int trade_position_volume =
      std::accumulate(order_follows_.begin(), order_follows_.end(), 0,
                      [=](int value, auto follow) {
                        if (follow.order_direction() == order_direction_) {
                          return value + follow.position_volume_for_trade();
                        }
                        return 0;
                      });

  int follow_position_volume =
      std::accumulate(order_follows_.begin(), order_follows_.end(), 0,
                      [=](int value, auto follow) {
                        if (follow.order_direction() == order_direction_) {
                          return value + follow.position_volume_for_follow();
                        }
                        return 0;
                      });

  // include position and unfill
  int trade_reverse_volume =
      std::accumulate(order_follows_.begin(), order_follows_.end(), 0,
                      [=](int value, auto follow) {
                        if (follow.order_direction() != order_direction_) {
                          return value + follow.total_volume_for_trade();
                        }
                        return 0;
                      });

  int follow_reverse_volume =
      std::accumulate(order_follows_.begin(), order_follows_.end(), 0,
                      [=](int value, auto follow) {
                        if (follow.order_direction() != order_direction_) {
                          return value + follow.total_volume_for_follow();
                        }
                        return 0;
                      });

  int trade_left_unlock_volume =
      (trade_position_volume - trade_reverse_volume) - order_volume;
  return follow_position_volume - follow_reverse_volume -
         trade_left_unlock_volume;
}

void InstrumentFollow::ResetOrderDirectionIfNeed(const OrderRtnData& order) {
  if (order_direction_ == kODUnkown) {
    order_direction_ = order.order_direction;
    return;
  }

  if (order_direction_ == order.order_direction) {
    return;
  }

  int positive_pos =
      std::accumulate(order_follows_.begin(), order_follows_.end(), 0,
                      [=](int value, auto follow) {
                        if (follow.order_direction() == order_direction_) {
                          return value + follow.position_volume_for_trade();
                        }
                        return 0;
                      });
  int negative_pos =
      std::accumulate(order_follows_.begin(), order_follows_.end(), 0,
                      [=](int value, auto follow) {
                        if (follow.order_direction() != order_direction_) {
                          return value + follow.position_volume_for_trade();
                        }
                        return 0;
                      });

  order_direction_ = negative_pos > positive_pos
                         ? ReverseOrderDirection(order_direction_)
                         : order_direction_;
}

OrderDirection InstrumentFollow::ReverseOrderDirection(
    OrderDirection order_direction) const {
  return order_direction == kODBuy ? kODSell : kODBuy;
}
