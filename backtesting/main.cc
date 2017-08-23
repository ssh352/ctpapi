#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <fstream>
#include "caf/all.hpp"
#include "common/api_struct.h"
#include "event_factory.h"
#include "event.h"
#include "execution_handler.h"
#include "price_handler.h"
#include "strategy.h"
#include "tick_series_data_base.h"
#include "portfolio_handler.h"
#include "cta_transaction_series_data_base.h"

class AbstractExecutionHandler;

class TickEvent : public AbstractEvent {
 public:
  TickEvent(AbstractStrategy* strategy,
            AbstractExecutionHandler* execution_handler,
            AbstractPortfolioHandler* portfolio_handler,
            const std::shared_ptr<TickData>& tick)
      : strategy_(strategy),
        execution_handler_(execution_handler),
        portfolio_handler_(portfolio_handler),
        tick_(tick) {}
  virtual void Do() override {
    strategy_->HandleTick(tick_);
    execution_handler_->HandleTick(tick_);
    portfolio_handler_->HandleTick(tick_);
  }

 private:
  AbstractStrategy* strategy_;
  AbstractExecutionHandler* execution_handler_;
  AbstractPortfolioHandler* portfolio_handler_;
  std::shared_ptr<TickData> tick_;
};

class FillEvent : public AbstractEvent {
 public:
  FillEvent(AbstractStrategy* strategy,
            AbstractPortfolioHandler* portfolio_handler,
            const std::shared_ptr<OrderField>& order)
      : strategy_(strategy),
        portfolio_handler_(portfolio_handler),
        order_(order) {}

  virtual void Do() override {
    strategy_->HandleOrder(order_);
    portfolio_handler_->HandleOrder(order_);
  }

 private:
  AbstractStrategy* strategy_;
  AbstractPortfolioHandler* portfolio_handler_;
  std::shared_ptr<OrderField> order_;
};

class CloseMarketEvent : public AbstractEvent {
 public:
  CloseMarketEvent(AbstractPortfolioHandler* portfolio_handler)
      : portfolio_handler_(portfolio_handler) {}

  virtual void Do() override { portfolio_handler_->HandleCloseMarket(); }

 private:
  AbstractPortfolioHandler* portfolio_handler_;
};

class BacktestingEventFactory : public AbstractEventFactory {
 public:
  BacktestingEventFactory(
      std::list<std::shared_ptr<AbstractEvent>>* event_queue)
      : event_queue_(event_queue) {}

  virtual void EnqueueTickEvent(
      const std::shared_ptr<TickData>& tick) const override {
    event_queue_->push_back(std::make_shared<TickEvent>(
        strategy_, execution_handler_, portfolio_handler_, tick));
  }

  virtual void EnqueueFillEvent(
      const std::shared_ptr<OrderField>& order) const override {
    event_queue_->push_back(
        std::make_shared<FillEvent>(strategy_, portfolio_handler_, order));
  }

  virtual void EnqueueInputOrderEvent(PositionEffect position_effect,
                                      OrderDirection order_direction,
                                      int qty) const override {
    event_queue_->push_back(std::make_shared<InputOrderEvent>(
        execution_handler_, position_effect, order_direction, qty));
  }

  void SetStrategy(AbstractStrategy* strategy) { strategy_ = strategy; }

  void SetExecutionHandler(AbstractExecutionHandler* execution_handler) {
    execution_handler_ = execution_handler;
  }

  void SetPortfolioHandler(AbstractPortfolioHandler* portfolio_handler) {
    portfolio_handler_ = portfolio_handler;
  }

  virtual void EnqueueCloseMarketEvent() override {
    event_queue_->push_back(
        std::make_shared<CloseMarketEvent>(portfolio_handler_));
  }

 private:
  std::list<std::shared_ptr<AbstractEvent>>* event_queue_;
  AbstractStrategy* strategy_;
  AbstractExecutionHandler* execution_handler_;

  AbstractPortfolioHandler* portfolio_handler_;
};

int main(int argc, char* argv[]) {
  using hrc = std::chrono::high_resolution_clock;
  auto beg = hrc::now();
  std::list<std::shared_ptr<AbstractEvent>> event_queue;
  bool running = true;

  double init_cash = 50 * 10000;

  BacktestingEventFactory event_factory(&event_queue);

  MyStrategy strategy("m1705", &event_factory);

  SimulatedExecutionHandler execution_handler(&event_factory);

  PriceHandler price_handler("dc", "m1705", &running, &event_factory);

  BacktestingPortfolioHandler portfolio_handler_(init_cash);

  event_factory.SetStrategy(&strategy);

  event_factory.SetExecutionHandler(&execution_handler);

  event_factory.SetPortfolioHandler(&portfolio_handler_);

  while (running) {
    if (!event_queue.empty()) {
      auto event = event_queue.back();
      event_queue.pop_back();
      event->Do();
    } else {
      price_handler.StreamNext();
    }
  }

  std::cout << "espces:"
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   hrc::now() - beg)
                   .count()
            << "\n";
  return 0;
}
