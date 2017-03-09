#include "instrument_follow.h"

void InstrumentFollow::HandleOrderRtnForTrader(
    const OrderRtnData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  if (order.order_status == OrderStatus::kOSCloseing) {
    enter_order->order_no = order.order_no;
    enter_order->instrument = order.instrument;
    enter_order->order_direction = order.order_direction;
    enter_order->order_price = order.order_price;
    enter_order->volume = order.volume;
    enter_order->action = EnterOrderAction::kEOAClose;

  } else {
    enter_order->order_no = order.order_no;
    enter_order->instrument = order.instrument;
    enter_order->order_direction = order.order_direction;
    enter_order->order_price = order.order_price;
    enter_order->volume = order.volume;
    enter_order->action = EnterOrderAction::kEOAOpen;
  }
}

void InstrumentFollow::HandleOrderRtnForFollow(
    const OrderRtnData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {}
