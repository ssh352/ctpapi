#include <iostream>
#include "caf/all.hpp"
#include "caf/io/all.hpp"

caf::behavior Pong(caf::event_based_actor* self) {
  return {
    [=](const std::string& what) {
      std::cout << what << std::endl;
    },
  };
}

int caf_main(caf::actor_system& system) {
  auto pong = system.spawn(Pong);
  auto ping = system.middleman().remote_actor("127.0.0.1", 4242);
  if (!ping) {
    std::cerr << "unable to connect to ping:" << system.render(ping.error())
              << std::endl;
  } else {
    std::string input;
    while (std::cin >> input) {
      caf::send_as(pong, *ping, input);
    }
  }
  return 0;
}

CAF_MAIN(caf::io::middleman)
