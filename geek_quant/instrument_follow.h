#ifndef FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
#define FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/order_follow.h"

class InstrumentFollow {
 public:
  InstrumentFollow();
  void HandleOrderRtnForTrader(const OrderRtnData& order,
                               EnterOrderData* enter_order,
                               std::vector<std::string>* cancel_order_no_list);

  void HandleOrderRtnForFollow(const OrderRtnData& order,
                               EnterOrderData* enter_order,
                               std::vector<std::string>* cancel_order_no_list);

 private:
  int CalcOrderReverseVolume(int order_volume) const;

  std::vector<OrderFollow> order_follows_;
  OrderDirection order_direction_;
};

#endif  // FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
