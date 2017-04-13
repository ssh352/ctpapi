#ifndef FOLLOW_TRADE_ORDER_H
#define FOLLOW_TRADE_ORDER_H
#include "follow_strategy_mode/defines.h"

class Order {
 public:
  Order() = default;
  Order(OrderData&& data);
  const std::string& instrument() const {
    return data_.instrument();
  }

  OrderDirection direction() const {
    return data_.direction();
  }
  
  bool IsQuantityChange(int filled_quantity) const;

  bool IsActiveOrder() const;

  OrderData order_data() const;
 private:
  OrderData data_;
  std::vector<std::pair<std::string, int> > order_quantitys_;
};

#endif  // FOLLOW_TRADE_ORDER_H
