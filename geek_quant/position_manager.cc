#include "geek_quant/position_manager.h"

std::vector<OrderQuantity> PositionManager::GetQuantitys(
    const std::string& instrument,
    std::vector<std::string> order_ids) {
  return instrument_positions_[instrument].GetQuantitys(order_ids);
}

int PositionManager::GetCloseableQuantityWithInstrument(
    const std::string& instrument,
    const std::string& order_id) const {
  if (instrument_positions_.find(instrument) == instrument_positions_.end()) {
    return 0;
  }

  return instrument_positions_.at(instrument)
      .GetCloseableQuantityWithInstrument(order_id);
}

int PositionManager::GetCloseableQuantityWithOrderDirection(
    const std::string& instrument,
    OrderDirection direction) const {
  if (instrument_positions_.find(instrument) == instrument_positions_.end()) {
    return 0;
  }
  return instrument_positions_.at(instrument)
      .GetCloseableQuantityWithOrderDirection(direction);
}

void PositionManager::HandleRtnOrder(
    const OrderData& rtn_order,
    CloseCorrOrdersManager* close_corr_orders_mgr) {
  instrument_positions_[rtn_order.instrument()].HandleRtnOrder(
      rtn_order, close_corr_orders_mgr);
}
