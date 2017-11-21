#ifndef BACKTESTING_BACKTESTING_DEFINES_H
#define BACKTESTING_BACKTESTING_DEFINES_H
#include <string>
#include "common/api_struct.h"
struct StrategyParam {
  std::string instrument;
  std::string market;
  double margin_rate;
  int constract_multiple;
  CostBasis cost_basis;
  int delay_open_order_after_seconds;
  int wait_optimal_open_price_fill_seconds;
  double price_offset;
};

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(StrategyParam)

#endif  // BACKTESTING_BACKTESTING_DEFINES_H
