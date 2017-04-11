#ifndef FOLLOW_TRADE_ORDER_H
#define FOLLOW_TRADE_ORDER_H
#include "geek_quant/caf_defines.h"

class Order {
 public:
  Order() = default;
  Order(OrderData&& data);
  const std::string& instrument() const {
    return data_.instrument();
  }

  bool IsUnfillOrder() const;
 private:
  OrderData data_;
  std::vector<std::pair<std::string, int> > order_quantitys_;
};

#endif  // FOLLOW_TRADE_ORDER_H
