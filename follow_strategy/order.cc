#include "order.h"
#include "follow_strategy/order_util.h"

Order::Order(const std::shared_ptr<const OrderField>& data) : data_(data) {}

bool Order::IsQuantityChange(int leaves_qty) const {
  return data_->leaves_qty != leaves_qty;
}

bool Order::IsActiveOrder() const {
  return data_->status == OrderStatus::kActive;
}

bool Order::IsOpen() const {
  return IsOpenOrder(data_->position_effect);
}

int Order::unfill_quantity() const {
  return data_->leaves_qty;
}

OrderField Order::order_data() const {
  return *data_;
}
