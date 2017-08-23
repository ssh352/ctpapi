#ifndef BACKTESTING_PORTFOLIO_HANDLER_H
#define BACKTESTING_PORTFOLIO_HANDLER_H
#include <fstream>
#include "common/api_struct.h"

class AbstractPortfolioHandler {
 public:
  virtual void HandleTick(const std::shared_ptr<TickData>& tick) = 0;
  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) = 0;
  virtual void HandleCloseMarket() = 0;
};

class BacktestingPortfolioHandler : public AbstractPortfolioHandler {
 public:
  BacktestingPortfolioHandler(double init_cash) : csv_("equitys.csv") {}
  virtual void HandleTick(const std::shared_ptr<TickData>& tick) override {
    // portfolio_.UpdateTick(tick);
    last_tick_ = tick->tick;
  }

  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) override {
    // portfolio_.TransactPosition(order);
  }

  virtual void HandleCloseMarket() override {
    // csv_ << last_tick_->timestamp << "," << portfolio_.total_value() << "\n";
  }

 private:
  // Portfolio portfolio_;
  std::shared_ptr<Tick> last_tick_;
  std::ofstream csv_;
};
#endif  // BACKTESTING_PORTFOLIO_HANDLER_H
