#ifndef CTP_BIND_API_STRUCT_H
#define CTP_BIND_API_STRUCT_H

enum class OrderDirection { kBuy, kSell };

enum class PositionEffect { kOpen, kClose, kCloseToday };

enum class OrderStatus {
  kActive,
  kAllFilled,
  kRejected,
};

struct OrderField {
  std::string instrument_name;
  std::string instrument_id;
  std::string exchange_id;
  OrderDirection direction;
  int qty;
  double price;
  PositionEffect position_effect;
  std::string datetime;
  std::string order_id;
  OrderStatus status;
  int leaves_qty;
  int trade_qty;
  double avg_price;
  int error_id;
  int raw_error_id;
  std::string error_message;
};

#endif  // CTP_BIND_API_STRUCT_H
