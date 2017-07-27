#ifndef FOLLOW_TRADE_POSITION_MANAGER_H
#define FOLLOW_TRADE_POSITION_MANAGER_H

#include <boost/optional.hpp>
#include "common/api_struct.h"
#include "follow_strategy_mode/instrument_position.h"

class CloseCorrOrdersManager;

class PositionManager {
 public:
  void AddQuantitys(const std::string& instrument,
                    std::vector<OrderQuantity> quantitys);

  void AddQuantity(const std::string& instrument, OrderQuantity quantitys);

  std::vector<OrderQuantity> GetQuantitys(
      const std::string& instrument,
      std::vector<std::string> orders) const;

  std::vector<OrderQuantity> GetQuantitysIf(
      const std::string& instrument,
      std::function<bool(const OrderQuantity&)> cond) const;

  std::vector<AccountPosition> GetAccountPositions() const;

  boost::optional<AccountPosition> GetAccountPosition(
      const std::string& instrument,
      const InstrumentPosition& instrument_position,
      OrderDirection direction) const;

  int GetCloseableQuantityWithInstrument(const std::string& order_id) const;

  int GetCloseableQuantityWithOrderDirection(const std::string& instrument,
                                             OrderDirection direction) const;

  void HandleRtnOrder(const OrderField& rtn_order,
                      CloseCorrOrdersManager* close_corr_orders_mgr);

 private:
  std::map<std::string, InstrumentPosition> instrument_positions_;
};

#endif  // FOLLOW_TRADE_POSITION_MANAGER_H
