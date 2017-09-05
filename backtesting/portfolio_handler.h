#ifndef BACKTESTING_PORTFOLIO_HANDLER_H
#define BACKTESTING_PORTFOLIO_HANDLER_H
#include <fstream>
#include "common/api_struct.h"
#include "portfolio.h"
#include "event_factory.h"

class AbstractPortfolioHandler {
 public:
  virtual void HandleTick(const std::shared_ptr<TickData>& tick) = 0;
  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) = 0;
  virtual void HandleCloseMarket() = 0;
  virtual void HandlerInputOrder(const InputOrder& input_order) = 0;
};

class BacktestingPortfolioHandler : public AbstractPortfolioHandler {
 public:
  BacktestingPortfolioHandler(double init_cash,
                              AbstractEventFactory* event_factory,
                              std::string instrument,
                              const std::string& csv_file_prefix,
                              double margin_rate,
                              int constract_multiple,
                              CostBasis cost_basis);
  virtual void HandleTick(const std::shared_ptr<TickData>& tick) override;

  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) override;

  virtual void HandleCloseMarket() override;

  virtual void HandlerInputOrder(const InputOrder& input_order) override;

 private:
  Portfolio portfolio_;
  std::shared_ptr<Tick> last_tick_;
  std::ofstream csv_;
  AbstractEventFactory* event_factory_;
};
#endif  // BACKTESTING_PORTFOLIO_HANDLER_H
