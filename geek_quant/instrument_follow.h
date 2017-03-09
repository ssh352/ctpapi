#ifndef FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
#define FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
#include "geek_quant/caf_defines.h"

class InstrumentFollow {
public:
  void HandleOrderRtnForTrader(const OrderRtnData& order, 
                               EnterOrderData* enter_order, 
                               std::vector<std::string>* cancel_order_no_list);

  void HandleOrderRtnForFollow(const OrderRtnData& order, 
                               EnterOrderData* enter_order, 
                               std::vector<std::string>* cancel_order_no_list);
};


#endif // FOLLOW_TRADE_INSTRUMENT_FOLLOW_H



