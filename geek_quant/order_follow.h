#ifndef FOLLOW_TRADE_ORDER_FOLLOW_H
#define FOLLOW_TRADE_ORDER_FOLLOW_H
#include <string>
#include "geek_quant/caf_defines.h"

class OrderFollow {
 public:
  OrderFollow();

  void MakeOpening(int opening_volume, OrderDirection order_direction);

  void MakePosition(int volume, OrderDirection order_direction);

  int CancelableVolume() const;

  int total_volume() const { return opening_ + position_; }

  int position_volume() const { return position_; }

  OrderDirection order_direction() const { return order_direction_; }

  void HandleOpened(int volume);

  void HandleCloseing(int volume);

  void HandleClosed(int volume);

  void HandleCanceledByOpen();

  void HandleCanceledByClose();

  int opening_;
  int position_;
  int closeing_;
  int closed_;
  int canceled_;
  OrderDirection order_direction_;
};

#endif  // FOLLOW_TRADE_ORDER_FOLLOW_H
