#include "follow_strategy_mode/position_manager.h"

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

std::vector<AccountPosition> PositionManager::GetAccountPositions() const {
  std::vector<AccountPosition> positions;
  for (auto instrument : instrument_positions_) {
    if (auto pos = GetAccountPosition(instrument.first, instrument.second,
                                      OrderDirection::kBuy)) {
      positions.push_back(*pos);
    }
    if (auto pos = GetAccountPosition(instrument.first, instrument.second,
                                      OrderDirection::kSell)) {
      positions.push_back(*pos);
    }
  }
  return positions;
}

boost::optional<AccountPosition> PositionManager::GetAccountPosition(
    const std::string& instrument,
    const InstrumentPosition& instrument_position,
    OrderDirection direction) const {
  int quantity = instrument_position.GetQuantityWithOrderDireciton(direction);
  int closeable_quantity =
      instrument_position.GetCloseableQuantityWithOrderDirection(direction);
  if (quantity != 0 || closeable_quantity != 0) {
    return AccountPosition{instrument, direction, quantity, closeable_quantity};
  };
  return {};
}

int PositionManager::GetCloseableQuantityWithInstrument(
    const std::string& order_id) const {
  int ret = 0;

  for (auto pos : instrument_positions_) {
    if (auto quantity = pos.second.GetCloseableQuantityWithOrderId(order_id)) {
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
