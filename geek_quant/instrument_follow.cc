#include "instrument_follow.h"

void InstrumentFollow::HandleOrderRtnForTrader(
    const OrderRtnData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  if (order.order_status == OrderStatus::kOSCloseing) {
    if (UnfillVolume() < order.volume) {
      enter_order->order_no = order.order_no;
      enter_order->instrument = order.instrument;
      enter_order->order_direction = order.order_direction;
      enter_order->order_price = order.order_price;
      enter_order->volume = order.volume - UnfillVolume();
      enter_order->action = EnterOrderAction::kEOAClose;
    }
    if (order.volume != PositionVolume()) {
      int pending_cancel_volume = order.volume - PositionVolume();
      for (auto forder : follow_orders_) {
        if ((forder.total_volume - forder.position_volume) > 0) {
          pending_cancel_volume -= forder.total_volume - forder.position_volume;
          cancel_order_no_list->push_back(forder.order_no);
        }
        if (pending_cancel_volume <= 0) {
          break;
        }
      }
    }
  } else if (order.order_status == OrderStatus::kOSCanceling) {
    cancel_order_no_list->push_back(order.order_no);
  } else if (order.order_status == OrderStatus::kOSOpening) {
    follow_orders_.push_back(FollowOrder{order.order_no, order.volume, 0});
    enter_order->order_no = order.order_no;
    enter_order->instrument = order.instrument;
    enter_order->order_direction = order.order_direction;
    enter_order->order_price = order.order_price;
    enter_order->volume = order.volume;
    enter_order->action = EnterOrderAction::kEOAOpen;
  } else {
  }
}

void InstrumentFollow::HandleOrderRtnForFollow(
    const OrderRtnData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  auto it = std::find_if(
      follow_orders_.begin(), follow_orders_.end(),
      [&](auto forder) { return forder.order_no == order.order_no; });

  // ASSERT(it != follow_orders_.end());
  if (it == follow_orders_.end()) {
    return;
  }
  if (order.order_status == OrderStatus::kOSOpened) {
    it->position_volume += order.volume;
  }
}

int InstrumentFollow::UnfillVolume() const {
  return std::accumulate(follow_orders_.begin(), follow_orders_.end(), 0,
                         [&](int pre_sum, auto order) {
                           return pre_sum +
                                  (order.total_volume - order.position_volume);
                         });
}

int InstrumentFollow::PositionVolume() const {
  return std::accumulate(
      follow_orders_.begin(), follow_orders_.end(), 0,
      [&](int pre_sum, auto order) { return pre_sum + order.position_volume; });
}
