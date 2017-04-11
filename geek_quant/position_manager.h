#ifndef FOLLOW_TRADE_POSITION_MANAGER_H
#define FOLLOW_TRADE_POSITION_MANAGER_H

#include "geek_quant/caf_defines.h"
#include "geek_quant/instrument_position.h"

class PositionManager {
 public:
  std::vector<std::string> GetCorrOrderNoWithOrderId(
      const std::string& order_id) const;

  std::vector<OrderQuantity> GetQuantitysWithOrderIds(
      const std::string& instrument,
      std::vector<std::string> orders);

  std::vector<OrderQuantity> GetCorrOrderQuantiysWithOrderNo(
      const std::string& instrument,
      const std::string& order_id) const;

  int GetPositionCloseableQuantity(const std::string& instrument,
                                   OrderDirection direction);

 private:
  std::map<std::string, InstrumentPosition> instrument_positions_;
};

#endif  // FOLLOW_TRADE_POSITION_MANAGER_H
