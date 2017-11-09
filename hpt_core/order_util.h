#ifndef FOLLOW_TRADE_ORDER_UTIL_H
#define FOLLOW_TRADE_ORDER_UTIL_H
#include "common/api_struct.h"

bool IsCloseOrder(PositionEffect position_effect);

bool IsOpenOrder(PositionEffect position_effect);

OrderDirection OppositeOrderDirection(OrderDirection direction);

bool IsCancelableOrderStatus(OrderStatus status);

#endif  // FOLLOW_TRADE_ORDER_UTIL_H
