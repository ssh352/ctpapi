#ifndef FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
#define FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/order_follow.h"

class InstrumentFollow {
 public:
  InstrumentFollow();

  void InitOrderVolumeOrderForTrader(std::vector<OrderVolume> orders);

  void InitOrderVolumeOrderForFollower(std::vector<OrderVolume> orders);

  void HandleOrderRtnForTrader(const OrderRtnData& order,
                               EnterOrderData* enter_order,
                               std::vector<std::string>* cancel_order_no_list);

  void HandleOrderRtnForFollow(const OrderRtnData& order,
                               EnterOrderData* enter_order,
                               std::vector<std::string>* cancel_order_no_list);

 private:
  int CalcOrderReverseVolume(int order_volume) const;
  void ResetOrderDirectionIfNeed(const OrderRtnData& order);
  OrderDirection ReverseOrderDirection(OrderDirection order_direction) const;
  void ProcessPendingOrder();

  bool pending_trader_init_;
  bool pending_follower_init_;

  std::vector<OrderFollow> order_follows_;
  std::vector<OrderVolume> pending_trader_orders_;
  std::vector<OrderVolume> pending_follower_orders_;
  OrderDirection order_direction_;
};

#endif  // FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
