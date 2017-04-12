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

const std::string& OrderManager::GetOrderInstrument(
    const std::string& order_id) const {
  auto it = orders_.find(order_id);
  if (it == orders_.end()) {
    return dummpy_empty_;
  }
  return it->second.instrument();
}

int OrderManager::ActiveOrderCount(const std::string& instrument,
                                   OrderDirection direction) const {
  return std::count_if(orders_.begin(), orders_.end(), [&](auto item) {
    if (item.second.instrument() != instrument ||
        item.second.direction() != direction) {
      return false;
    }
    return item.second.IsActiveOrder();
  });
}

std::vector<std::string> OrderManager::ActiveOrderIds(
    const std::string& instrument,
    OrderDirection direction) const {
  std::vector<std::string> order_ids;
  for (auto item : orders_) {
    if (item.second.instrument() == instrument &&
        item.second.direction() == direction &&
        item.second.IsActiveOrder()) {
      order_ids.push_back(item.first);
    }
  }
  return order_ids;
}

bool OrderManager::IsActiveOrder(const std::string& order_id) const {
  if (orders_.find(order_id) == orders_.end()) {
    return false;
  }

  return orders_.at(order_id).IsActiveOrder();
}
