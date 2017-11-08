#include "simply_portfolio.h"

void SimplyPortfolio::HandleOrder(const std::shared_ptr<OrderField>& order) {
  if (order->trading_qty == 0 && order->status == OrderStatus::kActive) {
    // New Order
    if (order->position_effect == PositionEffect::kOpen) {
      auto it =
          positions_.find(PositionKey{order->instrument_id, order->direction});
      if (it == positions_.end()) {
        positions_.insert({PositionKey{order->instrument_id, order->direction},
                           JustQtyPosition()});
      }
    } else {
      auto it =
          positions_.find(PositionKey{order->instrument_id, order->direction});
      BOOST_ASSERT(it != positions_.end());
      if (it != positions_.end()) {
        it->second.Freeze(order->qty);
      }
    }

    BOOST_ASSERT(order_container_.find(order->order_id) ==
                 order_container_.end());
    order_container_.insert({order->order_id, order});
  } else {
    switch (order->status) {
      case OrderStatus::kActive:
      case OrderStatus::kAllFilled: {
        auto it_pos = positions_.find(
            PositionKey{order->instrument_id, order->direction});
        BOOST_VERIFY(it_pos != positions_.end());
        if (order->position_effect == PositionEffect::kOpen) {  // Open
          it_pos->second.OpenTraded(order->trading_qty);
        } else {  // Close
          it_pos->second.CloseTraded(order->trading_qty);
        }
      } break;
      case OrderStatus::kCanceled: {
        auto it_pos = positions_.find(
            PositionKey{order->instrument_id, order->direction});
        BOOST_ASSERT(it_pos != positions_.end());
        it_pos->second.Unfreeze(order->leaves_qty);
      } break;
      case OrderStatus::kInputRejected:
        break;
      case OrderStatus::kCancelRejected:
        break;
      default:
        break;
    }

    order_container_[order->order_id] = order;
  }
}

int SimplyPortfolio::GetFrozenQty(const std::string& instrument,
                                  OrderDirection direction) const {
  auto it = positions_.find(PositionKey{instrument, direction});
  if (it == positions_.end()) {
    return 0;
  }
  return it->second.frozen();
}

int SimplyPortfolio::GetPositionQty(const std::string& instrument,
                                    OrderDirection direction) const {
  auto it = positions_.find(PositionKey{instrument, direction});
  if (it == positions_.end()) {
    return 0;
  }
  return it->second.qty();
}

int SimplyPortfolio::GetPositionCloseableQty(const std::string& instrument,
                                             OrderDirection direction) const {
  auto it = positions_.find(PositionKey{instrument, direction});
  if (it == positions_.end()) {
    return 0;
  }
  return it->second.Closeable();
}

int SimplyPortfolio::UnfillOpenQty(const std::string& instrument,
                                   OrderDirection direction) const {
  return std::accumulate(
      order_container_.begin(), order_container_.end(), 0,
      [&instrument, direction](int val, const auto& key_value) {
        const std::shared_ptr<OrderField>& order = key_value.second;
        if (order->position_effect == PositionEffect::kClose ||
            order->instrument_id != instrument ||
            order->direction != direction) {
          return val;
        }
        return val +
               (order->status == OrderStatus::kActive ? order->leaves_qty : 0);
      });
}

int SimplyPortfolio::UnfillCloseQty(const std::string& instrument,
                                    OrderDirection direction) const {
  return std::accumulate(
      order_container_.begin(), order_container_.end(), 0,
      [&instrument, direction](int val, const auto& key_value) {
        const std::shared_ptr<OrderField>& order = key_value.second;
        if (order->position_effect == PositionEffect::kOpen ||
            order->instrument_id != instrument ||
            order->direction != direction) {
          return val;
        }
        return val +
               (order->status == OrderStatus::kActive ? order->leaves_qty : 0);
      });
}

std::vector<std::shared_ptr<OrderField>> SimplyPortfolio::UnfillOpenOrders(
    const std::string& instrument,
    OrderDirection direction) const {
  std::vector<std::shared_ptr<OrderField>> ret_orders;
  std::for_each(order_container_.begin(), order_container_.end(),
                [&ret_orders, &instrument, direction](const auto& key_value) {
                  const std::shared_ptr<OrderField>& order = key_value.second;
                  if (order->position_effect == PositionEffect::kClose ||
                      order->instrument_id != instrument ||
                      order->direction != direction ||
                      order->status != OrderStatus::kActive) {
                    return;
                  }
                  ret_orders.push_back(order);
                });
  return std::move(ret_orders);
}

std::vector<std::shared_ptr<OrderField>> SimplyPortfolio::UnfillCloseOrders(
    const std::string& instrument,
    OrderDirection direction) const {
  std::vector<std::shared_ptr<OrderField>> ret_orders;
  std::for_each(order_container_.begin(), order_container_.end(),
                [&ret_orders, &instrument, direction](const auto& key_value) {
                  const std::shared_ptr<OrderField>& order = key_value.second;
                  if (order->position_effect == PositionEffect::kOpen ||
                      order->instrument_id != instrument ||
                      order->direction != direction ||
                      order->status != OrderStatus::kActive) {
                    return;
                  }
                  ret_orders.push_back(order);
                });
  return std::move(ret_orders);
}

std::shared_ptr<OrderField> SimplyPortfolio::GetOrder(
    const std::string& order_id) const {
  BOOST_ASSERT(order_container_.find(order_id) != order_container_.end());
  auto it = order_container_.find(order_id);
  if (it == order_container_.end()) {
    return std::shared_ptr<OrderField>();
  }

  return it->second;
}
