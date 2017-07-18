#include <boost/archive/binary_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <iostream>

#include <windows.h>

#include "caf/all.hpp"
#include "caf_ctp_util.h"
#include "follow_stragety_service_actor.h"
#include "follow_strategy_mode/print_portfolio_helper.h"
#include "follow_trade_server/binary_serialization.h"
#include "follow_trade_server/caf_defines.h"
#include "follow_trade_server/cta_trade_actor.h"
#include "follow_trade_server/ctp_trader.h"
#include "follow_trade_server/util.h"

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
          [=](std::vector<OrderData> orders) {
            *self->state.oa << orders.size();
            std::for_each(orders.begin(), orders.end(),
                          [&](auto order) { *self->state.oa << order; });
          },
          [=](OrderData order) { *self->state.oa << order; },
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

int caf_main(caf::actor_system& system, const caf::actor_system_config& cfg) {
  pt::ptree pt;
  try {
    pt::read_json(GetExecuableFileDirectoryPath() + "\\config.json", pt);
  } catch (pt::ptree_error& err) {
    std::cout << "Read Confirg File Error:" << err.what() << "\n";
    return 1;
  }

  ClearUpCTPFolwDirectory();

  // LogonInfo master_logon_info{"tcp://59.42.241.91:41205", "9080", "38030022",
  //                            "140616"};
  LogonInfo master_logon_info{pt.get<std::string>("master.front_server"),
                              pt.get<std::string>("master.broker_id"),
                              pt.get<std::string>("master.user_id"),
                              pt.get<std::string>("master.password")};

  auto cta_actor = system.spawn<CtpTrader>(
      master_logon_info.front_server, master_logon_info.broker_id,
      master_logon_info.user_id, master_logon_info.password,
      system.spawn(RtnOrderBinaryToFile, master_logon_info.user_id));
  // auto cta_actor = system.spawn<CtpTrader>("tcp://59.42.241.91:41205",
  // "9080", "38030022", "140616");
  // auto actor = system.spawn(foo);
  // auto actor = system.spawn<CtaTradeActor>();
  //   self->request(actor, std::chrono::seconds(10), StartAtom::value)
  //       .receive([=](bool result) { std::cout << "Logon sccuess!"; },
  //                [](caf::error& err) {});

  if (!Logon(cta_actor)) {
    return 1;
  }

  auto cta_init_positions = BlockRequestInitPositions(cta_actor);
  auto cta_history_rnt_orders = BlockRequestHistoryOrder(cta_actor);

  std::vector<LogonInfo> followers;
  for (auto slave : pt.get_child("slaves")) {
    if (slave.second.get<bool>("enable")) {
      followers.push_back({slave.second.get<std::string>("front_server"),
                           slave.second.get<std::string>("broker_id"),
                           slave.second.get<std::string>("user_id"),
                           slave.second.get<std::string>("password")});
    }
  }
  std::vector<caf::actor> servcies;
  for (auto follower : followers) {
    servcies.push_back(system.spawn<FollowStragetyServiceActor>(
        master_logon_info.user_id, follower.user_id, cta_init_positions,
        cta_history_rnt_orders, cta_actor,
        system.spawn<CtpTrader>(
            follower.front_server, follower.broker_id, follower.user_id,
            follower.password,
            system.spawn(RtnOrderBinaryToFile, follower.user_id)),
        system.spawn(FolloweMonitor, follower.user_id)));
  }

  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      caf::anon_send_exit(cta_actor, caf::exit_reason::user_shutdown);
      for (auto actor : servcies) {
        caf::anon_send_exit(actor, caf::exit_reason::user_shutdown);
      }
      break;
    }
  }
  return 0;
}

CAF_MAIN()
