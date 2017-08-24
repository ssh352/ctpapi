#ifndef BACKTESTING_PROTFOLIO_H
#define BACKTESTING_PROTFOLIO_H
#include <memory>
#include <unordered_map>
#include "common/api_struct.h"
#include "position.h"

class Portfolio {
 public:
  Portfolio(double init_cash);

  void AddMargin(const std::string& instrument,
                 double margin_rate,
                 int constract_multiple);

  void UpdateTick(const std::shared_ptr<TickData>& tick);

  void HandleOrder(const std::shared_ptr<OrderField>& order);

  double total_value() const {
    return cash_ + frozen_cash_ + margin_ + unrealised_pnl_;
  }

  double realised_pnl() const { return realised_pnl_; }

  double unrealised_pnl() const { return unrealised_pnl_; }

  double frozen_cash() const { return frozen_cash_; }

  double cash() const { return cash_; }

  double margin() const { return margin_; }

 private:
  double init_cash_ = 0.0;
  double cash_ = 0.0;
  double frozen_cash_ = 0.0;
  double market_value_ = 0.0;
  double realised_pnl_ = 0.0;
  double unrealised_pnl_ = 0.0;
  double margin_ = 0.0;
  std::unordered_map<std::string, std::shared_ptr<OrderField> >
      order_container_;
  std::unordered_map<std::string, Position> position_container_;
  std::unordered_map<std::string, std::pair<double, int> >
      instrument_info_container_;
};

#endif  // BACKTESTING_PROTFOLIO_H
