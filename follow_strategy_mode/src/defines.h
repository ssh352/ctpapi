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
  kFilled,
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

struct PositionData {
  // PositionData() = delete;
  // PositionData() {
  //   order_direction = kODInvalid;
  //   volume = 0;
  // }
  std::string instrument;
  OrderDirection order_direction;
  double volume;
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
  const std::string& instrument() const { return instrument_; }
  const std::string& datetime() const { return datetime_; }
  const std::string& user_product_info() const { return user_product_info_; }
  const std::string& exchange_id() const { return exchange_id_;  }
  int quanitty() const { return quanitty_; }
  int filled_quantity() const { return filled_quantity_; }
  int session_id() const { return session_id_; }
  double price() const { return price_; }
  OrderDirection direction() const { return direction_; }
  OrderPriceType type() const { return type_; }
  OrderStatus status() const { return status_; }
  PositionEffect position_effect() const { return position_effect_; }
};

struct RtnOrderData {
  RtnOrderData() {
    order_status = OldOrderStatus::kInvalid;
    order_direction = OrderDirection::kUnkown;
    order_price = 0.0;
    request_by = RequestBy::kInvalid;
    volume = 0;
    session_id = 0;
    today = false;
  }
  std::string account_id;
  std::string order_no;
  std::string instrument;
  int session_id;
  OrderDirection order_direction;
  OldOrderStatus order_status;
  RequestBy request_by;
  bool today;
  double order_price;
  int volume;
};

struct CorrOrderQuantity {
  std::string order_no;
  int quantity;
  int corr_quantity;
};

struct EnterOrderData {
  EnterOrderData() {
    action = EnterOrderAction::kInvalid;
    order_direction = OrderDirection::kUnkown;
    order_price = 0.0;
    today = false;
    volume = 0;
  }
  std::string order_no;
  std::string instrument;
  EnterOrderAction action;
  OrderDirection order_direction;
  double order_price;
  int volume;
  bool today;
};

struct OrderVolume {
  std::string order_no;
  OrderDirection order_direction;
  int opening;
  int position;
  int closeing;
  int closed;
  int canceling;
  int canceled;
};

struct OrderIdent {
  int front_id;
  int session_id;
  std::string order_id;
  std::string exchange_id;
  std::string order_sys_id;
};

struct CloseingActionItem {
  int close_volume;
  int position_volume;
};
struct CloseingActionInfo {
  std::string order_no;
  std::map<std::string, CloseingActionItem> items;
};

struct OpenReverseOrderItem {
  std::string order_no;
  int volume;
  int reverse_volume;
};

struct OpenReverseOrderActionInfo {
  std::string order_no;
  std::vector<OpenReverseOrderItem> items;
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


#endif // FOLLOW_TRADE_DEFINES_H



