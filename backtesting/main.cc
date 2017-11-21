#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <fstream>
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/any.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "caf/all.hpp"
#include "atom_defines.h"
#include "common/api_struct.h"
#include "hpt_core/backtesting/price_handler.h"
#include "hpt_core/backtesting/backtesting_mail_box.h"
#include "hpt_core/tick_series_data_base.h"
#include "hpt_core/cta_transaction_series_data_base.h"
#include "strategies/strategy.h"
#include "hpt_core/time_series_reader.h"
#include "hpt_core/backtesting/execution_handler.h"
#include "hpt_core/portfolio_handler.h"
#include "follow_strategy/delayed_open_strategy_ex.h"
#include "backtesting/backtesting_cta_signal_broker.h"
#include "hpt_core/backtesting/simulated_always_treade_execution_handler.h"
#include "backtesting_cta_signal_broker_ex.h"
#include "follow_strategy/cta_traded_strategy.h"
#include "rtn_order_recorder.h"
#include "type_defines.h"
#include "run_strategy.h"
#include "run_benchmark.h"
#include "caf_coordinator.h"

namespace pt = boost::property_tree;

// std::map<std::string, std::string> g_instrument_market_set = {
//    {"a1705", "dc"},  {"a1709", "dc"},  {"a1801", "dc"},  {"al1705", "sc"},
//    {"bu1705", "sc"}, {"bu1706", "sc"}, {"bu1709", "sc"}, {"c1705", "dc"},
//    {"c1709", "dc"},  {"c1801", "dc"},  {"cf705", "zc"},  {"cf709", "zc"},
//    {"cs1705", "dc"}, {"cs1709", "dc"}, {"cs1801", "dc"}, {"cu1702", "sc"},
//    {"cu1703", "sc"}, {"cu1704", "sc"}, {"cu1705", "sc"}, {"cu1706", "sc"},
//    {"cu1707", "sc"}, {"cu1708", "sc"}, {"cu1709", "sc"}, {"fg705", "zc"},
//    {"fg709", "zc"},  {"i1705", "dc"},  {"i1709", "dc"},  {"j1705", "dc"},
//    {"j1709", "dc"},  {"jd1705", "dc"}, {"jd1708", "dc"}, {"jd1709", "dc"},
//    {"jd1801", "dc"}, {"jm1705", "dc"}, {"jm1709", "dc"}, {"l1705", "dc"},
//    {"l1709", "dc"},  {"l1801", "dc"},  {"m1705", "dc"},  {"m1709", "dc"},
//    {"m1801", "dc"},  {"ma705", "zc"},  {"ma709", "zc"},  {"ma801", "zc"},
//    {"ni1705", "sc"}, {"ni1709", "sc"}, {"p1705", "dc"},  {"p1709", "dc"},
//    {"p1801", "dc"},  {"pp1705", "dc"}, {"pp1709", "dc"}, {"pp1801", "dc"},
//    {"rb1705", "sc"}, {"rb1710", "sc"}, {"rb1801", "sc"}, {"rm705", "zc"},
//    {"rm709", "zc"},  {"rm801", "zc"},  {"ru1705", "sc"}, {"ru1709", "sc"},
//    {"ru1801", "sc"}, {"sm709", "zc"},  {"sr705", "zc"},  {"sr709", "zc"},
//    {"ta705", "zc"},  {"ta709", "zc"},  {"v1709", "dc"},  {"y1705", "dc"},
//    {"y1709", "dc"},  {"y1801", "dc"},  {"zc705", "zc"},  {"zn1702", "sc"},
//    {"zn1703", "sc"}, {"zn1704", "sc"}, {"zn1705", "sc"}, {"zn1706", "sc"},
//    {"zn1707", "sc"}, {"zn1708", "sc"}, {"zn1709", "sc"}};

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(config*)

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
        expr::stream << "logs/" << expr::attr<std::string>("instrument")
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
  pt::ptree ins_infos_pt;
  try {
    pt::read_json(cfg.instrument_infos_file, ins_infos_pt);
  } catch (pt::ptree_error& err) {
    std::cout << "Read Confirg File Error:" << err.what() << "\n";
    return 1;
  }

  double margin_rate = ins_infos_pt.get<double>("a1705.MarginRate");

  pt::ptree strategy_config_pt;
  try {
    pt::read_json(cfg.strategy_config_file, strategy_config_pt);
  } catch (pt::ptree_error& err) {
    std::cout << "Read Confirg File Error:" << err.what() << "\n";
    return 1;
  }

  using hrc = std::chrono::high_resolution_clock;
  auto beg = hrc::now();
  auto instruments =
      std::make_shared<std::list<std::pair<std::string, std::string>>>();

  // if (cfg.instrument.empty()) {
  //  std::for_each(
  //      g_instrument_market_set.begin(), g_instrument_market_set.end(),
  //      [&instruments](const auto& item) {
  //        instruments->emplace_back(std::make_pair(item.second, item.first));
  //      });
  //} else {
  //  instruments->emplace_back(std::make_pair(
  //      g_instrument_market_set[cfg.instrument], cfg.instrument));
  //}

  auto coor =
      system.spawn(Coordinator, &ins_infos_pt, &strategy_config_pt, &cfg);
  std::cout << "start\n";
  for (size_t i = 0; i < 16; ++i) {
    if (cfg.run_benchmark) {
      auto actor = system.spawn(RunBenchmark, coor);
      caf::anon_send(coor, IdleAtom::value, actor);
    } else {
      auto actor = system.spawn(RunStrategy, coor,
                                cfg.cancel_limit_order_when_switch_trade_date);
      caf::anon_send(coor, IdleAtom::value, actor);
    }
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
