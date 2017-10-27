#include "position.h"
#include <boost/assert.hpp>

Position::Position(std::string instrument,
                   OrderDirection direction,
                   double margin_rate,
                   int constract_multiple,
                   CostBasis cost_basis)
    : instrument_(instrument),
      direction_(direction),
      margin_rate_(margin_rate),
      constract_multiple_(constract_multiple),
      cost_basis_(std::move(cost_basis)) {}

void Position::TradedOpen(double price,
                          int last_traded_qty,
                          double* adjust_margin) {
  *adjust_margin = price * last_traded_qty * margin_rate_ * constract_multiple_;
  total_ += price * last_traded_qty;
  qty_ += last_traded_qty;
  avg_price_ = total_ / qty_;
  frozen_open_qty_ -= last_traded_qty;
}

void Position::OpenOrder(int qty) {
  frozen_open_qty_ += qty;
}

void Position::InputClose(int qty) {
  frozen_qty_ += qty;
}

void Position::CancelOpenOrder(int leave_qty) {
  frozen_open_qty_ -= leave_qty;
}

void Position::TradedClose(double traded_price,
                           int last_traded_qty,
                           double* pnl,
                           double* add_cash,
                           double* release_margin,
                           double* unrealised_pnl) {
  *pnl = direction_ == OrderDirection::kBuy
             ? traded_price * last_traded_qty - avg_price_ * last_traded_qty
             : avg_price_ * last_traded_qty - traded_price * last_traded_qty;

  *pnl *= constract_multiple_;

  *add_cash = avg_price_ * last_traded_qty * constract_multiple_ * margin_rate_;

  *release_margin = *add_cash;

  BOOST_ASSERT(qty_ >= last_traded_qty);
  BOOST_ASSERT(frozen_qty_ >= last_traded_qty);
  qty_ -= last_traded_qty;
  frozen_qty_ -= last_traded_qty;
  if (qty_ == 0) {
    avg_price_ = 0.0;
    total_ = 0.0;
  } else {
    total_ -= avg_price_ * last_traded_qty;
  }

  if (qty_ == 0) {
    *unrealised_pnl = -unrealised_pnl_;
    unrealised_pnl_ = 0.0;
  } else {
    UpdateMarketPrice(traded_price, unrealised_pnl);
  }
}

void Position::UpdateMarketPrice(double price, double* update_pnl) {
  double lastst_unrealised_pnl_ = direction_ == OrderDirection::kBuy
                                      ? (price - avg_price_) * qty_
                                      : (avg_price_ - price) * qty_;

  lastst_unrealised_pnl_ *= constract_multiple_;
  *update_pnl = lastst_unrealised_pnl_ - unrealised_pnl_;
  unrealised_pnl_ = lastst_unrealised_pnl_;
}

void Position::Reset() {
  frozen_open_qty_ = 0;
  frozen_qty_ = 0;
}
