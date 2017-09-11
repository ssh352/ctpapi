#ifndef FOLLOW_TRADE_ORDERS_CONTEXT_H
#define FOLLOW_TRADE_ORDERS_CONTEXT_H
#include <map>
#include <vector>
#include "common/api_struct.h"
#include "follow_strategy/close_corr_orders_manager.h"

#include "follow_strategy/order_manager.h"
#include "follow_strategy/position_manager.h"

class OrdersContext {
 public:
  OrdersContext(std::string account_id);

  OrderEventType HandleRtnOrder(
      const std::shared_ptr<const OrderField>& rtn_order);

  std::vector<OrderQuantity> GetQuantitys(
      std::vector<std::string> order_ids) const;

  std::vector<OrderQuantity> GetQuantitysIf(
      const std::string& instrument,
      std::function<bool(const OrderQuantity&)> cond) const;

  int GetCloseableQuantityWithOrderDirection(const std::string& instrument,
                                             OrderDirection direction) const;

  std::vector<std::pair<std::string, int> > GetCorrOrderQuantiys(
      const std::string& order_id);

  std::vector<std::string> GetCloseCorrOrderIds(const std::string& order_id);

  int ActiveOrderCount(const std::string& instrument,
                       OrderDirection direction) const;

  std::vector<std::string> ActiveOrderIds(const std::string& instrument,
                                          OrderDirection direction) const;

  bool IsActiveOrder(const std::string& order_id) const;

  int GetCloseableQuantity(const std::string& order_id) const;

  bool IsOppositeOpen(const std::string& instrument,
                      OrderDirection direction) const;

  void InitPositions(std::vector<OrderPosition> quantitys);

  boost::optional<OrderField> GetOrderData(const std::string& order_id) const;

  std::vector<AccountPortfolio> GetAccountPortfolios() const;

  const std::string& account_id() const;

 private:
  std::vector<AccountPosition> GetAccountPositions() const;

  std::vector<std::tuple<std::string, OrderDirection, bool, int> >
  GetUnfillOrders() const;

  OrderManager account_order_mgr_;
  PositionManager account_position_mgr_;
  CloseCorrOrdersManager account_close_corr_orders_mgr_;
  std::string account_id_;
};

#endif  // FOLLOW_TRADE_ORDERS_CONTEXT_H
