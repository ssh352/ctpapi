#include "instrument_follow.h"

InstrumentFollow::InstrumentFollow() {
  order_direction_ = kODUnkown;
  has_sync_ = false;
  trader_order_rtn_seq_ = 0;
  follower_order_rtn_seq_ = 0;
  last_check_trader_order_rtn_seq_ = 0;
  last_check_follower_order_rtn_seq_ = 0;
}

bool InstrumentFollow::HasSyncOrders() {
  return has_sync_;
}

bool InstrumentFollow::TryCompleteSyncOrders() {
  if (trader_order_rtn_seq_ != last_check_trader_order_rtn_seq_ ||
      follower_order_rtn_seq_ != last_check_follower_order_rtn_seq_) {
    last_check_trader_order_rtn_seq_ = trader_order_rtn_seq_;
    last_check_follower_order_rtn_seq_ = follower_order_rtn_seq_;
    return false;
  }

  for (auto order : history_order_for_trader_) {
    auto it = std::find_if(
        history_order_for_follower_.begin(), history_order_for_follower_.end(),
        [=](auto f_order) {
          return order.trade_order_no() == f_order.trade_order_no();
        });
    if (it != history_order_for_follower_.end()) {
      order_follows_.push_back(
          OrderFollow{order.trader_volume_data(), it->trader_volume_data()});
      history_order_for_follower_.erase(it);
    }
  }
  has_sync_ = true;
  return true;
}

void InstrumentFollow::HandleOrderRtnForTrader(
    const OrderRtnData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  ++trader_order_rtn_seq_;
  if (!HasSyncOrders()) {
    ProcessHistoryOrderRtn(order, true);
    return;
  }
  switch (order.order_status) {
    case kOSOpening: {
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
        for (auto follow : order_follows_) {
          if (follow.CancelableVolume()) {
            cancel_order_no_list->push_back(follow.follow_order_no());
          }
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
    } break;
    case kOSCloseing: {
      int outstanding_close_volume = order.volume;
      int close_volume = 0;
      for (auto& follow : order_follows_) {
        int follow_close_volume = 0;
        bool cancel_order = false;
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
    } break;
    case kOSOpened: {
      auto it = std::find_if(order_follows_.begin(), order_follows_.end(),
                             [&](auto follow) {
                               return follow.trade_order_no() == order.order_no;
                             });
      if (it != order_follows_.end()) {
        it->FillOpenOrderForTrade(order.volume);
      } else {
        // ASSERT(FALSE)
      }
      ResetOrderDirectionIfNeed(order);
    } break;
    case kOSClosed: {
    } break;
    case kOSCanceled: {
      cancel_order_no_list->push_back(order.order_no);
    } break;
    default:
      break;
  }
}

void InstrumentFollow::HandleOrderRtnForFollow(
    const OrderRtnData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  ++follower_order_rtn_seq_;
  if (!HasSyncOrders()) {
    ProcessHistoryOrderRtn(order, false);
    return;
  }
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

void InstrumentFollow::ProcessHistoryOrderRtn(const OrderRtnData& order,
                                              bool for_trader) {
  DoProcessHistoryOrderRtn(order, for_trader ? &history_order_for_trader_
                                             : &history_order_for_follower_);
}

void InstrumentFollow::DoProcessHistoryOrderRtn(
    const OrderRtnData& order,
    std::vector<OrderFollow>* history_order) {
  switch (order.order_status) {
    case kOSOpening: {
      history_order->push_back(
          OrderFollow{order.order_no, order.volume, order.order_direction});
    } break;
    case kOSCloseing: {
      int outstanding_close_volume = order.volume;
      for (auto& follow : order_follows_) {
        int follow_close_volume = 0;
        bool cancel_order = false;
        outstanding_close_volume =
            follow.ProcessCloseOrder(order.order_no, outstanding_close_volume,
                                     &follow_close_volume, &cancel_order);
      }
    } break;
    case kOSOpened: {
      auto it = std::find_if(history_order->begin(), history_order->end(),
                             [&](auto follow) {
                               return follow.trade_order_no() == order.order_no;
                             });
      if (it != history_order->end()) {
        it->FillOpenOrderForTrade(order.volume);
      } else {
        // ASSERT(FALSE)
      }
    } break;
    case kOSClosed: {
    } break;
    case kOSCanceled: {
    } break;
    default:
      break;
  }
}
