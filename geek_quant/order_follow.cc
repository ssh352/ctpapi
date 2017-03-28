#include "order_follow.h"

void OrderFollow::MakeOpening(int opening_volume,
                              OrderDirection order_direction) {
  opening_ = opening_volume;
  order_direction_ = order_direction;
}

void OrderFollow::MakePosition(int volume, OrderDirection order_direction) {
  order_direction_ = order_direction;
  position_ = volume;
}

OrderFollow::OrderFollow() {
  opening_ = 0;
  position_ = 0;
  closeing_ = 0;
  closed_ = 0;
  canceled_ = 0;
}

int OrderFollow::CancelableVolume() const {
  return opening_;
}

void OrderFollow::HandleOpened(int volume) {
  opening_ -= volume;
  position_ += volume;
}

void OrderFollow::HandleCloseing(int volume) {
  closeing_ += volume;
  position_ -= volume;
}

void OrderFollow::HandleClosed(int volume) {
  closeing_ -= volume;
  closed_ += volume;
}

void OrderFollow::HandleCanceledByOpen() {
  opening_ = 0;
}

void OrderFollow::HandleCanceledByClose() {
  position_ += closeing_;
  closeing_ = 0;
}
