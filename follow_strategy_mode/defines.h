#ifndef FOLLOW_TRADE_DEFINES_H
#define FOLLOW_TRADE_DEFINES_H

enum class OrderEventType {
  kIgnore,
  kNewOpen,
  kNewClose,
  kOpenTraded,
  kCloseTraded,
  kCanceled,
};

enum class OrderDirection {
  kUndefine,
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
  kInputRejected,
  kCancelRejected,
};

struct OrderField {
  // std::string instrument_name;
  std::string account_id;
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
};

struct OrderData {
  std::string account_id_;
  std::string order_id_;
  std::string instrument_;
  std::string datetime_;
  std::string order_sys_id_;
  std::string exchange_id_;
  int quanitty_;
  int filled_quantity_;
  int session_id_;
  double price_;
  OrderDirection direction_;
  OrderStatus status_;
  PositionEffect position_effect_;

  const std::string& account_id() const { return account_id_; }
  const std::string& order_id() const { return order_id_; }
  const std::string& order_sys_id() const { return order_sys_id_; }
  const std::string& instrument() const { return instrument_; }
  const std::string& datetime() const { return datetime_; }
  const std::string& exchange_id() const { return exchange_id_; }
  int quanitty() const { return quanitty_; }
  int filled_quantity() const { return filled_quantity_; }
  int session_id() const { return session_id_; }
  double price() const { return price_; }
  OrderDirection direction() const { return direction_; }
  OrderStatus status() const { return status_; }
  PositionEffect position_effect() const { return position_effect_; }
};

struct CorrOrderQuantity {
  std::string order_id;
  int quantity;
  int corr_quantity;
};

struct OrderPosition {
  std::string instrument;
  OrderDirection order_direction;
  int quantity;
};

struct OrderQuantity {
  std::string order_id;
  OrderDirection direction;
  bool is_today_quantity;
  int quantity;
  int closeable_quantity;
};

struct AccountPortfolio {
  std::string instrument;
  OrderDirection direction;
  int closeable;
  int open;
  int close;
};

struct AccountPosition {
  std::string instrument;
  OrderDirection direction;
  int closeable;
};

#endif  // FOLLOW_TRADE_DEFINES_H
