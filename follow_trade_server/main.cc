#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>

#include "caf/all.hpp"
#include "follow_trade_server/ctp_trader.h"
#include "follow_trade_server/cta_trade_actor.h"
#include "follow_trade_server/caf_defines.h"
#include "follow_strategy_mode/follow_strategy_service.h"
#include "caf_ctp_util.h"
#include "follow_stragety_service_actor.h"

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

struct Follower {
  std::string front_server;
  std::string broker_id;
  std::string user_id;
  std::string password;
};

int caf_main(caf::actor_system& system, const caf::actor_system_config& cfg) {
  auto cta_actor = system.spawn<CtpTrader>("tcp://59.42.241.91:41205", "9080",
                                           "38030022", "140616");
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

  std::vector<Follower> followers{
      {"tcp://180.168.146.187:10000", "9999", "053861", "Cj12345678"},
      {"tcp://180.168.146.187:10000", "9999", "053867", "8661188"}};

  std::vector<caf::actor> servcies;
  for (auto follower : followers) {
    servcies.push_back(system.spawn<FollowStragetyServiceActor>(
        "38030022", follower.user_id, cta_init_positions,
        cta_history_rnt_orders, cta_actor,
        system.spawn<CtpTrader>(follower.front_server, follower.broker_id,
                                follower.user_id, follower.password)));
  }

  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      break;
    }
  }
  return 0;
}

CAF_MAIN()
