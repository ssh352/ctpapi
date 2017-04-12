#include "instrument_position.h"
#include "close_corr_orders_manager.h"
#include "order_util.h"

std::vector<OrderQuantity> InstrumentPosition::GetQuantitys(
    std::vector<std::string> orders) {
  std::vector<OrderQuantity> quanitys;
  for (auto order : orders) {
    auto it = positions_.find(order);
    if (it != positions_.end()) {
      quanitys.emplace_back(
          OrderQuantity{it->second.order_id, it->second.direction,
                        it->second.quantity, it->second.closeable_quantity});
    }
  }

  return quanitys;
}

int InstrumentPosition::GetCloseableQuantityWithOrderDirection(
    OrderDirection direction) const {
  return std::accumulate(positions_.begin(), positions_.end(), 0,
                         [=](int val, auto order_position) {
                           if (order_position.second.direction != direction) {
                             return val;
                           }
                           auto position = order_position.second;
                           return val + position.closeable_quantity;
                         });
}

int InstrumentPosition::GetCloseableQuantityWithInstrument(
    const std::string& order_id) const {
  if (positions_.find(order_id) == positions_.end()) {
    return 0;
  }
  return positions_.at(order_id).closeable_quantity;
}

void InstrumentPosition::HandleRtnOrder(
    const OrderData& rtn_order,
    CloseCorrOrdersManager* close_corr_orders_mgr) {
  if (rtn_order.status() != OrderStatus::kCancel &&
      IsOpenOrder(rtn_order.position_effect()) &&
      rtn_order.filled_quantity() != 0) {
    auto it = std::find_if(positions_.begin(), positions_.end(), [&](auto pos) {
      return pos.second.order_id == rtn_order.order_id();
    });

    if (it != positions_.end()) {
      int new_quantity = rtn_order.filled_quantity() - it->second.quantity;
      it->second.quantity += new_quantity;
      it->second.closeable_quantity += new_quantity;
    } else {
      positions_[rtn_order.order_id()] = {
          rtn_order.order_id(), rtn_order.direction(),
          rtn_order.filled_quantity(), rtn_order.filled_quantity()};
    }
  } else if (close_corr_orders_mgr->IsNewCloseOrder(rtn_order)) {
    std::vector<std::pair<std::string, int> > close_corr_orders;
    int outstanding_quantity = rtn_order.quanitty();
    for (auto& pos : positions_) {
      if (pos.second.direction == rtn_order.direction()) {
        continue;
      }
      int close_quantity =
          std::min<int>(outstanding_quantity, pos.second.closeable_quantity);
      close_corr_orders.emplace_back(pos.second.order_id, close_quantity);
      outstanding_quantity -= close_quantity;
      pos.second.closeable_quantity -= close_quantity;
      if (outstanding_quantity <= 0) {
        break;
      }
    }
    close_corr_orders_mgr->AddCloseCorrOrders(rtn_order.order_id(),
                                              std::move(close_corr_orders));
  } else if (rtn_order.status() == OrderStatus::kCancel) {
    int fill_quantity = rtn_order.filled_quantity();
    for (auto order_quantity :
         close_corr_orders_mgr->GetCorrOrderQuantiys(rtn_order.order_id())) {
      if (positions_.find(order_quantity.first) != positions_.end()) {
        positions_[order_quantity.first].closeable_quantity +=
            order_quantity.second;
      }
    }
  } else {
  }
}
