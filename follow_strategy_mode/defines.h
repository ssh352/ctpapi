#ifndef FOLLOW_TRADE_DEFINES_H
#define FOLLOW_TRADE_DEFINES_H

enum class RequestBy {
  kInvalid,
  kCTA,
  kStrategy,
  kApp,
};
enum class OrderRtnFrom {
  kInvalid,
  kSource,
  kDest,
};

enum class OrderDirection {
  kUnkown,
  kBuy,
  kSell,
};

enum class EnterOrderAction {
  kInvalid,
  kOpen,
  kClose,
  kOpenConfirm,
  kCloseConfirm,
  kOpenReverseOrder,
  kOpenReverseOrderConfirm,
  kCancelForTest,
};

enum class OrderStatus {
  kActive,
  kAllFilled,
  kCancel,
};

enum class OldOrderStatus {
  kInvalid,
  kOpening,
  kCloseing,
  kOpened,
  kClosed,
  kOpenCanceled,
  kCloseCanceled,
};

enum class OrderEventType {
  kIgnore,
  kNewOpen,
  kNewClose,
  kOpenTraded,
  kCloseTraded,
  kCanceled,
};

enum class OpenClose {
  kInvalid,
  kOpen,
  kClose,
};

enum class OrderPriceType {
  kLimit,
  kMarket,
};

enum class PositionEffect {
  kOpen,
  kClose,
  kCloseToday,
};

struct OpenOrderData {
  std::string instrument;
  OrderDirection direction;
  OldOrderStatus order_status;
};

struct OrderData {
  std::string account_id_;
  std::string order_id_;
  std::string instrument_;
  std::string datetime_;
  std::string user_product_info_;
  std::string order_sys_id_;
  std::string exchange_id_;
  int quanitty_;
  int filled_quantity_;
  int session_id_;
  double price_;
  OrderDirection direction_;
  OrderPriceType type_;
  OrderStatus status_;
  PositionEffect position_effect_;

  const std::string& account_id() const { return account_id_; }
  const std::string& order_id() const { return order_id_; }
  const std::string& order_sys_id() const { return order_sys_id_; }
  const std::string& instrument() const { return instrument_; }
  const std::string& datetime() const { return datetime_; }
  const std::string& user_product_info() const { return user_product_info_; }
  const std::string& exchange_id() const { return exchange_id_; }
  int quanitty() const { return quanitty_; }
  int filled_quantity() const { return filled_quantity_; }
  int session_id() const { return session_id_; }
  double price() const { return price_; }
  OrderDirection direction() const { return direction_; }
  OrderPriceType type() const { return type_; }
  OrderStatus status() const { return status_; }
  PositionEffect position_effect() const { return position_effect_; }
};

struct CorrOrderQuantity {
  std::string order_no;
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
