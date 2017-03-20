#include "order_follow.h"

OrderFollow::OrderFollow(const std::string& order_no,
                         int total_volume,
                         OrderDirection order_direction) {
  memset(&trader_, 0, sizeof(OrderVolume));
  memset(&follower_, 0, sizeof(OrderVolume));
  trader_.order_no = order_no;
  trader_.opening = total_volume;
  trader_.order_direction = order_direction;

  follower_.order_no = order_no;
  follower_.opening = total_volume;
  follower_.order_direction = order_direction;
}

OrderFollow::OrderFollow(OrderVolume trader) : trader_(trader) {}

void OrderFollow::InitFollowerOrderVolue(OrderVolume order) {
  follower_ = order;
}

int OrderFollow::CancelableVolume() const {
  return follower_.opening;
}

const std::string& OrderFollow::trade_order_no() const {
  return trader_.order_no;
}

const std::string& OrderFollow::follow_order_no() const {
  return follower_.order_no;
}

void OrderFollow::FillOpenOrderForTrade(int volume) {
  trader_.position += volume;
  trader_.opening -= volume;
}

void OrderFollow::FillOpenOrderForFollow(int volume) {
  follower_.position += volume;
  follower_.opening -= volume;
}

int OrderFollow::ProcessCloseOrder(const std::string& order_no,
                                   int close_volume,
                                   int* close_volume_by_follower,
                                   bool* cancel_order) {
  int close_volume_by_trader = std::min<int>(trader_.position, close_volume);

  *close_volume_by_follower = std::max<int>(
      close_volume_by_trader - (trader_.position - follower_.position), 0);

  *cancel_order = follower_.opening > 0 ? true : false;
  follower_.canceling = follower_.opening;
  follower_.opening = 0;

  trader_.position -= close_volume_by_trader;
  follower_.position -= *close_volume_by_follower;
  return close_volume - close_volume_by_trader;
}
