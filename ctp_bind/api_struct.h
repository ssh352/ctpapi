#ifndef CTP_BIND_API_STRUCT_H
#define CTP_BIND_API_STRUCT_H

enum class OrderDirection {
  kBuy,
  kSell 
};

enum class PositionEffect { 
  kUndefine,
  kOpen,
  kClose,
  kCloseToday 
};

enum class OrderStatus {
  kActive,
  kAllFilled,
  kCanceled,
  kRejected,
};

struct OrderField {
  // std::string instrument_name;
  std::string instrument_id;
  std::string exchange_id;
  OrderDirection direction;
  int qty;
  double price;
  PositionEffect position_effect;
  std::string date;
  std::string input_time;
  std::string update_time;
  std::string order_id;
  OrderStatus status;
  int leaves_qty;
  int traded_qty;
  double avg_price;
  int error_id;
  int raw_error_id;
  std::string raw_error_message;
  std::string addition_info;
};

#endif  // CTP_BIND_API_STRUCT_H
