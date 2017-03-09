#include "instrument_follow.h"

void InstrumentFollow::HandleOrderRtnForTrader(
    const OrderRtnData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  if (order.order_status == OrderStatus::kOSOpening) {
    order_follows_.push_back(OrderFollow{order.order_no, order.volume});
    enter_order->order_no = order.order_no;
    enter_order->instrument = order.instrument;
    enter_order->order_direction = order.order_direction;
    enter_order->order_price = order.order_price;
    enter_order->volume = order.volume;
    enter_order->action = EnterOrderAction::kEOAOpen;
  } else if (order.order_status == OrderStatus::kOSOpened) {
    auto it = std::find_if(
        order_follows_.begin(), order_follows_.end(),
        [&](auto follow) { return follow.trade_order_no() == order.order_no; });
    if (it != order_follows_.end()) {
      it->FillOpenOrderForTrade(order.volume);
    } else {
      // ASSERT(FALSE)
    }
  } else if (order.order_status == OrderStatus::kOSCloseing) {
      int outstanding_close_volume = order.volume;
      int close_volume = 0;
      for (auto& follow : order_follows_) {
        int follow_close_volume = 0;
        bool cancel_order = false;;
        outstanding_close_volume = follow.ProcessCloseOrder(
            order.order_no, outstanding_close_volume, &follow_close_volume, &cancel_order);
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
    /*
    int pending_cancel_volume = order.volume - PositionVolume();
    for (auto& forder : order_follows_) {
      int canceling_volume = forder.CancelableVolume();
      if (canceling_volume > 0) {
        pending_cancel_volume -= canceling_volume;
        forder.CancelOrder();
      }
      if (pending_cancel_volume <= 0) {
        break;
      }
    }
    */
  } else if (order.order_status == OrderStatus::kOSCanceling) {
    cancel_order_no_list->push_back(order.order_no);
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

int InstrumentFollow::UnfillVolume() const {
  return std::accumulate(
      order_follows_.begin(), order_follows_.end(), 0,
      [&](int pre_sum, auto order) { return pre_sum + order.UnfillVolume(); });
}

int InstrumentFollow::PositionVolume() const {
  return std::accumulate(order_follows_.begin(), order_follows_.end(), 0,
                         [&](int pre_sum, auto order) {
                           return pre_sum + order.position_volume();
                         });
}
