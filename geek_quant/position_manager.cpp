#include "position_manager.h"

std::vector<std::string> PositionManager::GetCorrOrderNoWithOrderId(
    const std::string& order_id) const {
  return {};
}

std::vector<OrderQuantity> PositionManager::GetQuantitysWithOrderIds(
    const std::string& instrument, 
  std::vector<std::string> order_ids) {
  return instrument_positions_[instrument].GetQuantitysWithOrderIds(order_ids);
}

std::vector<OrderQuantity> PositionManager::GetCorrOrderQuantiysWithOrderNo(
    const std::string& instrument, 
  const std::string& order_id) const {
  return {OrderQuantity{"", 0, 0}};
}

int PositionManager::GetPositionCloseableQuantity(const std::string& instrument,
                                                  OrderDirection direction) {
  return instrument_positions_[instrument].GetPositionCloseableQuantity(
      direction);
}
