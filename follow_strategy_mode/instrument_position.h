#ifndef FOLLOW_TRADE_INSTRUMENT_POSITION_H
#define FOLLOW_TRADE_INSTRUMENT_POSITION_H
#include "common/api_struct.h"
#include <boost/optional.hpp>
class CloseCorrOrdersManager;

class InstrumentPosition {
 public:
  std::vector<OrderQuantity> GetQuantitys(
      std::vector<std::string> orders) const;

  std::vector<OrderQuantity> GetQuantitysIf(
      std::function<bool(const OrderQuantity&)> cond) const;

  int GetCloseableQuantityWithOrderDirection(OrderDirection direction) const;

  boost::optional<int> GetCloseableQuantityWithOrderId(const std::string& order_id) const;

  void HandleRtnOrder(const OrderField& rtn_order,
                      CloseCorrOrdersManager* close_corr_orders_mgr);

  void AddQuantity(OrderQuantity quantity);
private:
  bool TestPositionEffect(const std::string& exchange_id,
                          PositionEffect position_effect,
                          bool is_today_quantity);
  std::map<std::string, OrderQuantity> positions_;
};

#endif  // FOLLOW_TRADE_INSTRUMENT_POSITION_H
