#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <fstream>
#include <boost/format.hpp>
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
    orders_csv_ << order->update_timestamp << "," << order->order_id << ","
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

using IdleAtom = caf::atom_constant<caf::atom("idle")>;

using TickContainer = std::vector<std::pair<std::shared_ptr<Tick>, int64_t>>;
using CTASignalContainer =
    std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>;

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(TickContainer)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTASignalContainer)

caf::behavior coordinator(
    caf::event_based_actor* self,
    const std::shared_ptr<std::list<std::pair<std::string, std::string>>>
        instruments,
    int delayed_input_order_by_minute,
    int cancel_order_after_minute) {
  return {[=](IdleAtom, caf::actor work) {
    std::string ts_tick_path = "d:/ts_futures.h5";
    std::string ts_cta_signal_path = "d:/cta_tstable.h5";
    std::string datetime_from = "2016-12-05 09:00:00";
    std::string datetime_to = "2017-07-31 15:00:00";
    TickSeriesDataBase ts_db(ts_tick_path.c_str());
    CTATransactionSeriesDataBase cta_trasaction_series_data_base(
        ts_cta_signal_path.c_str());
    auto instrument_with_market = instruments->front();
    instruments->pop_front();
    std::string market = instrument_with_market.first;
    std::string instrument = instrument_with_market.second;
    self->send(
        work, market, instrument, delayed_input_order_by_minute,
        cancel_order_after_minute,
        ts_db.ReadRange(str(boost::format("/%s/%s") % market % instrument),
                        boost::posix_time::time_from_string(datetime_from),
                        boost::posix_time::time_from_string(datetime_to)),
        cta_trasaction_series_data_base.ReadRange(
            str(boost::format("/%s") % instrument),
            boost::posix_time::time_from_string(datetime_from),
            boost::posix_time::time_from_string(datetime_to)));
    if (instruments->empty()) {
      self->quit();
    }
  }};
}

caf::behavior worker(caf::event_based_actor* self, caf::actor coor) {
  return {[=](const std::string& market, const std::string& instrument,
              int delayed_input_order_by_minute, int cancel_order_after_minute,
              TickContainer tick_container,
              CTASignalContainer cta_signal_container) {
    caf::aout(self) << market << ":" << instrument << "\n";
    std::list<std::shared_ptr<AbstractEvent>> event_queue;
    bool running = true;
    double init_cash = 50 * 10000;

    // std::string market = "dc";
    // std::string instrument = "a1709";

    std::string csv_file_prefix =
        str(boost::format("%s_%d_%d") % instrument %
            delayed_input_order_by_minute % cancel_order_after_minute);
    BacktestingEventFactory event_factory(&event_queue, csv_file_prefix);

    MyStrategy strategy(&event_factory, std::move(cta_signal_container),
                        delayed_input_order_by_minute,
                        cancel_order_after_minute);

    SimulatedExecutionHandler execution_handler(&event_factory);

    PriceHandler price_handler(instrument, &running, &event_factory,
                               std::move(tick_container));

    BacktestingPortfolioHandler portfolio_handler_(
        init_cash, &event_factory, std::move(instrument), csv_file_prefix, 0.1,
        10, CostBasis{CommissionType::kFixed, 165, 165, 165});

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

    // self->send(coor, IdleAtom::value, self);
    self->quit();
  }};
}

class config : public caf::actor_system_config {
 public:
  int delayed_close_minutes = 0;
  int cancel_after_minutes = 0;

  config() {
    opt_group{custom_options_, "global"}
        .add(delayed_close_minutes, "delayed,d", "set delayed close minutes")
        .add(cancel_after_minutes, "cancel,c", "set cancel after minutes");
  }
};

int caf_main(caf::actor_system& system, const config& cfg) {
  using hrc = std::chrono::high_resolution_clock;
  auto beg = hrc::now();
  auto instruments =
      std::make_shared<std::list<std::pair<std::string, std::string>>>();
  instruments->emplace_back(std::make_pair("dc", "a1705"));
  instruments->emplace_back(std::make_pair("dc", "a1709"));
  instruments->emplace_back(std::make_pair("dc", "a1801"));
  instruments->emplace_back(std::make_pair("sc", "al1705"));
  instruments->emplace_back(std::make_pair("sc", "bu1705"));
  instruments->emplace_back(std::make_pair("sc", "bu1706"));
  instruments->emplace_back(std::make_pair("sc", "bu1709"));
  instruments->emplace_back(std::make_pair("dc", "c1705"));
  instruments->emplace_back(std::make_pair("dc", "c1709"));
  instruments->emplace_back(std::make_pair("dc", "c1801"));
  instruments->emplace_back(std::make_pair("zc", "cf705"));
  instruments->emplace_back(std::make_pair("zc", "cf709"));
  instruments->emplace_back(std::make_pair("dc", "cs1705"));
  instruments->emplace_back(std::make_pair("dc", "cs1709"));
  instruments->emplace_back(std::make_pair("dc", "cs1801"));
  instruments->emplace_back(std::make_pair("sc", "cu1702"));
  instruments->emplace_back(std::make_pair("sc", "cu1703"));
  instruments->emplace_back(std::make_pair("sc", "cu1704"));
  instruments->emplace_back(std::make_pair("sc", "cu1705"));
  instruments->emplace_back(std::make_pair("sc", "cu1706"));
  instruments->emplace_back(std::make_pair("sc", "cu1707"));
  instruments->emplace_back(std::make_pair("sc", "cu1708"));
  instruments->emplace_back(std::make_pair("sc", "cu1709"));
  instruments->emplace_back(std::make_pair("zc", "fg705"));
  instruments->emplace_back(std::make_pair("zc", "fg709"));
  instruments->emplace_back(std::make_pair("dc", "i1705"));
  instruments->emplace_back(std::make_pair("dc", "i1709"));
  instruments->emplace_back(std::make_pair("dc", "j1705"));
  instruments->emplace_back(std::make_pair("dc", "j1709"));
  instruments->emplace_back(std::make_pair("dc", "jd1705"));
  instruments->emplace_back(std::make_pair("dc", "jd1708"));
  instruments->emplace_back(std::make_pair("dc", "jd1709"));
  instruments->emplace_back(std::make_pair("dc", "jd1801"));
  instruments->emplace_back(std::make_pair("dc", "jm1705"));
  instruments->emplace_back(std::make_pair("dc", "jm1709"));
  instruments->emplace_back(std::make_pair("dc", "l1705"));
  instruments->emplace_back(std::make_pair("dc", "l1709"));
  instruments->emplace_back(std::make_pair("dc", "l1801"));
  instruments->emplace_back(std::make_pair("dc", "m1705"));
  instruments->emplace_back(std::make_pair("dc", "m1709"));
  instruments->emplace_back(std::make_pair("dc", "m1801"));
  instruments->emplace_back(std::make_pair("zc", "ma705"));
  instruments->emplace_back(std::make_pair("zc", "ma709"));
  instruments->emplace_back(std::make_pair("zc", "ma801"));
  instruments->emplace_back(std::make_pair("sc", "ni1705"));
  instruments->emplace_back(std::make_pair("sc", "ni1709"));
  instruments->emplace_back(std::make_pair("dc", "p1705"));
  instruments->emplace_back(std::make_pair("dc", "p1709"));
  instruments->emplace_back(std::make_pair("dc", "p1801"));
  instruments->emplace_back(std::make_pair("dc", "pp1705"));
  instruments->emplace_back(std::make_pair("dc", "pp1709"));
  instruments->emplace_back(std::make_pair("dc", "pp1801"));
  instruments->emplace_back(std::make_pair("sc", "rb1705"));
  instruments->emplace_back(std::make_pair("sc", "rb1710"));
  instruments->emplace_back(std::make_pair("sc", "rb1801"));
  instruments->emplace_back(std::make_pair("zc", "rm705"));
  instruments->emplace_back(std::make_pair("zc", "rm709"));
  instruments->emplace_back(std::make_pair("zc", "rm801"));
  instruments->emplace_back(std::make_pair("sc", "ru1705"));
  instruments->emplace_back(std::make_pair("sc", "ru1709"));
  instruments->emplace_back(std::make_pair("sc", "ru1801"));
  instruments->emplace_back(std::make_pair("zc", "sm709"));
  instruments->emplace_back(std::make_pair("zc", "sr705"));
  instruments->emplace_back(std::make_pair("zc", "sr709"));
  instruments->emplace_back(std::make_pair("zc", "ta705"));
  instruments->emplace_back(std::make_pair("zc", "ta709"));
  instruments->emplace_back(std::make_pair("dc", "v1709"));
  instruments->emplace_back(std::make_pair("dc", "y1705"));
  instruments->emplace_back(std::make_pair("dc", "y1709"));
  instruments->emplace_back(std::make_pair("dc", "y1801"));
  instruments->emplace_back(std::make_pair("zc", "zc705"));
  instruments->emplace_back(std::make_pair("sc", "zn1702"));
  instruments->emplace_back(std::make_pair("sc", "zn1703"));
  instruments->emplace_back(std::make_pair("sc", "zn1704"));
  instruments->emplace_back(std::make_pair("sc", "zn1705"));
  instruments->emplace_back(std::make_pair("sc", "zn1706"));
  instruments->emplace_back(std::make_pair("sc", "zn1707"));
  instruments->emplace_back(std::make_pair("sc", "zn1708"));
  instruments->emplace_back(std::make_pair("sc", "zn1709"));

  auto coor = system.spawn(coordinator, instruments, cfg.delayed_close_minutes,
                           cfg.cancel_after_minutes);
  for (size_t i = 0; i < instruments->size(); ++i) {
    auto actor = system.spawn(worker, coor);

    caf::anon_send(coor, IdleAtom::value, actor);
  }
  system.await_all_actors_done();

  std::cout << "espces:"
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   hrc::now() - beg)
                   .count()
            << "\n";
  return 0;
}

CAF_MAIN()
