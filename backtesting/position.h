#ifndef BACKTESTING_POSITION_H
#define BACKTESTING_POSITION_H
#include "common/api_struct.h"
class Position {
 public:
  Position(double margin_rate, int constract_multiple, CostBasis cost_basis);
  void TradedOpen(OrderDirection direction,
                  double price,
                  int last_traded_qty,
                  double* adjust_margin);

  void OpenOrder(OrderDirection direciton, int qty);

  void InputClose(OrderDirection direction, int qty);

  void CancelOpenOrder(OrderDirection direction, int leave_qty);

  void TradedClose(OrderDirection direction,
                   double traded_price,
                   int last_traded_qty,
                   double* pnl,
                   double* add_cash,
                   double* release_margin,
                   double* unrealised_pnl);

  void UpdateMarketPrice(double price, double* update_pnl);

  bool IsEmptyQty() const;

  int long_closeable_qty() const { return long_qty_ - frozen_long_qty_; }
  int short_closeable_qty() const { return short_qty_ - frozen_short_qty_; }

  int long_qty() const { return long_qty_; }

  int short_qty() const { return short_qty_; }

 private:
  double long_avg_price_ = 0.0;
  double short_avg_price_ = 0.0;
  int long_qty_ = 0;
  int short_qty_ = 0;
  int frozen_open_long_qty_ = 0;
  int frozen_open_short_qty_ = 0;
  int frozen_long_qty_ = 0;
  int frozen_short_qty_ = 0;
  double total_long_ = 0.0;
  double total_short_ = 0.0;
  double long_margin_ = 0.0;
  double short_margin_ = 0.0;
  double unrealised_pnl_ = 0.0;
  int constract_multiple_ = 0;
  double margin_rate_ = 0.0;
  CostBasis cost_basis_;
};

#endif  // BACKTESTING_POSITION_H
