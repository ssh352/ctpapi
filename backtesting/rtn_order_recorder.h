#ifndef BACKTESTING_RTN_ORDER_RECORDER_H
#define BACKTESTING_RTN_ORDER_RECORDER_H
#include "follow_strategy/cta_order_signal_subscriber.h"

class RtnOrderToCSV {
 public:
  RtnOrderToCSV(bft::ChannelDelegate* mail_box,
                const std::string& out_dir,
                const std::string& prefix_);

  void HandleOrder(const std::shared_ptr<OrderField>& order);

 private:
  bft::ChannelDelegate* mail_box_;
  mutable std::ofstream orders_csv_;
};


#endif // BACKTESTING_RTN_ORDER_RECORDER_H



