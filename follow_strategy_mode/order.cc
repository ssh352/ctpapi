#include "order.h"
#include "follow_strategy_mode/order_util.h"

Order::Order(OrderData&& data) : data_(data) {

}

bool Order::IsQuantityChange(int filled_quantity) const {
  return data_.filled_quantity_ != filled_quantity;
}

bool Order::IsActiveOrder() const {
  return data_.status() == OrderStatus::kActive;
}

bool Order::IsOpen() const {
  return IsOpenOrder(data_.position_effect());
}

int Order::unfill_quantity() const {
  return data_.quanitty() - data_.filled_quantity();
}

OrderData Order::order_data() const {
  return data_;
}
