#ifndef FOLLOW_TRADE_ORDER_FOLLOW_H
#define FOLLOW_TRADE_ORDER_FOLLOW_H
#include <string>

class OrderFollow {
 public:
  OrderFollow(const std::string& order_no, int total_volume);
  int CancelableVolume() const;

  const std::string& trade_order_no() const;

  const std::string& follow_order_no() const;

  void FillOpenOrderForTrade(int volume);

  void FillOpenOrderForFollow(int volume);

  int ProcessCloseOrder(const std::string& order_no,
                        int close_volume,
                        int* follow_close_volume,
                        bool* cancel_order);

 private:
  std::string trade_order_no_;
  std::string follow_order_no_;

  int position_volume_for_trade_ = 0;
  int position_volume_for_follow_ = 0;

  int closeing_volume_ = 0;

  int total_volume_for_follow_ = 0;
};

#endif  // FOLLOW_TRADE_ORDER_FOLLOW_H
