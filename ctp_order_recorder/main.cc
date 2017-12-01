#include "caf/all.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "ctp_order_recorder.h"

namespace pt = boost::property_tree;
int caf_main(caf::actor_system& system) {
  pt::ptree pt;
  try {
    pt::read_json("config.json", pt);
  } catch (pt::ptree_error& err) {
    std::cout << "Read Confirg File Error:" << err.what() << "\n";
    return 1;
  }

  std::vector<caf::actor> actors;
  for (auto item : pt) {
    if (item.second.get<bool>("enable")) {
      actors.push_back(system.spawn<CtpOrderRecorder>(
          item.second.get<std::string>("front_server"),
          item.second.get<std::string>("broker_id"),
          item.second.get<std::string>("user_id"),
          item.second.get<std::string>("password")));
    }
  }

  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      for (const auto& actor : actors) {
        caf::anon_send(actor, CtpDisconnectAtom::value);
      }
      actors.clear();
      break;
    }
  }
  system.await_all_actors_done();
  return 0;
}

CAF_MAIN()