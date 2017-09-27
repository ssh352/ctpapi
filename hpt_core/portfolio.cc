#include "portfolio.h"
#include <algorithm>
#include <boost/assert.hpp>

bool IsOpenPositionEffect(PositionEffect position_effect) {
  return position_effect == PositionEffect::kOpen;
}

Portfolio::Portfolio(double init_cash) {
  init_cash_ = init_cash;
  cash_ = init_cash;
}

void Portfolio::ResetByNewTradingDate() {
  daily_commission_ = 0;
  order_container_.clear();
  for (auto& key_value : position_container_) {
    key_value.second.Reset();
  }
}

void Portfolio::InitInstrumentDetail(std::string instrument,
                                     double margin_rate,
                                     int constract_multiple,
                                     CostBasis cost_basis) {
  instrument_info_container_.insert(
      {std::move(instrument),
       {margin_rate, constract_multiple, std::move(cost_basis)}});
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
      BOOST_ASSERT(order->trading_qty == 0);
      BOOST_ASSERT_MSG(instrument_info_container_.find(order->instrument_id) !=
                           instrument_info_container_.end(),
                       "The Instrument info must be found!");
      double margin_rate = 0.0;
      int constract_multiple = 0;
      CostBasis cost_basis;
      memset(&cost_basis, 0, sizeof(CostBasis));
      std::tie(margin_rate, constract_multiple, cost_basis) =
          instrument_info_container_.at(order->instrument_id);
      if (position_container_.find(order->instrument_id) ==
          position_container_.end()) {
        Position position(margin_rate, constract_multiple, cost_basis);
        position_container_.insert({order->instrument_id, std::move(position)});
      }
      Position& position = position_container_.at(order->instrument_id);
      position.OpenOrder(order->direction, order->qty);
      double frozen_cash =
          order->input_price * order->qty * margin_rate * constract_multiple +
          CalcCommission(PositionEffect::kOpen, order->input_price, order->qty,
                         constract_multiple, cost_basis);
      frozen_cash_ += frozen_cash;
      cash_ -= frozen_cash;
    }

    order_container_.insert({order->order_id, order});
  } else {
    const auto& previous_order = order_container_.at(order->order_id);
    int last_traded_qty = previous_order->leaves_qty - order->leaves_qty;
    switch (order->status) {
      case OrderStatus::kActive:
      case OrderStatus::kAllFilled: {
        if (IsOpenPositionEffect(order->position_effect)) {  // Open
          Position& position = position_container_.at(order->instrument_id);
          double add_margin = 0.0;
          position.TradedOpen(order->direction, order->input_price,
                              last_traded_qty, &add_margin);
          frozen_cash_ -= add_margin;
          margin_ += add_margin;
        } else {  // Close
          BOOST_ASSERT(position_container_.find(order->instrument_id) !=
                       position_container_.end());
          Position& position = position_container_.at(order->instrument_id);
          double pnl = 0.0;
          double add_cash = 0.0;
          double release_margin = 0.0;
          double update_unrealised_pnl = 0.0;
          position.TradedClose(order->direction, order->input_price,
                               last_traded_qty, &pnl, &add_cash,
                               &release_margin, &update_unrealised_pnl);
          margin_ -= release_margin;
          cash_ += add_cash + pnl;
          realised_pnl_ += pnl;
          unrealised_pnl_ += update_unrealised_pnl;
          if (position.IsEmptyQty()) {
            position_container_.erase(order->instrument_id);
          }
        }

        if (order->status == OrderStatus::kAllFilled) {
          BOOST_ASSERT(instrument_info_container_.find(order->instrument_id) !=
                       instrument_info_container_.end());
          if (instrument_info_container_.find(order->instrument_id) !=
              instrument_info_container_.end()) {
            double margin_rate = 0;
            int constract_mutiple = 0;
            CostBasis cost_basis;
            std::tie(margin_rate, constract_mutiple, cost_basis) =
                instrument_info_container_.at(order->instrument_id);
            double commission =
                UpdateCostBasis(order->position_effect, order->input_price,
                                order->qty, constract_mutiple, cost_basis);
            if (IsOpenPositionEffect(order->position_effect)) {
              frozen_cash_ -= commission;
            } else {
              cash_ -= commission;
            }
          }
        }
      } break;
      case OrderStatus::kCanceled: {
        BOOST_ASSERT(instrument_info_container_.find(order->instrument_id) !=
                     instrument_info_container_.end());
        if (instrument_info_container_.find(order->instrument_id) !=
            instrument_info_container_.end()) {
          double margin_rate = 0;
          int constract_mutiple = 0;
          CostBasis cost_basis;
          std::tie(margin_rate, constract_mutiple, cost_basis) =
              instrument_info_container_.at(order->instrument_id);
          (void)UpdateCostBasis(order->position_effect, order->input_price,
                                order->qty - order->leaves_qty,
                                constract_mutiple, cost_basis);
          if (IsOpenPositionEffect(order->position_effect)) {
            double unfrozen_margin = order->leaves_qty * order->input_price *
                                     margin_rate * constract_mutiple;
            double unfrozen_cash =
                unfrozen_margin + CalcCommission(order->position_effect,
                                                 order->input_price, order->qty,
                                                 constract_mutiple, cost_basis);
            frozen_cash_ -= unfrozen_cash;
            cash_ += unfrozen_margin +
                     CalcCommission(order->position_effect, order->input_price,
                                    order->leaves_qty, constract_mutiple,
                                    cost_basis);
            Position& position = position_container_.at(order->instrument_id);
            position.CancelOpenOrder(order->direction, order->leaves_qty);
            if (position.IsEmptyQty()) {
              position_container_.erase(order->instrument_id);
            }
          }
        }
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

void Portfolio::HandleNewInputCloseOrder(const std::string& instrument,
                                         OrderDirection direction,
                                         int qty) {
  BOOST_ASSERT(position_container_.find(instrument) !=
               position_container_.end());
  if (position_container_.find(instrument) != position_container_.end()) {
    auto& position = position_container_.at(instrument);
    position.InputClose(direction, qty);
  }
}

int Portfolio::GetPositionCloseableQty(const std::string& instrument,
                                       OrderDirection direction) const {
  // TODO: Should be tag warrning!
  //   BOOST_ASSERT(position_container_.find(instrument) !=
  //                position_container_.end());

  if (position_container_.find(instrument) != position_container_.end()) {
    const auto& position = position_container_.at(instrument);
    return direction == OrderDirection::kBuy ? position.long_closeable_qty()
                                             : position.short_closeable_qty();
  }

  return 0;
}

double Portfolio::UpdateCostBasis(PositionEffect position_effect,
                                  double price,
                                  int qty,
                                  int constract_multiple,
                                  const CostBasis& const_basis) {
  double commission = CalcCommission(position_effect, price, qty,
                                     constract_multiple, const_basis);
  daily_commission_ += commission;
  return commission;
}

double Portfolio::CalcCommission(PositionEffect position_effect,
                                 double price,
                                 int qty,
                                 int constract_multiple,
                                 const CostBasis& cost_basis) {
  double commission_param = 0.0;
  switch (position_effect) {
    case PositionEffect::kOpen:
      commission_param = cost_basis.open_commission;
      break;
    case PositionEffect::kClose:
      commission_param = cost_basis.close_commission;
      break;
    case PositionEffect::kCloseToday:
      commission_param = cost_basis.close_today_commission;
      break;
    default:
      BOOST_ASSERT(false);
      break;
  }

  return cost_basis.type == CommissionType::kFixed
             ? qty * commission_param / 100.0
             : price * qty * constract_multiple * commission_param / 10000.0;
}

int Portfolio::UnfillQty(const std::string& instrument,
                         OrderDirection direction) const {
  return std::accumulate(
      order_container_.begin(), order_container_.end(), 0,
      [&instrument, direction](int val, const auto& key_value) {
        const std::shared_ptr<OrderField>& order = key_value.second;
        if (order->instrument_id != instrument ||
            order->direction != direction) {
          return val;
        }
        return val +
               (order->status == OrderStatus::kActive ? order->leaves_qty : 0);
      });
}

std::vector<std::shared_ptr<OrderField> > Portfolio::UnfillOrders(
    const std::string& instrument,
    OrderDirection direction) const {
  std::vector<std::shared_ptr<OrderField> > ret_orders;
  std::for_each(order_container_.begin(), order_container_.end(),
                [&ret_orders, &instrument, direction](const auto& key_value) {
                  const std::shared_ptr<OrderField>& order = key_value.second;
                  if (order->instrument_id != instrument ||
                      order->direction != direction) {
                    return;
                  }
                  ret_orders.push_back(order);
                });
  return std::move(ret_orders);
}

int Portfolio::PositionQty(const std::string& instrument,
                           OrderDirection direction) const {
  if (position_container_.find(instrument) == position_container_.end()) {
    return 0;
  }
  return direction == OrderDirection::kBuy
             ? position_container_.at(instrument).long_qty()
             : position_container_.at(instrument).short_qty();
}
