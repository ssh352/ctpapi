#include "position.h"
#include <boost/assert.hpp>

Position::Position(double margin_rate, int constract_multiple)
    : margin_rate_(margin_rate), constract_multiple_(constract_multiple) {}

void Position::TradedOpen(OrderDirection direction,
                          double price,
                          int last_traded_qty,
                          double* adjust_margin) {
  *adjust_margin = price * last_traded_qty * margin_rate_ * constract_multiple_;
  if (direction == OrderDirection::kBuy) {
    total_long_ += price * last_traded_qty;
    long_qty_ += last_traded_qty;
    long_avg_price_ = total_long_ / long_qty_;
  } else {
    total_short_ += price * last_traded_qty;
    short_qty_ += last_traded_qty;
    short_avg_price_ = total_short_ / short_qty_;
  }
}

void Position::TradedClose(OrderDirection direction,
                           double traded_price,
                           int last_traded_qty,
                           double* pnl,
                           double* add_cash,
                           double* release_margin,
                           double* unrealised_pnl) {
  *pnl =
      direction == OrderDirection::kBuy
          ? short_avg_price_ * last_traded_qty - traded_price * last_traded_qty
          : traded_price * last_traded_qty - long_avg_price_ * last_traded_qty;

  *pnl *= constract_multiple_;

  *add_cash =
      (direction == OrderDirection::kBuy ? short_avg_price_ : long_avg_price_) *
      last_traded_qty * constract_multiple_ * margin_rate_;

  *release_margin = *add_cash;

  if (direction == OrderDirection::kBuy) {
    BOOST_ASSERT(short_qty_ >= last_traded_qty);
    short_qty_ -= last_traded_qty;
    if (short_qty_ == 0) {
      short_avg_price_ = 0.0;
      total_short_ = 0.0;
    } else {
      total_short_ -= short_avg_price_ * last_traded_qty;
    }
  } else {
    BOOST_ASSERT(long_qty_ >= last_traded_qty);
    long_qty_ -= last_traded_qty;
    if (long_qty_ == 0) {
      long_avg_price_ = 0.0;
      total_long_ = 0.0;
    } else {
      total_long_ -= long_avg_price_ * last_traded_qty;
    }
  }
  if (IsEmptyQty()) {
    *unrealised_pnl = -unrealised_pnl_;
    unrealised_pnl_ = 0.0;
  } else {
    UpdateMarketPrice(traded_price, unrealised_pnl);
  }
}

void Position::UpdateMarketPrice(double price, double* update_pnl) {
  double lastst_unrealised_pnl_ =
      (price * long_qty_ - long_avg_price_ * long_qty_) +
      (short_avg_price_ * short_qty_ - price * short_qty_);
  lastst_unrealised_pnl_ *= constract_multiple_;
  *update_pnl = lastst_unrealised_pnl_ - unrealised_pnl_;
  unrealised_pnl_ = lastst_unrealised_pnl_;
}

bool Position::IsEmptyQty() const {
  return long_qty_ == 0 && short_qty_ == 0;
}
