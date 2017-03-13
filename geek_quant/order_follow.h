#ifndef FOLLOW_TRADE_ORDER_FOLLOW_H
#define FOLLOW_TRADE_ORDER_FOLLOW_H
#include <string>
#include "geek_quant/caf_defines.h"

class OrderFollow {
 public:
  OrderFollow(const std::string& order_no,
              int total_volume,
              OrderDirection order_direction);
  int CancelableVolume() const;

  const std::string& trade_order_no() const;

  const std::string& follow_order_no() const;

  int position_volume_for_trade() const {
    return trader_.position;
  }

  int position_volume_for_follow() const {
    return follower_.position;
  }

  int total_volume_for_follow() const {
    return follower_.opening + follower_.position;
  }

  int total_volume_for_trade() const {
    return trader_.opening + trader_.position;
  }

  OrderDirection order_direction() const {
    return order_direction_;
  }

  void FillOpenOrderForTrade(int volume);

  void FillOpenOrderForFollow(int volume);

  int ProcessCloseOrder(const std::string& order_no,
                        int close_volume,
                        int* follow_close_volume,
                        bool* cancel_order);

 private:
  std::string trade_order_no_;

  std::string follow_order_no_;

  OrderDirection order_direction_ = kODUnkown;

  struct OrderVolume {
    int opening;
    int position;
    int closeing;
    int closed;
    int canceling;
    int canceled;
  };
  
  OrderVolume trader_;
  OrderVolume follower_;
};

#endif  // FOLLOW_TRADE_ORDER_FOLLOW_H
