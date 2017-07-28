#ifndef FOLLOW_TRADE_ORDER_H
#define FOLLOW_TRADE_ORDER_H
#include <boost/shared_ptr.hpp>
#include "common/api_struct.h"

class Order {
 public:
  Order() = default;
  Order(const boost::shared_ptr<const OrderField>& data);
  const std::string& instrument() const { return data_->instrument_id; }

  OrderDirection direction() const { return data_->direction; }

  bool IsQuantityChange(int traded_qty) const;

  bool IsActiveOrder() const;

  bool IsOpen() const;

  int unfill_quantity() const;

  OrderField order_data() const;

 private:
  boost::shared_ptr<const OrderField> data_;
  std::vector<std::pair<std::string, int> > order_quantitys_;
};

#endif  // FOLLOW_TRADE_ORDER_H
