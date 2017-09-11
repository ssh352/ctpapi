#ifndef FOLLOW_TRADE_ORDER_MANAGER_H
#define FOLLOW_TRADE_ORDER_MANAGER_H
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include "common/api_struct.h"
#include "follow_strategy/order.h"

class OrderManager {
 public:
  OrderEventType HandleRtnOrder(const std::shared_ptr<const OrderField>& order);

  const std::string& GetOrderInstrument(const std::string& order_id) const;

  std::vector<std::tuple<std::string, OrderDirection, bool, int> >
  GetUnfillOrders() const;

  int ActiveOrderCount(const std::string& instrument,
                       OrderDirection direction) const;

  std::vector<std::string> ActiveOrderIds(const std::string& instrument,
                                          OrderDirection direction) const;

  bool IsActiveOrder(const std::string& order_id) const;

  boost::optional<OrderField> order_data(const std::string& order_id) const;

 private:
  int GetUnfillQuantity(const std::string& instrument,
                        OrderDirection direction,
                        bool is_open) const;
  std::map<std::string, Order> orders_;
  std::string dummpy_empty_;
};

#endif  // FOLLOW_TRADE_ORDER_MANAGER_H
