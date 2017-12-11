#ifndef BACKTESTING_CAF_CONFIG_H
#define BACKTESTING_CAF_CONFIG_H
#include "caf/all.hpp"


class config : public caf::actor_system_config {
 public:
  std::string out_dir = "";

  config();
};

#endif // BACKTESTING_CAF_CONFIG_H



