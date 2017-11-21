#include "caf_config.h"

config::config() {
  opt_group{custom_options_, "global"}
      .add(strategy_config_file, "strategy_config_file",
           "strategy config file path")
      .add(instrument_infos_file, "instrument_infos_file",
           "instrument infos file path")
      .add(force_close_before_close_market, "force_close",
           "force close before close market minues. default is 5")
      .add(cancel_limit_order_when_switch_trade_date,
           "cancel_limit_order_when_switch_trade_date",
           "cancel limit order when switch trade date.")
      .add(enable_logging, "enable_logging", "enable logging")
      .add(ts_db, "tick_db", "tick time series db")
      .add(cta_transaction_db, "ts_cta_db", "cta transaction time series db")
      .add(backtesting_from_datetime, "bs_from_datetime",
           "backtesting from datetime %Y-%m-%d %H:%M:%D, default is "
           "\"2016-12-05 09:00:00\"")
      .add(backtesting_to_datetime, "bs_to_datetime",
           "backtesting from datetime %Y-%m-%d %H:%M:%D, default is "
           "\"2017-07-23 15:00:00\"")
      .add(run_benchmark, "run_benchmark", "run benchmark")
      .add(out_dir, "out_dir", "");
}
