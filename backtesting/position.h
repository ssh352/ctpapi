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

  void TradedClose(OrderDirection direction,
                   double traded_price,
                   int last_traded_qty,
                   double* pnl,
                   double* add_cash,
                   double* release_margin,
                   double* unrealised_pnl);

  void UpdateMarketPrice(double price, double* update_pnl);

  bool IsEmptyQty() const;

 private:
  double long_avg_price_ = 0.0;
  double short_avg_price_ = 0.0;
  int long_qty_ = 0;
  int short_qty_ = 0;
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
