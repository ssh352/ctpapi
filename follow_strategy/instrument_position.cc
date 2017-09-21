#include "instrument_position.h"
#include "close_corr_orders_manager.h"
#include "order_util.h"
#include "string_util.h"

std::vector<OrderQuantity> InstrumentPosition::GetQuantitys(
    std::vector<std::string> orders) const {
  std::vector<OrderQuantity> quanitys;
  for (auto order : orders) {
    auto it = positions_.find(order);
    if (it != positions_.end()) {
      quanitys.emplace_back(
          OrderQuantity{it->second.order_id, it->second.direction,
                        it->second.is_today_quantity, it->second.quantity,
                        it->second.closeable_quantity});
    }
  }
  return quanitys;
}

std::vector<OrderQuantity> InstrumentPosition::GetQuantitysIf(
    std::function<bool(const OrderQuantity&)> cond) const {
  std::vector<OrderQuantity> quanitys;
  for (auto item : positions_) {
    if (cond(item.second)) {
      quanitys.emplace_back(
          OrderQuantity{item.second.order_id, item.second.direction,
                        item.second.is_today_quantity, item.second.quantity,
                        item.second.closeable_quantity});
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

boost::optional<int> InstrumentPosition::GetCloseableQuantityWithOrderId(
    const std::string& order_id) const {
  if (positions_.find(order_id) == positions_.end()) {
    return {};
  }
  return positions_.at(order_id).closeable_quantity;
}

void InstrumentPosition::HandleRtnOrder(
    const std::shared_ptr<const OrderField>& rtn_order,
    CloseCorrOrdersManager* close_corr_orders_mgr) {
  int traded_qty = rtn_order->qty - rtn_order->leaves_qty;
  if (rtn_order->status != OrderStatus::kCanceled &&
      IsOpenOrder(rtn_order->position_effect) && traded_qty != 0) {
    auto it = std::find_if(positions_.begin(), positions_.end(), [&](auto pos) {
      return pos.second.order_id == rtn_order->order_id;
    });

    if (it != positions_.end()) {
      int new_quantity = traded_qty - it->second.quantity;
      it->second.quantity += new_quantity;
      it->second.closeable_quantity += new_quantity;
    } else {
      positions_[rtn_order->order_id] = {rtn_order->order_id,
                                         rtn_order->direction, true, traded_qty,
                                         traded_qty};
    }
  } else if (close_corr_orders_mgr->IsNewCloseOrder(rtn_order)) {
    std::vector<std::pair<std::string, int> > close_corr_orders;
    int outstanding_quantity = rtn_order->qty;
    for (auto& pos : positions_) {
      if (pos.second.direction == rtn_order->direction ||
          pos.second.closeable_quantity == 0 ||
          !TestPositionEffect(rtn_order->exchange_id,
                              rtn_order->position_effect,
                              pos.second.is_today_quantity)) {
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
    close_corr_orders_mgr->AddCloseCorrOrders(rtn_order->order_id,
                                              std::move(close_corr_orders));
  } else if (rtn_order->status == OrderStatus::kCanceled) {
    for (auto order_quantity :
         close_corr_orders_mgr->GetCorrOrderQuantiys(rtn_order->order_id)) {
      if (positions_.find(order_quantity.first) != positions_.end()) {
        if (positions_[order_quantity.first].closeable_quantity !=
            positions_[order_quantity.first].quantity) {
          positions_[order_quantity.first].closeable_quantity +=
              order_quantity.second - traded_qty;
        }
      }
    }
  } else {
  }
}

void InstrumentPosition::AddQuantity(OrderQuantity quantity) {
  positions_.insert_or_assign(quantity.order_id, quantity);
}

bool InstrumentPosition::TestPositionEffect(const std::string& exchange_id,
                                            PositionEffect position_effect,
                                            bool is_today_quantity) {
  if (exchange_id != kSHFEExchangeId) {
    return true;
  }

  return ((position_effect == PositionEffect::kCloseToday &&
           is_today_quantity) ||
          (position_effect == PositionEffect::kClose && !is_today_quantity))
             ? true
             : false;
}
