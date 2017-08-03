#include <boost/archive/binary_oarchive.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/support/exception.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

#include <iostream>

#include "caf/all.hpp"
#include "caf_ctp_util.h"
#include "follow_stragety_service_actor.h"
#include "follow_strategy_mode/print_portfolio_helper.h"
#include "follow_trade_server/atom_defines.h"
#include "follow_trade_server/binary_serialization.h"
// #include "follow_trade_server/cta_trade_actor.h"
#include "atom_defines.h"
#include "cta_signal_trader.h"

#include "db_store.h"
#include "follow_trade_server/ctp_trader.h"
#include "follow_trade_server/util.h"
#include "strategy_trader.h"
#include "websocket_typedef.h"

/*
behavior StrategyListener(event_based_actor* self,
                          const CtpObserver& observer_actor) {
  self->request(observer_actor, caf::infinite, AddListenerAtom::value,
                actor_cast<strong_actor_ptr>(self));
  return {
      [=](CtpRtnOrderAtom) { caf::aout(self) << "Listener invoker\n"; },
  };
}

using CtpObserver =
    caf::typed_actor<caf::reacts_to<CtpLoginAtom>,
                     caf::reacts_to<CtpRtnOrderAtom, CThostFtdcOrderField>,
                     caf::reacts_to<AddListenerAtom, caf::strong_actor_ptr> >;

CtpObserver::behavior_type DummyObserver(CtpObserver::pointer self) {
  return {
      [](CtpLoginAtom) {}, [](CtpRtnOrderAtom, CThostFtdcOrderField) {},
      [](AddListenerAtom, caf::strong_actor_ptr) {},
  };
}

*/

/*

caf::behavior wtf(caf::event_based_actor* self) {
  return {[=](std::string str) -> std::string {
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    return str;
  }};
}

caf::behavior bar(caf::event_based_actor* self) {
  return {[=](std::string str) -> std::string { return str; }};
}

caf::behavior foo(caf::event_based_actor* self) {
  self->set_default_handler(caf::skip);
  return {[=](int i) {
            std::cout << i << "\n";
            return caf::skip();
          },
          [=](std::string str) {
            std::cout << str << "\n";
            self->become(caf::keep_behavior,
                         [=](std::string str) {
                           std::cout << str << "\n";
                           self->unbecome();
                         },
                         [=](const caf::error& err) { std::cout << "err\n"; });
          }};
}

*/
struct LogBinaryArchive {
  std::ofstream file;
  std::unique_ptr<boost::archive::binary_oarchive> oa;
};

caf::behavior FolloweMonitor(caf::stateful_actor<LogBinaryArchive>* self,
                             std::string slave_account_id) {
  self->state.file.open(MakeDataBinaryFileName(slave_account_id, "flow_bin"),
                        std::ios_base::binary);
  self->state.oa =
      std::make_unique<boost::archive::binary_oarchive>(self->state.file);
  return {[=](std::string account_id, std::vector<OrderPosition> positions) {
            *self->state.oa << account_id;
            *self->state.oa << positions.size();
            std::for_each(positions.begin(), positions.end(),
                          [&](auto pos) { *self->state.oa << pos; });
          },
          [=](std::vector<OrderField> orders) {
            *self->state.oa << orders.size();
            std::for_each(orders.begin(), orders.end(),
                          [&](auto order) { *self->state.oa << order; });
          },
          [=](OrderField order) { *self->state.oa << order; },
          [=](std::string account_id,
              std::vector<AccountPortfolio> master_portfolio,
              std::vector<AccountPortfolio> slave_portfolio, bool fully) {
            caf::aout(self) << FormatPortfolio(account_id, master_portfolio,
                                               slave_portfolio, fully);
          }};
}

caf::behavior RtnOrderBinaryToFile(caf::stateful_actor<LogBinaryArchive>* self,
                                   std::string account_id) {
  self->state.file.open(MakeDataBinaryFileName(account_id, "rtn_order"),
                        std::ios_base::binary);
  self->state.oa =
      std::make_unique<boost::archive::binary_oarchive>(self->state.file);
  return {[=](CThostFtdcOrderField order) { *self->state.oa << order; }};
}

struct LogonInfo {
  std::string front_server;
  std::string broker_id;
  std::string user_id;
  std::string password;
};

namespace pt = boost::property_tree;

void on_open(std::vector<caf::actor> actors, websocketpp::connection_hdl hdl) {
  for (auto actor : actors) {
    caf::anon_send(actor, StragetyPortfilioAtom::value, hdl);
  }
}

void InitLogging() {
  namespace logging = boost::log;
  namespace attrs = boost::log::attributes;
  namespace src = boost::log::sources;
  namespace sinks = boost::log::sinks;
  namespace expr = boost::log::expressions;
  namespace keywords = boost::log::keywords;
  boost::shared_ptr<logging::core> core = logging::core::get();

  {
    boost::shared_ptr<sinks::text_multifile_backend> backend =
        boost::make_shared<sinks::text_multifile_backend>();
    // Set up the file naming pattern
    backend->set_file_name_composer(sinks::file::as_file_name_composer(
        expr::stream << "logs/"
                     << "order_" << expr::attr<std::string>("strategy_id")
                     << ".log"));

    // Wrap it into the frontend and register in the core.
    // The backend requires synchronization in the frontend.
    typedef sinks::asynchronous_sink<sinks::text_multifile_backend> sink_t;
    boost::shared_ptr<sink_t> sink(new sink_t(backend));
    sink->set_formatter(expr::stream
                        << expr::attr<boost::posix_time::ptime>("TimeStamp")
                        << "[" << expr::attr<std::string>("strategy_id")
                        << "]: " << expr::smessage);

    core->add_sink(sink);
  }
  {
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
    sink->set_formatter(expr::format("[%1%] %2%") %
                        expr::attr<boost::posix_time::ptime>("TimeStamp") %
                        expr::smessage);
    core->add_sink(sink);
  }

  boost::log::add_common_attributes();
}

int main2(caf::actor_system& system, const caf::actor_system_config& cfg) {
  auto sqlite = system.spawn<DBStore>();
  system.registry().put(caf::atom("db"),
                        caf::actor_cast<caf::strong_actor_ptr>(sqlite));

  pt::ptree pt;
  try {
    pt::read_json(GetExecuableFileDirectoryPath() + "\\config.json", pt);
  } catch (pt::ptree_error& err) {
    std::cout << "Read Confirg File Error:" << err.what() << "\n";
    return 1;
  }

  boost::asio::io_service io_service;
  // ClearUpCTPFolwDirectory();
  std::vector<LogonInfo> followers;
  for (auto slave : pt.get_child("slaves")) {
    if (slave.second.get<bool>("enable")) {
      followers.push_back({slave.second.get<std::string>("front_server"),
                           slave.second.get<std::string>("broker_id"),
                           slave.second.get<std::string>("user_id"),
                           slave.second.get<std::string>("password")});
    }
  }
  // LogonInfo master_logon_info{"tcp://59.42.241.91:41205", "9080", "38030022",
  //                            "140616"};

  LogonInfo master_logon_info{pt.get<std::string>("master.front_server"),
                              pt.get<std::string>("master.broker_id"),
                              pt.get<std::string>("master.user_id"),
                              pt.get<std::string>("master.password")};

  caf::group grp = system.groups().anonymous();
  caf::scoped_actor self(system);
  self->join(grp);

  auto trader = system.spawn<StrategyTrader>(
      grp, master_logon_info.front_server, master_logon_info.broker_id,
      master_logon_info.user_id, master_logon_info.password);

  bool running = true;
  self->receive_while(running)(
      [sqlite](const boost::shared_ptr<OrderField>& order) {
        caf::anon_send(sqlite, InsertStrategyRtnOrder::value, order);
      },
      caf::after(std::chrono::seconds(2)) >> [&running] { running = false; });
  //   caf::anon_send_exit(sqlite, caf::exit_reason::user_shutdown);
  //   caf::anon_send_exit(trader, caf::exit_reason::user_shutdown);
  self->leave(grp);
  system.await_all_actors_done();
  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      break;
    }
  }
  return 0;
}

int caf_main(caf::actor_system& system, const caf::actor_system_config& cfg) {
  InitLogging();

  auto sqlite = system.spawn<DBStore>();
  system.registry().put(caf::atom("db"),
                        caf::actor_cast<caf::strong_actor_ptr>(sqlite));

  pt::ptree pt;
  try {
    pt::read_json(GetExecuableFileDirectoryPath() + "\\config.json", pt);
  } catch (pt::ptree_error& err) {
    std::cout << "Read Confirg File Error:" << err.what() << "\n";
    return 1;
  }

  // ClearUpCTPFolwDirectory();
  // LogonInfo master_logon_info{"tcp://59.42.241.91:41205", "9080", "38030022",
  //                            "140616"};

  LogonInfo master_logon_info{pt.get<std::string>("master.front_server"),
                              pt.get<std::string>("master.broker_id"),
                              pt.get<std::string>("master.user_id"),
                              pt.get<std::string>("master.password")};

  auto cta = system.spawn<CTASignalTrader>(
      master_logon_info.front_server, master_logon_info.broker_id,
      master_logon_info.user_id, master_logon_info.password);

  std::vector<LogonInfo> followers;
  for (auto slave : pt.get_child("slaves")) {
    if (slave.second.get<bool>("enable")) {
      followers.push_back({slave.second.get<std::string>("front_server"),
                           slave.second.get<std::string>("broker_id"),
                           slave.second.get<std::string>("user_id"),
                           slave.second.get<std::string>("password")});
    }
  }

  auto grp = system.groups().anonymous();
  std::vector<caf::actor> strategy_actors;
  for (auto follower : followers) {
    auto trader = system.spawn<StrategyTrader>(
        grp, follower.front_server, follower.broker_id, follower.user_id,
        follower.password);

    strategy_actors.push_back(system.spawn<FollowStragetyServiceActor>(
        trader, cta, master_logon_info.user_id));
  }

  // caf::anon_send_exit(trader, caf::exit_reason::user_shutdown);
  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      break;
    }
  }

  system.registry().erase(caf::atom("db"));
  return 0;
}

CAF_MAIN()
