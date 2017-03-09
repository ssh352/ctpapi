#ifndef STRATEGY_UNITTEST_INSTRUMENT_ORDER_H
#define STRATEGY_UNITTEST_INSTRUMENT_ORDER_H
#include "caf_defines.h"

class InstrumentOrder {
 public:
  class OrderDelegate {
   public:
  };

  InstrumentOrder(OrderDelegate* delegate, const OrderRtnData& order);
  ~InstrumentOrder();

  void HandleOrderRtnData(OrderRtnFrom from, const OrderRtnData& order);

 private:
  struct InnerOrderData {
    OrderDirection order_direction;
    OrderStatus order_status;
    int unfill_volume;
    int position_volume;
  };

  InnerOrderData source_order_;
  InnerOrderData dest_order_;
  OrderDelegate* delegate_;
};

#endif  // STRATEGY_UNITTEST_INSTRUMENT_ORDER_H
