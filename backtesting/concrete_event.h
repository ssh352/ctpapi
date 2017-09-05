#ifndef BACKTESTING_CONCRETE_EVENT_H
#define BACKTESTING_CONCRETE_EVENT_H
#include "event.h"
#include "strategy.h"
#include "execution_handler.h"
#include "portfolio_handler.h"

class TickEvent : public AbstractEvent {
 public:
  TickEvent(AbstractStrategy* strategy,
            AbstractExecutionHandler* execution_handler,
            AbstractPortfolioHandler* portfolio_handler,
            const std::shared_ptr<TickData>& tick);
  virtual void Do() override;

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
            const std::shared_ptr<OrderField>& order);

  virtual void Do() override;

 private:
  AbstractStrategy* strategy_;
  AbstractPortfolioHandler* portfolio_handler_;
  std::shared_ptr<OrderField> order_;
};

class CloseMarketEvent : public AbstractEvent {
 public:
  CloseMarketEvent(AbstractPortfolioHandler* portfolio_handler,
                   AbstractStrategy* strategy);

  virtual void Do() override;

 private:
  AbstractPortfolioHandler* portfolio_handler_;
  AbstractStrategy* strategy_;
};

class InputOrderSignal : public AbstractEvent {
 public:
  InputOrderSignal(AbstractPortfolioHandler* portfolio_handler,
                   InputOrder input_order);

  virtual void Do() override;

 private:
  AbstractPortfolioHandler* portfolio_handler_;
  InputOrder input_order_;
};

class CancelOrderEvent : public AbstractEvent {
 public:
  CancelOrderEvent(AbstractExecutionHandler* exectuion_handler,
                   std::string order_id);

  // Inherited via AbstractEvent
  virtual void Do() override;

 private:
  AbstractExecutionHandler* exectuion_handler_;
  std::string order_id_;
};

class InputOrderEvent : public AbstractEvent {
 public:
  InputOrderEvent(AbstractExecutionHandler* execution_handler,
                  InputOrder input_order);

  virtual void Do() override;

 private:
  AbstractExecutionHandler* execution_handler_;
  InputOrder input_order_;
};

#endif  // BACKTESTING_CONCRETE_EVENT_H
