#ifndef FOLLOW_TRADE_ORDER_MANAGER_H
#define FOLLOW_TRADE_ORDER_MANAGER_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/order.h"

class OrderManager {
 public:
  OrderEventType HandleRtnOrder(OrderData order);

  const std::string& GetOrderInstrument(const std::string& order_id) const;

  int ActiveOrderCount(const std::string& instrument,
                       OrderDirection direction) const;

  std::vector<std::string> ActiveOrderIds(const std::string& instrument,
                                          OrderDirection direction) const;

  bool IsActiveOrder(const std::string& order_id) const;

 private:
  std::map<std::string, Order> orders_;
  std::string dummpy_empty_;
};

#endif  // FOLLOW_TRADE_ORDER_MANAGER_H
