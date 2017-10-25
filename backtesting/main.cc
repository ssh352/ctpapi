#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <fstream>
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/any.hpp>
#include "caf/all.hpp"
#include "common/api_struct.h"
#include "hpt_core/backtesting/price_handler.h"
#include "hpt_core/backtesting/backtesting_mail_box.h"
#include "hpt_core/tick_series_data_base.h"
#include "hpt_core/cta_transaction_series_data_base.h"
#include "strategies/strategy.h"
using CTASignalAtom = caf::atom_constant<caf::atom("cta")>;
using BeforeTradingAtom = caf::atom_constant<caf::atom("bt")>;
using BeforeCloseMarketAtom = caf::atom_constant<caf::atom("bcm")>;
using CloseMarketNearAtom = caf::atom_constant<caf::atom("cmn")>;
using DaySettleAtom = caf::atom_constant<caf::atom("daysetl")>;
#include "hpt_core/time_series_reader.h"
#include "hpt_core/backtesting/execution_handler.h"
#include "hpt_core/portfolio_handler.h"
#include "follow_strategy/delayed_open_strategy.h"
#include "follow_strategy/cta_traded_strategy.h"
#include "backtesting/backtesting_cta_signal_broker.h"
#include "hpt_core/backtesting/simulated_always_treade_execution_handler.h"

// #include "follow_strategy/cta_traded_strategy.h"

using IdleAtom = caf::atom_constant<caf::atom("idle")>;
using TickContainer = std::vector<std::pair<std::shared_ptr<Tick>, int64_t>>;
using CTASignalContainer =
    std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>;

std::map<std::string, std::string> g_instrument_market_set = {
    {"a1705", "dc"},  {"a1709", "dc"},  {"a1801", "dc"},  {"al1705", "sc"},
    {"bu1705", "sc"}, {"bu1706", "sc"}, {"bu1709", "sc"}, {"c1705", "dc"},
    {"c1709", "dc"},  {"c1801", "dc"},  {"cf705", "zc"},  {"cf709", "zc"},
    {"cs1705", "dc"}, {"cs1709", "dc"}, {"cs1801", "dc"}, {"cu1702", "sc"},
    {"cu1703", "sc"}, {"cu1704", "sc"}, {"cu1705", "sc"}, {"cu1706", "sc"},
    {"cu1707", "sc"}, {"cu1708", "sc"}, {"cu1709", "sc"}, {"fg705", "zc"},
    {"fg709", "zc"},  {"i1705", "dc"},  {"i1709", "dc"},  {"j1705", "dc"},
    {"j1709", "dc"},  {"jd1705", "dc"}, {"jd1708", "dc"}, {"jd1709", "dc"},
    {"jd1801", "dc"}, {"jm1705", "dc"}, {"jm1709", "dc"}, {"l1705", "dc"},
    {"l1709", "dc"},  {"l1801", "dc"},  {"m1705", "dc"},  {"m1709", "dc"},
    {"m1801", "dc"},  {"ma705", "zc"},  {"ma709", "zc"},  {"ma801", "zc"},
    {"ni1705", "sc"}, {"ni1709", "sc"}, {"p1705", "dc"},  {"p1709", "dc"},
    {"p1801", "dc"},  {"pp1705", "dc"}, {"pp1709", "dc"}, {"pp1801", "dc"},
    {"rb1705", "sc"}, {"rb1710", "sc"}, {"rb1801", "sc"}, {"rm705", "zc"},
    {"rm709", "zc"},  {"rm801", "zc"},  {"ru1705", "sc"}, {"ru1709", "sc"},
    {"ru1801", "sc"}, {"sm709", "zc"},  {"sr705", "zc"},  {"sr709", "zc"},
    {"ta705", "zc"},  {"ta709", "zc"},  {"v1709", "dc"},  {"y1705", "dc"},
    {"y1709", "dc"},  {"y1801", "dc"},  {"zc705", "zc"},  {"zn1702", "sc"},
    {"zn1703", "sc"}, {"zn1704", "sc"}, {"zn1705", "sc"}, {"zn1706", "sc"},
    {"zn1707", "sc"}, {"zn1708", "sc"}, {"zn1709", "sc"}};

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(TickContainer)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTASignalContainer)

class config : public caf::actor_system_config {
 public:
  int delay_open_order_after_seconds = 10;
  int force_close_before_close_market = 5;
  double price_offset_rate = 0.0;
  bool enable_logging = false;
  std::string instrument = "";
  std::string ts_db = "";
  std::string cta_transaction_db = "";
  std::string backtesting_from_datetime = "";
  std::string backtesting_to_datetime = "";

  config() {
    opt_group{custom_options_, "global"}
        .add(delay_open_order_after_seconds, "delayed_seconds",
             "set delayed close seconds")
        .add(force_close_before_close_market, "force_close",
             "force close before close market minues. default is 5")
        .add(price_offset_rate, "price_offset_rate", "price offset rate")
        .add(enable_logging, "enable_logging", "enable logging")
        .add(instrument, "instrument",
             "instrument: is no set then backtesting all instrument")
        .add(ts_db, "tick_db", "tick time series db")
        .add(cta_transaction_db, "ts_cta_db", "cta transaction time series db")
        .add(backtesting_from_datetime, "bs_from_datetime",
             "backtesting from datetime %Y-%m-%d %H:%M:%D, default is "
             "\"2016-12-05 09:00:00\"")
        .add(backtesting_to_datetime, "bs_to_datetime",
             "backtesting from datetime %Y-%m-%d %H:%M:%D, default is "
             "\"2017-07-23 15:00:00\"");
  }
};

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(config*)

template <typename MailBox>
class RtnOrderToCSV {
 public:
  RtnOrderToCSV(MailBox* mail_box, const std::string& prefix_)
      : mail_box_(mail_box), orders_csv_(prefix_ + "_orders.csv") {
    mail_box_->Subscribe(&RtnOrderToCSV::HandleOrder, this);
    boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
    facet->format("%Y-%m-%d %H:%M:%S");
    orders_csv_.imbue(std::locale(std::locale::classic(), facet));
  }

  void HandleOrder(const std::shared_ptr<OrderField>& order) {
    boost::posix_time::ptime pt(
        boost::gregorian::date(1970, 1, 1),
        boost::posix_time::milliseconds(order->update_timestamp));
    orders_csv_ << pt << "," << order->order_id << ","
                << (order->position_effect == PositionEffect::kOpen ? "O" : "C")
                << "," << (order->direction == OrderDirection::kBuy ? "B" : "S")
                << "," << static_cast<int>(order->status) << ","
                << order->input_price << "," << order->qty << "\n";
  }

 private:
  MailBox* mail_box_;
  mutable std::ofstream orders_csv_;
};

auto ReadTickTimeSeries(const char* hdf_file,
                        const std::string& market,
                        const std::string& instrument,
                        const std::string& datetime_from,
                        const std::string& datetime_to) {
  hid_t file = H5Fopen(hdf_file, H5F_ACC_RDONLY, H5P_DEFAULT);
  hid_t tick_compound = H5Tcreate(H5T_COMPOUND, sizeof(Tick));
  TimeSeriesReader<Tick> ts_db(file, tick_compound);

  H5Tinsert(tick_compound, "timestamp", HOFFSET(Tick, timestamp),
            H5T_NATIVE_INT64);
  H5Tinsert(tick_compound, "last_price", HOFFSET(Tick, last_price),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound, "qty", HOFFSET(Tick, qty), H5T_NATIVE_INT64);
  H5Tinsert(tick_compound, "bid_price1", HOFFSET(Tick, bid_price1),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound, "bid_qty1", HOFFSET(Tick, bid_qty1),
            H5T_NATIVE_INT64);
  H5Tinsert(tick_compound, "ask_price1", HOFFSET(Tick, ask_price1),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound, "ask_qty1", HOFFSET(Tick, ask_qty1),
            H5T_NATIVE_INT64);

  auto ts_ret =
      ts_db.ReadRange(str(boost::format("/%s/%s") % market % instrument),
                      boost::posix_time::time_from_string(datetime_from),
                      boost::posix_time::time_from_string(datetime_to));
  herr_t ret = H5Fclose(file);
  return std::move(ts_ret);
}

auto ReadCTAOrderSignalTimeSeries(const char* hdf_file,
                                  const std::string& instrument,
                                  const std::string& datetime_from,
                                  const std::string& datetime_to) {
  hid_t file = H5Fopen(hdf_file, H5F_ACC_RDONLY, H5P_DEFAULT);
  hid_t tick_compound = H5Tcreate(H5T_COMPOUND, sizeof(CTATransaction));

  H5Tinsert(tick_compound, "timestamp", HOFFSET(CTATransaction, timestamp),
            H5T_NATIVE_INT64);
  hid_t order_no_str = H5Tcopy(H5T_C_S1);
  int len = sizeof(OrderIDType) / sizeof(char);
  H5Tset_size(order_no_str, len);

  H5Tinsert(tick_compound, "order_no", HOFFSET(CTATransaction, order_id),
            order_no_str);

  H5Tinsert(tick_compound, "position_effect",
            HOFFSET(CTATransaction, position_effect), H5T_NATIVE_INT32);
  H5Tinsert(tick_compound, "direction", HOFFSET(CTATransaction, direction),
            H5T_NATIVE_INT32);
  H5Tinsert(tick_compound, "status", HOFFSET(CTATransaction, status),
            H5T_NATIVE_INT32);
  H5Tinsert(tick_compound, "price", HOFFSET(CTATransaction, price),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound, "qty", HOFFSET(CTATransaction, qty),
            H5T_NATIVE_INT32);

  H5Tinsert(tick_compound, "traded_qty", HOFFSET(CTATransaction, traded_qty),
            H5T_NATIVE_INT32);

  TimeSeriesReader<CTATransaction> ts_db(file, tick_compound);
  auto ts_ret =
      ts_db.ReadRange(str(boost::format("/%s") % instrument),
                      boost::posix_time::time_from_string(datetime_from),
                      boost::posix_time::time_from_string(datetime_to));
  herr_t ret = H5Fclose(file);
  return std::move(ts_ret);
}

caf::behavior coordinator(
    caf::event_based_actor* self,
    const std::shared_ptr<std::list<std::pair<std::string, std::string>>>&
        instruments,
    const config* cfg) {
  std::string ts_tick_path = cfg->ts_db;
  std::string ts_cta_signal_path = cfg->cta_transaction_db;
  std::string datetime_from = cfg->backtesting_from_datetime;
  std::string datetime_to = cfg->backtesting_to_datetime;
  int delay_open_order_after_seconds = cfg->delay_open_order_after_seconds;
  int force_close_before_close_market = cfg->force_close_before_close_market;

  return {[=](IdleAtom, caf::actor work) {
    // std::string ts_tick_path = "d:/ts_futures.h5";
    // std::string ts_cta_signal_path =
    //    "d:/WorkSpace/backtesing_cta/cta_tstable.h5";

    auto instrument_with_market = instruments->front();
    instruments->pop_front();
    std::string market = instrument_with_market.first;
    std::string instrument = instrument_with_market.second;
    self->send(
        work, market, instrument, delay_open_order_after_seconds,
        force_close_before_close_market,
        ReadTickTimeSeries(ts_tick_path.c_str(), market, instrument,
                           datetime_from, datetime_to),
        ReadCTAOrderSignalTimeSeries(ts_cta_signal_path.c_str(), instrument,
                                     datetime_from, datetime_to));

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
    std::list<std::function<void(void)>> callable_queue;
    bool running = true;
    double init_cash = 50 * 10000;

    // std::string market = "dc";
    // std::string instrument = "a1709";
    std::string csv_file_prefix = str(boost::format("%s") % instrument);
    BacktestingMailBox mail_box(&callable_queue);

    RtnOrderToCSV<BacktestingMailBox> order_to_csv(&mail_box, csv_file_prefix);

    // FollowStrategy<BacktestingMailBox> strategy(&mail_box, "cta", "follower",
    //                                           10 * 60);

    BacktestingCTASignalBroker<BacktestingMailBox>
        backtesting_cta_signal_broker_(&mail_box, cta_signal_container,
                                       instrument);

    ///****
    DelayedOpenStrategy<BacktestingMailBox>::StrategyParam strategy_param;
    strategy_param.delayed_open_after_seconds = 60;
    strategy_param.price_offset_rate = 0.002;
    DelayedOpenStrategy<BacktestingMailBox> strategy(
        &mail_box, "cta", "follower", std::move(strategy_param), instrument);

    SimulatedExecutionHandler<BacktestingMailBox> execution_handler(&mail_box);

    PortfolioHandler<BacktestingMailBox> portfolio_handler(
        init_cash, &mail_box, std::move(instrument), csv_file_prefix, 0.1, 10,
        CostBasis{CommissionType::kFixed, 165, 165, 165}, false);
    ///***

    ///**************************
    // CTATradedStrategy<BacktestingMailBox> strategy(&mail_box);
    // SimulatedAlwaysExcutionHandler<BacktestingMailBox> execution_handler(
    //    &mail_box);

    // PortfolioHandler<BacktestingMailBox> portfolio_handler_(
    //    init_cash, &mail_box, std::move(instrument), csv_file_prefix, 0.1, 10,
    //    CostBasis{CommissionType::kFixed, 165, 165, 165}, true);
    ///*********************

    PriceHandler<BacktestingMailBox> price_handler(
        instrument, &running, &mail_box, std::move(tick_container), 5 * 60,
        60 * 10);

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

void InitLogging(bool enable_logging) {
  namespace logging = boost::log;
  namespace attrs = boost::log::attributes;
  namespace src = boost::log::sources;
  namespace sinks = boost::log::sinks;
  namespace expr = boost::log::expressions;
  namespace keywords = boost::log::keywords;
  boost::shared_ptr<logging::core> core = logging::core::get();

  {
      // boost::shared_ptr<sinks::text_multifile_backend> backend =
      //    boost::make_shared<sinks::text_multifile_backend>();
      //// Set up the file naming pattern
      // backend->set_file_name_composer(sinks::file::as_file_name_composer(
      //    expr::stream << "logs/"
      //                 << "order_" << expr::attr<std::string>("strategy_id")
      //                 << ".log"));

      //// Wrap it into the frontend and register in the core.
      //// The backend requires synchronization in the frontend.
      // typedef sinks::asynchronous_sink<sinks::text_multifile_backend> sink_t;
      // boost::shared_ptr<sink_t> sink(new sink_t(backend));
      // sink->set_formatter(expr::stream
      //                    << expr::attr<boost::posix_time::ptime>("TimeStamp")
      //                    << "[" << expr::attr<std::string>("strategy_id")
      //                    << "]: " << expr::smessage);

      // core->add_sink(sink);
  } {
    boost::shared_ptr<sinks::text_multifile_backend> backend =
        boost::make_shared<sinks::text_multifile_backend>();
    // Set up the file naming pattern
    backend->set_file_name_composer(sinks::file::as_file_name_composer(
        expr::stream << "logs/" << expr::attr<std::string>("account_id")
                     << ".log"));

    // Wrap it into the frontend and register in the core.
    // The backend requires synchronization in the frontend.
    typedef sinks::asynchronous_sink<sinks::text_multifile_backend> sink_t;
    boost::shared_ptr<sink_t> sink(new sink_t(backend));
    sink->set_formatter(
        expr::format("[%1%] %2%") %
        expr::attr<boost::posix_time::ptime>("quant_timestamp") %
        expr::smessage);
    core->add_sink(sink);
  }

  boost::log::add_common_attributes();
  core->set_logging_enabled(enable_logging);
}

int caf_main(caf::actor_system& system, const config& cfg) {
  InitLogging(cfg.enable_logging);
  using hrc = std::chrono::high_resolution_clock;
  auto beg = hrc::now();
  auto instruments =
      std::make_shared<std::list<std::pair<std::string, std::string>>>();

  if (cfg.instrument.empty()) {
    std::for_each(
        g_instrument_market_set.begin(), g_instrument_market_set.end(),
        [&instruments](const auto& item) {
          instruments->emplace_back(std::make_pair(item.second, item.first));
        });
  } else {
    instruments->emplace_back(std::make_pair(
        g_instrument_market_set[cfg.instrument], cfg.instrument));
  }

  auto coor = system.spawn(coordinator, instruments, &cfg);
  std::cout << "start\n";
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
