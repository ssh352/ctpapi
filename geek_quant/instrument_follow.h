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

 private:
  int UnfillVolume() const;
  int PositionVolume() const;

  struct FollowOrder {
    std::string order_no;
    int total_volume;
    int position_volume;
    FollowOrder() = delete;
  };
  std::vector<FollowOrder> follow_orders_;
};

#endif  // FOLLOW_TRADE_INSTRUMENT_FOLLOW_H
