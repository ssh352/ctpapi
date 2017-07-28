#ifndef COMMON_API_STRUCT_H
#define COMMON_API_STRUCT_H
#include "common/api_data_type.h"

struct OrderField {
  OrderDirection direction;
  PositionEffect position_effect;
  OrderStatus status;
  int qty;
  int leaves_qty;
  int traded_qty;
  int error_id;
  int raw_error_id;
  double price;
  double avg_price;
  std::string account_id;
  std::string instrument_id;
  std::string exchange_id;
  std::string date;
  std::string input_time;
  std::string update_time;
  std::string order_id;
  std::string raw_error_message;
};

struct InvestorPositionField {

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


#endif  // COMMON_API_STRUCT_H
