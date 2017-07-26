#include "follow_strategy_mode/order_manager.h"
#include "follow_strategy_mode/order_util.h"

OrderEventType OrderManager::HandleRtnOrder(OrderData order) {
  OrderEventType ret_type = OrderEventType::kIgnore;
  if (orders_.find(order.order_id()) == orders_.end()) {
    ret_type = IsCloseOrder(order.position_effect()) ? OrderEventType::kNewClose
                                                     : OrderEventType::kNewOpen;
  } else if (order.status() == OrderStatus::kCanceled) {
    ret_type = OrderEventType::kCanceled;
  } else if (orders_[order.order_id()].IsQuantityChange(
                 order.filled_quantity())) {
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

std::vector<std::tuple<std::string, OrderDirection, bool, int> >
OrderManager::GetUnfillOrders() const {
  std::set<std::tuple<std::string, OrderDirection, bool> > unique_orders;
  for (auto item : orders_) {
    if (item.second.IsActiveOrder()) {
      unique_orders.emplace(item.second.instrument(), item.second.direction(),
                            item.second.IsOpen());
    }
  }

  std::vector<std::tuple<std::string, OrderDirection, bool, int> >
      unfill_orders;
  for (auto item : unique_orders) {
    unfill_orders.push_back(
        {std::get<0>(item), std::get<1>(item), std::get<2>(item),
         GetUnfillQuantity(std::get<0>(item), std::get<1>(item),
                           std::get<2>(item))});
  }

  return unfill_orders;
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
        item.second.direction() == direction && item.second.IsActiveOrder()) {
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

boost::optional<OrderData> OrderManager::order_data(
    const std::string& order_id) const {
  if (orders_.find(order_id) == orders_.end()) {
    return {};
  }
  return orders_.at(order_id).order_data();
}

int OrderManager::GetUnfillQuantity(const std::string& instrument,
                                    OrderDirection direction,
                                    bool is_open) const {
  return std::accumulate(orders_.begin(), orders_.end(), 0,
                         [=](int val, auto item) {
                           if (item.second.IsActiveOrder() &&
                               instrument == item.second.instrument() &&
                               direction == item.second.direction() &&
                               is_open == item.second.IsOpen()) {
                             return val + item.second.unfill_quantity();
                           }
                           return val;
                         });
}
