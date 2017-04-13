#ifndef FOLLOW_TRADE_ORDER_MANAGER_H
#define FOLLOW_TRADE_ORDER_MANAGER_H
#include <boost/optional.hpp>
#include "follow_strategy_mode/defines.h"
#include "follow_strategy_mode/order.h"

class OrderManager {
 public:
  OrderEventType HandleRtnOrder(OrderData order);

  const std::string& GetOrderInstrument(const std::string& order_id) const;

  int ActiveOrderCount(const std::string& instrument,
                       OrderDirection direction) const;

  std::vector<std::string> ActiveOrderIds(const std::string& instrument,
                                          OrderDirection direction) const;

  bool IsActiveOrder(const std::string& order_id) const;

  boost::optional<OrderData> order_data(const std::string& order_id) const;

 private:
  std::map<std::string, Order> orders_;
  std::string dummpy_empty_;
};

#endif  // FOLLOW_TRADE_ORDER_MANAGER_H
