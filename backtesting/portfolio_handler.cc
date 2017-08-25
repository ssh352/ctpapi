#include "portfolio_handler.h"

BacktestingPortfolioHandler::BacktestingPortfolioHandler(double init_cash)
    : portfolio_(init_cash), csv_("equitys.csv") {
  portfolio_.InitInstrumentDetail(
      "m1705", 0.1, 10, CostBasis{CommissionType::kFixed, 158, 158, 158});
}

void BacktestingPortfolioHandler::HandleTick(
    const std::shared_ptr<TickData>& tick) {
  portfolio_.UpdateTick(tick);
  last_tick_ = tick->tick;
}

void BacktestingPortfolioHandler::HandleOrder(
    const std::shared_ptr<OrderField>& order) {
  portfolio_.HandleOrder(order);
}

void BacktestingPortfolioHandler::HandleCloseMarket() {
  csv_ << last_tick_->timestamp << "," << portfolio_.total_value() << "\n";
}
