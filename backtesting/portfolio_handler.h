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
  virtual void HandlerInputOrder(const std::string& instrument,
                                 PositionEffect position_effect,
                                 OrderDirection direction,
                                 double price,
                                 int qty,
                                 TimeStamp timestamp) = 0;
};

class BacktestingPortfolioHandler : public AbstractPortfolioHandler {
 public:
  BacktestingPortfolioHandler(double init_cash,
                              AbstractEventFactory* event_factory);
  virtual void HandleTick(const std::shared_ptr<TickData>& tick) override;

  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) override;

  virtual void HandleCloseMarket() override;

  virtual void HandlerInputOrder(const std::string& instrument,
                                 PositionEffect position_effect,
                                 OrderDirection direction,
                                 double price,
                                 int qty,
                                 TimeStamp timestamp) override;

 private:
  Portfolio portfolio_;
  std::shared_ptr<Tick> last_tick_;
  std::ofstream csv_;
  AbstractEventFactory* event_factory_;
};
#endif  // BACKTESTING_PORTFOLIO_HANDLER_H
