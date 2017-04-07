#ifndef FOLLOW_TRADE_ORDER_FOLLOW_MANAGER_H
#define FOLLOW_TRADE_ORDER_FOLLOW_MANAGER_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/order_follow.h"

class OrderFollowMananger {
 public:
  void AddPosition(const std::string& order_no, OrderDirection order_direction, int volume);

  void HandleOrderRtn(const RtnOrderData& order);

  bool IsOpenReverseOrder(const RtnOrderData& order) const;

  std::vector<std::pair<std::string, int> > GetPositionVolumeWithOrderNoList(
      const std::vector<std::string>& order_list) const;

  OpenReverseOrderActionInfo ParseOpenReverseOrderRtn(
      const RtnOrderData& order) const;

  CloseingActionInfo ParseCloseingOrderRtn(const RtnOrderData& order) const;

 private:
  void HandleCloseing(const RtnOrderData& order);
  mutable std::map<std::string, OrderFollow> orders_;
  std::map<std::string, std::vector<std::pair<std::string, int> > >
      closeing_orders_;
};

#endif  // FOLLOW_TRADE_ORDER_FOLLOW_MANAGER_H
