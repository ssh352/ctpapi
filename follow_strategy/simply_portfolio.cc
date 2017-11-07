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
