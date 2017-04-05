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

caf::behavior foo(caf::event_based_actor* self,
                  const caf::actor& a,
                  const caf::actor& b) {
  caf::scoped_actor block_actor(self->system());
  block_actor->request(a, caf::infinite, "hello")
      .receive([=](std::string str) { std::cout << str; },
               [=](caf::error& err) {});
  self->request(a, caf::infinite, "hello").await([=](std::string str) {
    std::cout << str << "\n";
  });

  self->request(b, caf::infinite, "wtf").await([=](std::string str) {
    std::cout << str << "\n";
  });
  return {[=](int i) {}};
}

int caf_main(caf::actor_system& system, const caf::actor_system_config& cfg) {
  caf::scoped_actor self(system);
  auto actor = system.spawn<CtaTradeActor>();
  //   self->request(actor, std::chrono::seconds(10), StartAtom::value)
  //       .receive([=](bool result) { std::cout << "Logon sccuess!"; },
  //                [](caf::error& err) {});

  auto f = caf::make_function_view(actor);

  bool result = f(StartAtom::value)->get_as<bool>(0);

  std::vector<OrderPosition> positions =
      f(QryInvestorPositionsAtom::value)->get_as<std::vector<OrderPosition> >(0);

  std::vector<OrderRtnData> orders =
      f(RestartRtnOrdersAtom::value)->get_as<std::vector<OrderRtnData>>(0);

  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      break;
    }
  }
  return 0;
}

CAF_MAIN()
