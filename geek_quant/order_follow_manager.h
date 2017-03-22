#ifndef FOLLOW_TRADE_ORDER_FOLLOW_MANAGER_H
#define FOLLOW_TRADE_ORDER_FOLLOW_MANAGER_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/order_follow.h"

class OrderFollowMananger {
 public:
  void HandleOrderRtn(const OrderRtnData& order);

  bool IsOpenReverseOrder(const OrderRtnData& order) const;

  std::vector<std::pair<std::string, int> > GetPositionVolumeWithOrderNoList(
      const std::vector<std::string>& order_list) const;

  OpenReverseOrderActionInfo ParseOpenReverseOrderRtn(
      const OrderRtnData& order) const;

  CloseingActionInfo ParseCloseingOrderRtn(const OrderRtnData& order) const;

 private:
  void HandleCloseing(const OrderRtnData& order);
  mutable std::map<std::string, OrderFollow> orders_;
  std::map<std::string, std::vector<std::pair<std::string, int> > >
      closeing_orders_;
};

#endif  // FOLLOW_TRADE_ORDER_FOLLOW_MANAGER_H
