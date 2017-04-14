#ifndef FOLLOW_TRADE_SERVER_BINARY_SERIALIZATION_H
#define FOLLOW_TRADE_SERVER_BINARY_SERIALIZATION_H
#include "follow_strategy_mode/defines.h"

namespace boost {
namespace serialization {

template <class Archive>
void serialize(Archive& ar, OrderPosition& order, const unsigned int version) {
  ar& order.instrument;
  ar& order.order_direction;
  ar& order.quantity;
}

template <class Archive>
void serialize(Archive& ar, OrderData& order, const unsigned int version) {
  ar & order.account_id_;
  ar & order.order_id_;
  ar & order.instrument_;
  ar & order.datetime_;
  ar & order.user_product_info_;
  ar & order.order_sys_id_;
  ar & order.exchange_id_;
  ar & order.quanitty_;
  ar & order.filled_quantity_;
  ar & order.session_id_;
  ar & order.price_;
  ar & order.direction_;
  ar & order.type_;
  ar & order.status_;
  ar & order.position_effect_;
}

}  // namespace serialization
}  // namespace boost

#endif  // FOLLOW_TRADE_SERVER_BINARY_SERIALIZATION_H
