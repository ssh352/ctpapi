#include <string>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "caf/all.hpp"
#include "tradeapi/ThostFtdcTraderApi.h"

using std::endl;
using std::string;

using namespace caf;


using LoginAtom = atom_constant<atom("login")>;
using OrderPushAtom = atom_constant<atom("order_push")>;
using BatchOrderPushAtom = atom_constant<atom("batchorder")>;

using Broker = typed_actor<reacts_to<LoginAtom>, reacts_to<OrderPushAtom> >;

Broker::behavior_type CtpBroker(event_based_actor* self) {
  return {
    [=](LoginAtom) {
      aout(self) << "LoginAtom Message \n";
    },
    [](OrderPushAtom) {

    },
  };
}


int main() {
  // our CAF environment
  actor_system_config cfg;
  actor_system system{cfg};

  auto ctp_broker = boost::make_shared<CTPTradingSpi>(system);
  // ctp_broker->LoginServer("053867", "8661188");
  ctp_broker->LoginServer("tcp://180.168.146.187:10000", "9999", "053867", "8661188");
  // ctp_broker->LoginServer("tcp://59.42.241.91:41205", "9080", "38030022", "140616");

  // system will wait until both actors are destroyed before leaving main
  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      break;
    }
  }
}
