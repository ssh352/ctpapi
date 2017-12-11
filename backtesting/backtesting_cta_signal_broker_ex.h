#ifndef _BACKTESTING_CTA_SIGNAL_BROKER_EX_H
#define _BACKTESTING_CTA_SIGNAL_BROKER_EX_H

#include "hpt_core/order_util.h"
#include "follow_strategy/cta_order_signal_subscriber.h"
#include "caf_common/caf_atom_defines.h"

class BacktestingCTASignalBrokerEx {
 public:
  BacktestingCTASignalBrokerEx(
      bft::ChannelDelegate* mail_box,
      std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>
          cta_signal_container,
      std::string instrument);

  void BeforeTrading(BeforeTradingAtom,
                     const TradingTime& trading_time);

  void HandleTick(const std::shared_ptr<TickData>& tick);

 private:
  PositionEffect ParseThostFtdcPosition(int position_effect) const;

  OrderDirection ParseThostFtdcOrderDirection(int order_direction) const;

  bft::ChannelDelegate* mail_box_;
  std::list<std::shared_ptr<CTATransaction>> transactions_;
  std::list<std::shared_ptr<CTATransaction>>::iterator range_beg_it_;
  std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>> keep_memory_;

  CTAOrderSignalSubscriber order_signal_subscriber_;
  int delayed_input_order_minute_ = 0;
  int cancel_order_after_minute_ = 0;
  int backtesting_position_effect_ = 0;
  int current_order_id_ = 0;
  std::ofstream csv_;
  std::string instrument_;
};

#endif  // _BACKTESTING_CTA_SIGNAL_BROKER_EX_H
