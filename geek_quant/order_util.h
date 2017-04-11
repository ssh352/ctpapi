#ifndef FOLLOW_TRADE_ORDER_UTIL_H
#define FOLLOW_TRADE_ORDER_UTIL_H
#include "geek_quant/caf_defines.h"

bool IsCloseOrder(PositionEffect position_effect);

bool IsOpenOrder(PositionEffect position_effect);

OrderDirection OppositeOrderDirection(OrderDirection direction);


#endif  // FOLLOW_TRADE_ORDER_UTIL_H
