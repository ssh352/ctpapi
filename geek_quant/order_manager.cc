#include "order_manager.h"

OrderEventType OrderManager::HandleRtnOrder(OrderData order) {
  OrderEventType ret_type = OrderEventType::kIgnore;
  if (orders_.find(order.order_id()) == orders_.end()) {
    orders_[order.order_id()] = Order(std::move(order));
    ret_type = IsCloseOrder(order.position_effect()) ? OrderEventType::kNewClose
                                                     : OrderEventType::kNewOpen;
  } else if (order.status() == OrderStatus::kCancel) {
    ret_type = OrderEventType::kCanceled;
  } else {
    ret_type = IsCloseOrder(order.position_effect())
                   ? OrderEventType::kCloseTraded
                   : OrderEventType::kOpenTraded;
  }
  return ret_type;
}

std::vector<std::string> OrderManager::GetCorrOrderNoWithOrderId(
    const std::string& order_id) const {

}

bool OrderManager::IsCloseOrder(PositionEffect effect) {
  return effect == PositionEffect::kClose ||
         effect == PositionEffect::kCloseToday;
}
