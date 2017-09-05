#include "concrete_event.h"

TickEvent::TickEvent(AbstractStrategy* strategy,
                     AbstractExecutionHandler* execution_handler,
                     AbstractPortfolioHandler* portfolio_handler,
                     const std::shared_ptr<TickData>& tick)
    : strategy_(strategy),
      execution_handler_(execution_handler),
      portfolio_handler_(portfolio_handler),
      tick_(tick) {}

void TickEvent::Do() {
  // The order is import!
  execution_handler_->HandleTick(tick_);
  portfolio_handler_->HandleTick(tick_);
  strategy_->HandleTick(tick_);
}

FillEvent::FillEvent(AbstractStrategy* strategy,
                     AbstractPortfolioHandler* portfolio_handler,
                     const std::shared_ptr<OrderField>& order)
    : strategy_(strategy),
      portfolio_handler_(portfolio_handler),
      order_(order) {}

void FillEvent::Do() {
  strategy_->HandleOrder(order_);
  portfolio_handler_->HandleOrder(order_);
}

CloseMarketEvent::CloseMarketEvent(AbstractPortfolioHandler* portfolio_handler,
                                   AbstractStrategy* strategy)
    : portfolio_handler_(portfolio_handler), strategy_(strategy) {}

void CloseMarketEvent::Do() {
  portfolio_handler_->HandleCloseMarket();
  strategy_->HandleCloseMarket();
}

InputOrderSignal::InputOrderSignal(AbstractPortfolioHandler* portfolio_handler,
                                   std::string instrument,
                                   PositionEffect position_effect,
                                   OrderDirection order_direction,
                                   double price,
                                   int qty,
                                   TimeStamp timestamp)
    : portfolio_handler_(portfolio_handler),
      instrument_(std::move(instrument)),
      position_effect_(position_effect),
      order_direction_(order_direction),
      price_(price),
      qty_(qty),
      timestamp_(timestamp) {}

void InputOrderSignal::Do() {
  portfolio_handler_->HandlerInputOrder(instrument_, position_effect_,
                                        order_direction_, price_, qty_,
                                        timestamp_);
}

CancelOrderEvent::CancelOrderEvent(AbstractExecutionHandler* exectuion_handler,
                                   std::string order_id)
    : exectuion_handler_(exectuion_handler), order_id_(std::move(order_id)) {}

void CancelOrderEvent::Do() {
  exectuion_handler_->HandleCancelOrder(order_id_);
}

InputOrderEvent::InputOrderEvent(AbstractExecutionHandler* execution_handler,
                                 std::string instrument,
                                 PositionEffect position_effect,
                                 OrderDirection order_direction,
                                 double price,
                                 int qty,
                                 TimeStamp timestamp)
    : execution_handler_(execution_handler),
      instrument_(std::move(instrument)),
      position_effect_(position_effect),
      order_direction_(order_direction),
      price_(price),
      qty_(qty),
      timestamp_(timestamp) {}

void InputOrderEvent::Do() {
  execution_handler_->HandlerInputOrder(instrument_, position_effect_,
                                        order_direction_, price_, qty_,
                                        timestamp_);
}
