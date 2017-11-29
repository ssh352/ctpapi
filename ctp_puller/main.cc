
#include "caf/all.hpp"
#include "init_instrument_list.h"

class my_config : public caf::actor_system_config {
 public:
  std::string server;
  std::string broker_id;
  std::string user_id;
  std::string password;
  my_config() {
    opt_group{custom_options_, "ctp"}
        .add(server, "server", "ctp front server")
        .add(broker_id, "broker_id", "broker id")
        .add(user_id, "user_id", "user id")
        .add(password, "password", "password");
  }
};
int caf_main(caf::actor_system& system, const my_config& cfg) {
  InitInstrumenList(system, cfg.server, cfg.broker_id, cfg.user_id,
                    cfg.password);
  return 0;
}

CAF_MAIN()
