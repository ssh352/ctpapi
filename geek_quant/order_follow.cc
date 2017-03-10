#include "order_follow.h"

OrderFollow::OrderFollow(const std::string& order_no, int total_volume)
    : trade_order_no_(order_no),
      follow_order_no_(order_no),
      total_volume_for_follow_(total_volume) {}

int OrderFollow::CancelableVolume() const {
  return total_volume_for_follow_ - position_volume_for_follow_;
}

const std::string& OrderFollow::trade_order_no() const {
  return trade_order_no_;
}

const std::string& OrderFollow::follow_order_no() const {
  return follow_order_no_;
}

void OrderFollow::FillOpenOrderForTrade(int volume) {
  position_volume_for_trade_ += volume;
}

void OrderFollow::FillOpenOrderForFollow(int volume) {
  position_volume_for_follow_ += volume;
}

int OrderFollow::ProcessCloseOrder(const std::string& order_no,
                                   int close_volume,
                                   int* follow_close_volume,
                                   bool* cancel_order) {
  int real_close_volume =
      std::min<int>(position_volume_for_trade_, close_volume);
  *follow_close_volume = std::max<int>(
      real_close_volume -
          (position_volume_for_trade_ - position_volume_for_follow_),
      0);

  int canceling_volume =
      (total_volume_for_follow_ - position_volume_for_follow_);
  total_volume_for_follow_ -= canceling_volume;
  *cancel_order = canceling_volume > 0 ? true : false;

  position_volume_for_trade_ -= real_close_volume;
  position_volume_for_follow_ -= *follow_close_volume;
  total_volume_for_follow_ -= *follow_close_volume;
  return close_volume - real_close_volume;
}
