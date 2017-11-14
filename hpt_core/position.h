#ifndef BACKTESTING_POSITION_H
#define BACKTESTING_POSITION_H
#include "common/api_struct.h"
class Position {
 public:
  Position(std::string instrument,
           OrderDirection direction,
           double margin_rate,
           int constract_multiple,
           CostBasis cost_basis);

  void TradedOpen(double price, int last_traded_qty, double* adjust_margin);

  void OpenOrder(int qty);

  void InputClose(int qty);

  void CancelOpenOrder(int leave_qty);

  void CancelCloseOrder(int qty);

  void TradedClose(double traded_price,
                   int last_traded_qty,
                   double* pnl,
                   double* add_cash,
                   double* release_margin,
                   double* unrealised_pnl);

  void UpdateMarketPrice(double price, double* update_pnl);

  void Reset();

  int closeable_qty() const { return qty_ - frozen_qty_; }

  int qty() const { return qty_; }

  int frozen_qty() const { return frozen_qty_; }
  
  int frozen_open_qty() const { return frozen_open_qty_; }

  const std::string instrument() const { return instrument_; }

  OrderDirection direction() const { return direction_; }

 private:
  double avg_price_ = 0.0;
  int qty_ = 0;
  int frozen_open_qty_ = 0;
  int frozen_qty_ = 0;
  double total_ = 0.0;
  double margin_ = 0.0;
  double unrealised_pnl_ = 0.0;
  int constract_multiple_ = 0;
  double margin_rate_ = 0.0;
  CostBasis cost_basis_;
  const std::string instrument_;
  const OrderDirection direction_;
};

#endif  // BACKTESTING_POSITION_H
