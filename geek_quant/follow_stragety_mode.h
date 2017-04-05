#ifndef FOLLOW_TRADE_FOLLOW_STRAGETY_MODE_H
#define FOLLOW_TRADE_FOLLOW_STRAGETY_MODE_H
#include "geek_quant/caf_defines.h"

class FollowTradeFollowStragetyMode {
 public:
  void Init(std::vector<OrderPosition> trader_position,
            std::vector<OrderPosition> follower_position,
            std::vector<OrderRtnData> trader_orders,
            std::vector<OrderRtnData> follower_orders);

  void HandleRtnOrderForTrader(OrderRtnData rtn_order);

  void HandleRtnOrderForFollower(OrderRtnData rtn_order);
 private:
};

#endif  // FOLLOW_TRADE_FOLLOW_STRAGETY_MODE_H
