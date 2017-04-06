#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>

#include "caf/all.hpp"
#include "geek_quant/ctp_trader.h"
#include "follow_trade_actor.h"
#include "geek_quant/cta_trade_actor.h"

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

int caf_main(caf::actor_system& system, const caf::actor_system_config& cfg) {
  caf::scoped_actor self(system);
  auto actor = system.spawn(foo);
  // auto actor = system.spawn<CtaTradeActor>();
  //   self->request(actor, std::chrono::seconds(10), StartAtom::value)
  //       .receive([=](bool result) { std::cout << "Logon sccuess!"; },
  //                [](caf::error& err) {});

  for (int i = 0; i < 100; ++i) {
    caf::anon_send(actor, i);
  }
  caf::anon_send(actor, "hello, world");

  caf::anon_send(actor, "wtf");
  /*
  auto f = caf::make_function_view(actor);

  bool result = f(CTPLogin::value, "tcp://59.42.241.91:41205", "9080",
                  "38030022", "140616")
                    ->get_as<bool>(0);

  std::vector<OrderPosition> positions =
      f(CTPQryInvestorPositionsAtom::value)
          ->get_as<std::vector<OrderPosition>>(0);

  std::vector<OrderRtnData> orders =
      f(CTPReqRestartRtnOrdersAtom::value,
        caf::actor_cast<caf::strong_actor_ptr>(actor))
          ->get_as<std::vector<OrderRtnData>>(0);
  
  */

  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      break;
    }
  }
  return 0;
}

CAF_MAIN()
