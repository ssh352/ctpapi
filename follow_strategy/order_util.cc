#include "order_util.h"

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
