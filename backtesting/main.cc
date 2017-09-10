#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <fstream>
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/any.hpp>
#include "caf/all.hpp"
#include "common/api_struct.h"
#include "hpt_core/backtesting/execution_handler.h"
#include "hpt_core/backtesting/price_handler.h"
#include "hpt_core/backtesting/backtesting_mail_box.h"
#include "hpt_core/tick_series_data_base.h"
#include "hpt_core/portfolio_handler.h"
#include "hpt_core/cta_transaction_series_data_base.h"
#include "strategies/strategy.h"

using CTASignalAtom = caf::atom_constant<caf::atom("cta")>;

#include "strategies/delayed_open_strategy.h"

using IdleAtom = caf::atom_constant<caf::atom("idle")>;
using TickContainer = std::vector<std::pair<std::shared_ptr<Tick>, int64_t>>;
using CTASignalContainer =
    std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>;

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(TickContainer)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTASignalContainer)

template <typename MailBox>
class RtnOrderToCSV {
 public:
  RtnOrderToCSV(MailBox* mail_box, const std::string& prefix_)
      : mail_box_(mail_box), orders_csv_(prefix_ + "_orders.csv") {
    mail_box_->Subscribe(&RtnOrderToCSV::HandleOrder, this);
  }

  void HandleOrder(const std::shared_ptr<OrderField>& order) {
    orders_csv_ << order->update_timestamp << "," << order->order_id << ","
                << (order->position_effect == PositionEffect::kOpen ? "O" : "C")
                << "," << (order->direction == OrderDirection::kBuy ? "B" : "S")
                << "," << static_cast<int>(order->status) << "," << order->price
                << "," << order->qty << "\n";
  }

 private:
  MailBox* mail_box_;
  mutable std::ofstream orders_csv_;
};

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

caf::behavior worker(caf::event_based_actor* self,
                     caf::actor coor,
                     int backtesting_position_effect) {
  return {[=](const std::string& market, const std::string& instrument,
              int delayed_input_order_by_minute, int cancel_order_after_minute,
              TickContainer tick_container,
              CTASignalContainer cta_signal_container) {
    caf::aout(self) << market << ":" << instrument << "\n";
    std::list<std::function<void(void)>> callable_queue;
    bool running = true;
    double init_cash = 50 * 10000;

    // std::string market = "dc";
    // std::string instrument = "a1709";
    std::string csv_file_prefix =
        str(boost::format("%s_%d_%d_%s") % instrument %
            delayed_input_order_by_minute % cancel_order_after_minute %
            (backtesting_position_effect == 0 ? "O" : "C"));
    BacktestingMailBox mail_box(&callable_queue);

    RtnOrderToCSV<BacktestingMailBox> order_to_csv(&mail_box, csv_file_prefix);

    DelayedOpenStrategy<BacktestingMailBox> strategy(&mail_box, 30 * 60);
    // MyStrategy<BacktestingMailBox> strategy(
    //    &mail_box, std::move(cta_signal_container),
    //    delayed_input_order_by_minute, cancel_order_after_minute,
    //    backtesting_position_effect);

    SimulatedExecutionHandler<BacktestingMailBox> execution_handler(&mail_box);

    PriceHandler<BacktestingMailBox> price_handler(
        instrument, &running, &mail_box, std::move(tick_container));

    PortfolioHandler<BacktestingMailBox> portfolio_handler_(
        init_cash, &mail_box, std::move(instrument), csv_file_prefix, 0.1, 10,
        CostBasis{CommissionType::kFixed, 165, 165, 165});

    while (running) {
      if (!callable_queue.empty()) {
        auto callable = callable_queue.front();
        callable();
        callable_queue.pop_front();
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
  int delayed_close_minutes = 10;
  int cancel_after_minutes = 10;
  int position_effect = 0;

  config() {
    opt_group{custom_options_, "global"}
        .add(delayed_close_minutes, "delayed,d", "set delayed close minutes")
        .add(cancel_after_minutes, "cancel,c", "set cancel after minutes")
        .add(position_effect, "open_close", "backtesting open(0) close(1)");
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

  std::cout << "start\n";
  auto coor = system.spawn(coordinator, instruments, cfg.delayed_close_minutes,
                           cfg.cancel_after_minutes);
  for (size_t i = 0; i < instruments->size(); ++i) {
    auto actor = system.spawn(worker, coor, cfg.position_effect);

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
