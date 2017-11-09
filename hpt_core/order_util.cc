#include "hpt_core/order_util.h"

bool IsCloseOrder(PositionEffect position_effect) {
  return position_effect == PositionEffect::kClose;
}

bool IsOpenOrder(PositionEffect position_effect) {
  return !IsCloseOrder(position_effect);
}

OrderDirection OppositeOrderDirection(OrderDirection direction) {
  return direction == OrderDirection::kBuy ? OrderDirection::kSell
                                           : OrderDirection::kBuy;
}

bool IsCancelableOrderStatus(OrderStatus status) {
  return status == OrderStatus::kActive;
}

OrderDirection AdjustDirectionByPositionEffect(PositionEffect position_effect,
                                               OrderDirection direction) {
  return position_effect == PositionEffect::kOpen
             ? direction
             : OppositeOrderDirection(direction);
}
