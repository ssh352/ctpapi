#include "portfolio.h"
#include <boost/assert.hpp>

bool IsOpenPositionEffect(PositionEffect position_effect) {
  return position_effect == PositionEffect::kOpen;
}

Portfolio::Portfolio(double init_cash) {
  init_cash_ = init_cash;
  cash_ = init_cash;
}

void Portfolio::AddMargin(const std::string& instrument,
                          double margin_rate,
                          int constract_multiple) {
  instrument_info_container_.insert(
      {instrument, {margin_rate, constract_multiple}});
}

void Portfolio::UpdateTick(const std::shared_ptr<TickData>& tick) {
  if (position_container_.find(*tick->instrument) !=
      position_container_.end()) {
    Position& position = position_container_.at(*tick->instrument);
    double update_pnl_ = 0.0;
    position.UpdateMarketPrice(tick->tick->last_price, &update_pnl_);
    unrealised_pnl_ += update_pnl_;
  }
}

void Portfolio::HandleOrder(const std::shared_ptr<OrderField>& order) {
  if (order_container_.find(order->order_id) == order_container_.end()) {
    // New Order
    if (IsOpenPositionEffect(order->position_effect)) {
      BOOST_ASSERT(order->traded_qty == 0);
      BOOST_ASSERT(position_container_.find(order->instrument_id) ==
                   position_container_.end());
      BOOST_ASSERT_MSG(instrument_info_container_.find(order->instrument_id) !=
                           instrument_info_container_.end(),
                       "The Instrument info must be found!");
      auto ins_info = instrument_info_container_.at(order->instrument_id);
      Position position(ins_info.first, ins_info.second);
      position_container_.insert({order->instrument_id, std::move(position)});
      double frozen_cash =
          order->price * order->qty * ins_info.first * ins_info.second;
      frozen_cash_ += frozen_cash;
      cash_ -= frozen_cash;
    }
    order_container_.insert({order->order_id, order});
  } else {
    const auto& previous_order = order_container_.at(order->order_id);
    int last_traded_qty = order->traded_qty - previous_order->traded_qty;
    switch (order->status) {
      case OrderStatus::kActive:
      case OrderStatus::kAllFilled: {
        if (IsOpenPositionEffect(order->position_effect)) {
          Position& position = position_container_.at(order->instrument_id);
          double add_margin = 0.0;
          position.TradedOpen(order->direction, order->price, last_traded_qty,
                              &add_margin);
          frozen_cash_ -= add_margin;
          margin_ += add_margin;
        } else {
          BOOST_ASSERT(position_container_.find(order->instrument_id) !=
                       position_container_.end());
          Position& position = position_container_.at(order->instrument_id);
          double pnl = 0.0;
          double add_cash = 0.0;
          double release_margin = 0.0;
          double update_unrealised_pnl = 0.0;
          position.TradedClose(order->direction, order->price, last_traded_qty,
                               &pnl, &add_cash, &release_margin,
                               &update_unrealised_pnl);
          margin_ -= release_margin;
          cash_ += add_cash + pnl;
          realised_pnl_ += pnl;
          unrealised_pnl_ += update_unrealised_pnl;
          if (position.IsEmptyQty()) {
            position_container_.erase(order->instrument_id);
          }
        }
      } break;
      case OrderStatus::kCanceled:
        break;
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
