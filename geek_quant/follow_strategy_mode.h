#ifndef FOLLOW_TRADE_FOLLOW_strategy_MODE_H
#define FOLLOW_TRADE_FOLLOW_strategy_MODE_H
#include "geek_quant/caf_defines.h"

class FollowTradeFollowStragetyMode {
 public:
  void Init(std::vector<OrderPosition> trader_position,
            std::vector<OrderPosition> follower_position,
            std::vector<RtnOrderData> trader_orders,
            std::vector<RtnOrderData> follower_orders);

  void HandleRtnOrderForTrader(RtnOrderData rtn_order);

  void HandleRtnOrderForFollower(RtnOrderData rtn_order);
 private:
};

#endif  // FOLLOW_TRADE_FOLLOW_strategy_MODE_H
