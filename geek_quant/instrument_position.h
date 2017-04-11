#ifndef FOLLOW_TRADE_INSTRUMENT_POSITION_H
#define FOLLOW_TRADE_INSTRUMENT_POSITION_H
#include "geek_quant/caf_defines.h"

class InstrumentPosition {
 public:
  std::vector<OrderQuantity> GetQuantitysWithOrderIds(
      std::vector<std::string> orders);

  int GetPositionCloseableQuantity(OrderDirection direction);

 private:
  std::vector<Position> buy_positions_;
  std::vector<Position> sell_positions_;
  std::vector<CorrCloseOrder
};

#endif  // FOLLOW_TRADE_INSTRUMENT_POSITION_H
