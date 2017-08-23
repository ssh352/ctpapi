#include "portfolio.h"
#include <boost/assert.hpp>

bool IsOpenPositionEffect(PositionEffect position_effect) {
  return position_effect == PositionEffect::kOpen;
}

Portfolio::Portfolio(double init_cash) {
  init_cash_ = init_cash;
  cash_ = init_cash;
}

void Portfolio::UpdateTick(const std::shared_ptr<TickData>& tick) {
  UpdateUnrealisedPNL(*tick->instrument, tick->tick->last_price);
}

void Portfolio::HandleOrder(const std::shared_ptr<OrderField>& order) {
  if (order_container_.find(order->order_id) == order_container_.end()) {
    // New Order
    if (IsOpenPositionEffect(order->position_effect)) {
      BOOST_ASSERT(order->traded_qty == 0);
      frozen_cash_ += order->price * order->qty;
      cash_ -= order->price * order->qty;
    }
    order_container_.insert({order->order_id, order});
  } else {
    const auto& previous_order = order_container_.at(order->order_id);
    int last_traded_qty = order->traded_qty - previous_order->traded_qty;
    switch (order->status) {
      case OrderStatus::kActive:
      case OrderStatus::kAllFilled: {
        if (IsOpenPositionEffect(order->position_effect)) {
          double unfrozen_cash = order->price * last_traded_qty;
          frozen_cash_ -= unfrozen_cash;
          if (position_container_.find(order->instrument_id) ==
              position_container_.end()) {
            Position position{0};
            position_container_.insert(
                {order->instrument_id, std::move(position)});
          }
          Position& position = position_container_.at(order->instrument_id);
          total_long_and_short_ -= (position.total_long + position.total_short);
          if (order->direction == OrderDirection::kBuy) {
            position.total_long += order->price * last_traded_qty;
            position.long_qty += last_traded_qty;
            position.long_avg_price = position.total_long / position.long_qty;
          } else {
            position.total_short += order->price * last_traded_qty;
            position.short_qty += last_traded_qty;
            position.short_avg_price =
                position.total_short / position.short_qty;
          }
          total_long_and_short_ += position.total_long + position.total_short;
        } else {
          BOOST_ASSERT(position_container_.find(order->instrument_id) !=
                       position_container_.end());
          Position& position = position_container_.at(order->instrument_id);
          total_long_and_short_ -= (position.total_long + position.total_short);

          double pnl = order->direction == OrderDirection::kBuy
                           ? position.short_avg_price * last_traded_qty -
                                 order->price * last_traded_qty
                           : order->price * last_traded_qty -
                                 position.long_avg_price * last_traded_qty;

          cash_ += (order->direction == OrderDirection::kBuy
                        ? position.short_avg_price
                        : position.long_avg_price) *
                       last_traded_qty +
                   pnl;
          // Close
          realised_pnl_ += pnl;
          unrealised_pnl_ -= pnl;
          position.unrealised_pnl -= pnl;
          if (order->direction == OrderDirection::kBuy) {
            BOOST_ASSERT(position.short_qty >= last_traded_qty);
            position.short_qty -= last_traded_qty;
            if (position.short_qty == 0) {
              position.short_avg_price = 0.0;
              position.total_short = 0.0;
            } else {
              position.total_short -=
                  position.short_avg_price * last_traded_qty;
            }
          } else {
            BOOST_ASSERT(position.long_qty >= last_traded_qty);
            position.long_qty -= last_traded_qty;
            if (position.long_qty == 0) {
              position.long_avg_price = 0.0;
              position.total_long = 0.0;
            } else {
              position.total_long -= position.long_avg_price * last_traded_qty;
            }
          }
          total_long_and_short_ += position.total_long + position.total_short;
          if (position.long_qty == 0 && position.short_qty == 0) {
            unrealised_pnl_ -= position.unrealised_pnl;
            position_container_.erase(order->instrument_id);
          } else {
            UpdateUnrealisedPNL(order->instrument_id, order->price);
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

void Portfolio::UpdateUnrealisedPNL(const std::string& instrument,
                                    double price) {
  if (position_container_.find(instrument) != position_container_.end()) {
    Position& position = position_container_.at(instrument);
    unrealised_pnl_ -= position.unrealised_pnl;
    position.unrealised_pnl = (price * position.long_qty -
                               position.long_avg_price * position.long_qty) +
                              (position.short_avg_price * position.short_qty -
                               price * position.short_qty);

    unrealised_pnl_ += position.unrealised_pnl;
  }
}
