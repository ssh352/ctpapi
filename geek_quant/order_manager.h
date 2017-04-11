#ifndef FOLLOW_TRADE_ORDER_MANAGER_H
#define FOLLOW_TRADE_ORDER_MANAGER_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/order.h"

class OrderManager {
 public:
  OrderEventType HandleRtnOrder(OrderData order);
  std::vector<std::string> GetCorrOrderNoWithOrderId(
      const std::string& order_id) const;

 private:
  bool IsCloseOrder(PositionEffect effect);
  std::map<std::string, Order> orders_;
};

#endif  // FOLLOW_TRADE_ORDER_MANAGER_H
