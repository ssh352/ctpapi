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
    // The order is import!
    execution_handler_->HandleTick(tick_);
    portfolio_handler_->HandleTick(tick_);
    strategy_->HandleTick(tick_);
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
  CloseMarketEvent(AbstractPortfolioHandler* portfolio_handler,
                   AbstractStrategy* strategy)
      : portfolio_handler_(portfolio_handler), strategy_(strategy) {}

  virtual void Do() override {
    portfolio_handler_->HandleCloseMarket();
    strategy_->HandleCloseMarket();
  }

 private:
  AbstractPortfolioHandler* portfolio_handler_;
  AbstractStrategy* strategy_;
};

class InputOrderSignal : public AbstractEvent {
 public:
  InputOrderSignal(AbstractPortfolioHandler* portfolio_handler,
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

  virtual void Do() override {
    portfolio_handler_->HandlerInputOrder(instrument_, position_effect_,
                                          order_direction_, price_, qty_,
                                          timestamp_);
  }

 private:
  AbstractPortfolioHandler* portfolio_handler_;
  std::string instrument_;
  PositionEffect position_effect_;
  OrderDirection order_direction_;
  double price_;
  int qty_;
  TimeStamp timestamp_;
};

class CancelOrderEvent : public AbstractEvent {
 public:
  CancelOrderEvent(AbstractExecutionHandler* exectuion_handler,
                   std::string order_id)
      : exectuion_handler_(exectuion_handler), order_id_(std::move(order_id)) {}

  // Inherited via AbstractEvent
  virtual void Do() override {
    exectuion_handler_->HandleCancelOrder(order_id_);
  }

 private:
  AbstractExecutionHandler* exectuion_handler_;
  std::string order_id_;
};

class BacktestingEventFactory : public AbstractEventFactory {
 public:
  BacktestingEventFactory(
      std::list<std::shared_ptr<AbstractEvent>>* event_queue,
      const std::string& prefix_)
      : event_queue_(event_queue), orders_csv_(prefix_ + "_orders.csv") {}

  virtual void EnqueueTickEvent(
      const std::shared_ptr<TickData>& tick) const override {
    event_queue_->push_back(std::make_shared<TickEvent>(
        strategy_, execution_handler_, portfolio_handler_, tick));
  }

  virtual void EnqueueRtnOrderEvent(
      const std::shared_ptr<OrderField>& order) const override {
    event_queue_->push_back(
        std::make_shared<FillEvent>(strategy_, portfolio_handler_, order));
    orders_csv_ << order->update_timestamp << ","
                << (order->position_effect == PositionEffect::kOpen ? "O" : "C")
                << "," << (order->direction == OrderDirection::kBuy ? "B" : "S")
                << "," << static_cast<int>(order->status) << "," << order->price
                << "," << order->qty << "\n";
  }

  virtual void EnqueueInputOrderEvent(const std::string& instrument,
                                      PositionEffect position_effect,
                                      OrderDirection order_direction,
                                      double price,
                                      int qty,
                                      TimeStamp timestamp) const override {
    event_queue_->push_back(std::make_shared<InputOrderEvent>(
        execution_handler_, instrument, position_effect, order_direction, price,
        qty, timestamp));
  }

  virtual void EnqueueInputOrderSignal(const std::string& instrument,
                                       PositionEffect position_effect,
                                       OrderDirection order_direction,
                                       double price,
                                       int qty,
                                       TimeStamp timestamp) const override {
    event_queue_->push_back(std::make_shared<InputOrderSignal>(
        portfolio_handler_, instrument, position_effect, order_direction, price,
        qty, timestamp));
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
        std::make_shared<CloseMarketEvent>(portfolio_handler_, strategy_));
  }

  // Inherited via AbstractEventFactory
  virtual void EnqueueCancelOrderEvent(const std::string& order_id) override {
    event_queue_->push_back(
        std::make_shared<CancelOrderEvent>(execution_handler_, order_id));
  }

 private:
  std::list<std::shared_ptr<AbstractEvent>>* event_queue_;
  AbstractStrategy* strategy_;
  AbstractExecutionHandler* execution_handler_;
  AbstractPortfolioHandler* portfolio_handler_;
  mutable std::ofstream orders_csv_;
};

int main(int argc, char* argv[]) {
  using hrc = std::chrono::high_resolution_clock;
  auto beg = hrc::now();
  std::list<std::shared_ptr<AbstractEvent>> event_queue;
  bool running = true;

  double init_cash = 50 * 10000;

  std::string market = "dc";

  std::string instrument = "a1709";

  std::string ts_tick_path = "d:/ts_futures.h5";

  std::string datetime_from = "2016-12-05 09:00:00";

  std::string datetime_to = "2017-07-31 15:00:00";

  std::string ts_cta_signal_path = "d:/cta_tstable.h5";

  BacktestingEventFactory event_factory(&event_queue, instrument);

  CTATransactionSeriesDataBase cta_trasaction_series_data_base(
      ts_cta_signal_path.c_str());

  MyStrategy strategy(&event_factory,
                      cta_trasaction_series_data_base.ReadRange(
                          str(boost::format("/%s") % instrument),
                          boost::posix_time::time_from_string(datetime_from),
                          boost::posix_time::time_from_string(datetime_to)),
                      1, 10);

  SimulatedExecutionHandler execution_handler(&event_factory);

  TickSeriesDataBase ts_db(ts_tick_path.c_str());

  PriceHandler price_handler(
      instrument, &running, &event_factory,
      ts_db.ReadRange(str(boost::format("/%s/%s") % market % instrument),
                      boost::posix_time::time_from_string(datetime_from),
                      boost::posix_time::time_from_string(datetime_to)));

  BacktestingPortfolioHandler portfolio_handler_(
      init_cash, &event_factory, std::move(instrument), 0.1, 10,
      CostBasis{CommissionType::kFixed, 165, 165, 165});

  event_factory.SetStrategy(&strategy);

  event_factory.SetExecutionHandler(&execution_handler);

  event_factory.SetPortfolioHandler(&portfolio_handler_);

  while (running) {
    if (!event_queue.empty()) {
      auto event = event_queue.front();
      event_queue.pop_front();
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
