#ifndef FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
#define FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/order_follow.h"

class InstrumentFollow {
 public:
  InstrumentFollow();

  bool HasSyncOrders();

  bool TryCompleteSyncOrders();

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
  void ProcessHistoryOrderRtn(const OrderRtnData& order, bool for_trader);
  void DoProcessHistoryOrderRtn(const OrderRtnData& order,
                                std::vector<OrderFollow>* history_order);

  bool has_sync_;

  int trader_order_rtn_seq_;
  int follower_order_rtn_seq_;

  int last_check_trader_order_rtn_seq_;
  int last_check_follower_order_rtn_seq_;

  std::vector<OrderFollow> order_follows_;
  std::vector<OrderFollow> history_order_for_trader_;
  std::vector<OrderFollow> history_order_for_follower_;
  OrderDirection order_direction_;
};

#endif  // FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
