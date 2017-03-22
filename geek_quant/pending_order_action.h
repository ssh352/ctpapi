#ifndef FOLLOW_TRADE_PENDING_ORDER_ACTION_H
#define FOLLOW_TRADE_PENDING_ORDER_ACTION_H

#include "geek_quant/caf_defines.h"

class PendingOrderAction {
 public:
  PendingOrderAction() = default;

  PendingOrderAction(const OrderRtnData& order, int follower_volum); 

  void HandleOrderRtnForTrader(const OrderRtnData& order);

  bool HandleOrderRtnForFollower(const OrderRtnData& order, std::vector<std::string>* cancel_order_no_list);

  void HandleCloseing(std::vector<std::string>* cancel_order_no_list);

  void HandleOpenReverse(std::vector<std::string>* cancel_order_no_list);

  OrderDirection order_direction() const {
    return order_direction_;
  }

  int pending_close_volume() const {
    return pending_close_volume_;
  }
private:
  struct InnerOrder {
    int traded_volume;
    int total_volume;
    bool cancel;
  };
  InnerOrder trader_;
  InnerOrder follower_;
  std::string order_no_;
  OrderDirection order_direction_;
  int pending_close_volume_;
};

#endif  // FOLLOW_TRADE_PENDING_ORDER_ACTION_H
