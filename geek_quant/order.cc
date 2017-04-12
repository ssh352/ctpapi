#include "order.h"

Order::Order(OrderData&& data) : data_(data) {

}

bool Order::IsActiveOrder() const {
  return data_.status() == OrderStatus::kActive;
}
