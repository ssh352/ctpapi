#ifndef STRATEGIES_WR_STRATEGY_H
#define STRATEGIES_WR_STRATEGY_H
#include <boost/format.hpp>
#include <list>
#include <set>
#include "common/api_struct.h"

template <class MailBox>
class WRStrategy {
 public:
  WRStrategy(MailBox* mail_box) : mail_box_(mail_box) {
    mail_box_->Subscribe(&WRStrategy::HandleTick, this);
    mail_box_->Subscribe(&WRStrategy::HandleOrder, this);
    mail_box_->Subscribe(&WRStrategy::HandleCloseMarket, this);
  }

  void HandleTick(const std::shared_ptr<TickData>& tick) {
    std::cout << tick->tick->timestamp << ":" << tick->tick->last_price << "\n";
  }

  void HandleOrder(const std::shared_ptr<OrderField>& order) {}

  void HandleCloseMarket(const CloseMarket&) {}

 private:
  MailBox* mail_box_;
};

#endif  // STRATEGIES_WR_STRATEGY_H
