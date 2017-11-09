#ifndef BACKTESTING_CAF_CONFIG_H
#define BACKTESTING_CAF_CONFIG_H
#include "caf/all.hpp"


class config : public caf::actor_system_config {
 public:
  int force_close_before_close_market = 5;
  bool enable_logging = false;
  bool run_benchmark = false;
  std::string strategy_config_file = "";
  std::string instrument_infos_file = "";
  std::string ts_db = "";
  std::string cta_transaction_db = "";
  std::string backtesting_from_datetime = "";
  std::string backtesting_to_datetime = "";
  std::string out_dir = "";

  config();
};

#endif // BACKTESTING_CAF_CONFIG_H



