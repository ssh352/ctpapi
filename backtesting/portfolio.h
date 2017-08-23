#ifndef BACKTESTING_PROTFOLIO_H
#define BACKTESTING_PROTFOLIO_H
#include <memory>
#include <unordered_map>
#include "common/api_struct.h"
#include "margin_rate_mode.h"
#include "cost_basis_mode.h"

struct Position {
  double long_avg_price;
  double short_avg_price;
  int long_qty;
  int short_qty;
  double total_long;
  double total_short;
  double unrealised_pnl;
};

class Portfolio {
 public:
  Portfolio(double init_cash);

  void UpdateTick(const std::shared_ptr<TickData>& tick);

  void HandleOrder(const std::shared_ptr<OrderField>& order);

  double total_value() const {
    return cash_ + frozen_cash_ + total_long_and_short_ + unrealised_pnl_;
  }

  double realised_pnl() const { return realised_pnl_; }

  double unrealised_pnl() const { return unrealised_pnl_; }

  double frozen_cash() const { return frozen_cash_; }

  double cash() const { return cash_; }

 private:
  double init_cash_ = 0.0;
  double cash_ = 0.0;
  double frozen_cash_ = 0.0;
  double market_value_ = 0.0;
  double realised_pnl_ = 0.0;
  double unrealised_pnl_ = 0.0;
  double total_long_and_short_ = 0.0;
  std::unordered_map<std::string, std::shared_ptr<OrderField> >
      order_container_;
  std::unordered_map<std::string, Position> position_container_;
  void UpdateUnrealisedPNL(const std::string& instrument, double last_price);
};

#endif  // BACKTESTING_PROTFOLIO_H
