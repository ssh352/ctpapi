#ifndef BACKTESTING_PRICE_HANDLER_H
#define BACKTESTING_PRICE_HANDLER_H
#include <boost/format.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "hpt_core/tick_series_data_base.h"
#include "common/api_data_type.h"
#include "bft_core/channel_delegate.h"

class PriceHandler {
 public:
  PriceHandler(
      const std::string& instrument,
      bool* running,
      bft::ChannelDelegate* mail_box,
      std::vector<std::pair<std::shared_ptr<Tick>, int64_t> > tick_containter,
      int event_before_close_market_seconds,
      int event_before_close_market_near_seconds);

  void StreamNext();

 private:
  bool IsNextMarketOpen(Tick* tick) const;

  bool NeedsSeedBeforeCloseMarketEvent(Tick* tick);

  TradingTime GetTradingTime(TimeStamp time_stamp) const;

  void UpdateNextBeforeCloseMarketEventTimeStamp(TradingTime trading_time,
                                                 TimeStamp current_timestamp);

  bool NeedsSendCloseMarketNearEvent(Tick* tick);

  bool NeedsSendDaySettleEvent(TimeStamp timestamp) const;

  std::vector<std::pair<std::shared_ptr<Tick>, int64_t> > tick_containter_;
  std::vector<std::pair<std::shared_ptr<Tick>, int64_t> >::const_iterator it_;
  std::shared_ptr<std::string> instrument_;
  int current_tick_index_ = 0;
  bool* running_;
  bft::ChannelDelegate* mail_box_;
  TimeStamp previous_time_stamp_ = 0;
  int event_before_close_market_seconds_ = 0;
  int event_before_close_market_near_seconds_ = 0;
  TimeStamp next_event_before_close_market_timestamp_ = 0;
  TimeStamp next_event_before_close_market_near_timestamp_ = 0;
};

#endif  // BACKTESTING_PRICE_HANDLER_H
