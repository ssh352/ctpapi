#include "follow_strategy_mode/src/position_manager.h"

void PositionManager::AddQuantity(const std::string& instrument,
                                  OrderQuantity quantitys) {
  instrument_positions_[instrument].AddQuantity(std::move(quantitys));
}

std::vector<OrderQuantity> PositionManager::GetQuantitys(
    const std::string& instrument,
    std::vector<std::string> order_ids) const {
  if (instrument_positions_.find(instrument) == instrument_positions_.end()) {
    return {};
  }
  return instrument_positions_.at(instrument).GetQuantitys(order_ids);
}

std::vector<OrderQuantity> PositionManager::GetQuantitysIf(
    const std::string& instrument,
    std::function<bool(const OrderQuantity&)> cond) const {
  if (instrument_positions_.find(instrument) == instrument_positions_.end()) {
    return {};
  }
  return instrument_positions_.at(instrument).GetQuantitysIf(cond);
}

int PositionManager::GetCloseableQuantityWithInstrument(
    const std::string& order_id) const {
  int ret = 0;

  for (auto pos : instrument_positions_) {
    if (auto quantity =
            pos.second.GetCloseableQuantityWithInstrument(order_id)) {
      ret = *quantity;
      break;
    }
  }

  return ret;
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
