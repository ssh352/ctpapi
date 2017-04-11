#include "geek_quant/order_manager.h"
#include "geek_quant/order_util.h"

OrderEventType OrderManager::HandleRtnOrder(OrderData order) {
  OrderEventType ret_type = OrderEventType::kIgnore;
  if (orders_.find(order.order_id()) == orders_.end()) {
    ret_type = IsCloseOrder(order.position_effect()) ? OrderEventType::kNewClose
                                                     : OrderEventType::kNewOpen;
  } else if (order.status() == OrderStatus::kCancel) {
    ret_type = OrderEventType::kCanceled;
  } else {
    ret_type = IsCloseOrder(order.position_effect())
                   ? OrderEventType::kCloseTraded
                   : OrderEventType::kOpenTraded;
  }
  std::swap(orders_[order.order_id()], Order(std::move(order)));
  return ret_type;
}

std::vector<std::string> OrderManager::GetCorrOrderNoWithOrderId(
    const std::string& order_id) const {
  return {};
}

const std::string& OrderManager::GetOrderInstrument(
    const std::string& order_id) const {
  auto it = orders_.find(order_id);
  if (it == orders_.end()) {
    return dummpy_empty_;
  }
  return it->second.instrument();
}

bool OrderManager::IsUnfillOrder(const std::string& order_id) const {
  if (orders_.find(order_id) == orders_.end()) {
    return false;
  }

  return orders_.at(order_id).IsUnfillOrder();
}
