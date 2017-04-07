#ifndef FOLLOW_TRADE_ORDER_H
#define FOLLOW_TRADE_ORDER_H
#include "geek_quant/caf_defines.h"

class Order {
 public:
  Order() = default;
  Order(OrderData&& data);

 private:
  OrderData data_;
};

#endif  // FOLLOW_TRADE_ORDER_H
