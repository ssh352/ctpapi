#include "order.h"

Order::Order(OrderData&& data) : data_(data) {

}

bool Order::IsQuantityChange(int filled_quantity) const {
  return data_.filled_quantity_ != filled_quantity;
}

bool Order::IsActiveOrder() const {
  return data_.status() == OrderStatus::kActive;
}
