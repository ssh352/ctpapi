#include "order.h"

Order::Order(OrderData&& data) : data_(data) {

}

bool Order::IsUnfillOrder() const {
  return data_.status() == OrderStatus::kActive;
}
