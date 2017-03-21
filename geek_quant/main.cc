#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

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

int caf_main(caf::actor_system& system, const caf::actor_system_config& cfg) {
  CtaTradeActor cta(system.spawn<FollowTradeActor>());
  // cta.Start("tcp://59.42.241.91:41205", "9080", "38030022", "140616");
  cta.Start("tcp://180.168.146.187:10000", "9999", "053867", "8661188");
  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      break;
    }
  }
  return 0;
}

CAF_MAIN()
