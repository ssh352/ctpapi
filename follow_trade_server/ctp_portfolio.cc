#include "ctp_portfolio.h"
#include <boost/lexical_cast.hpp>

void CTPPortfolio::InitYesterdayPosition(std::vector<OrderPosition> positions) {
  for (auto position : positions) {
    auto position_key = std::make_pair(
        position.instrument, position.order_direction == OrderDirection::kBuy
                                 ? THOST_FTDC_D_Buy
                                 : THOST_FTDC_D_Sell);
    instrument_positions_[position_key].InitYesterdayPosition(
        position.quantity);
  }
}

void CTPPortfolio::OnRtnOrder(CThostFtdcOrderField order) {
  auto key = std::make_pair(order.SessionID, order.OrderRef);
  auto it = active_orders_.find(key);
  if (it == active_orders_.end()) {
    active_orders_[key] = std::move(order);
  } else {
    if (order.OrderStatus == THOST_FTDC_OST_AllTraded ||
        order.OrderStatus == THOST_FTDC_OST_Canceled) {
      active_orders_.erase(it);
    } else {
      active_orders_[key] = std::move(order);
    }
    auto position_key = std::make_pair(order.InstrumentID, order.Direction);
    instrument_positions_[position_key].OnRtnOrder(it->second, order);
  }
}
