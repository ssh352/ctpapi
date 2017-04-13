#ifndef FOLLOW_TRADE_SERVER_CAF_CTP_UTIL_H
#define FOLLOW_TRADE_SERVER_CAF_CTP_UTIL_H
#include "caf/all.hpp"
#include "follow_strategy_mode/defines.h"

bool Logon(caf::actor actor);

std::vector<OrderPosition> BlockRequestInitPositions(caf::actor actor);

std::vector<OrderData> BlockRequestHistoryOrder(caf::actor actor);

void SettlementInfoConfirm(caf::actor actor);

#endif  // FOLLOW_TRADE_SERVER_CAF_CTP_UTIL_H
