#ifndef BACKTESTING_PRICE_HANDLER_H
#define BACKTESTING_PRICE_HANDLER_H
#include <boost/format.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "hpt_core/tick_series_data_base.h"
#include "common/api_data_type.h"

template <class MailBox>
class PriceHandler {
 public:
  PriceHandler(
      const std::string& instrument,
      bool* running,
      MailBox* mail_box,
      std::vector<std::pair<std::shared_ptr<Tick>, int64_t> > tick_containter,
      int event_before_close_market_seconds,
      int event_before_close_market_near_seconds)
      : instrument_(std::make_shared<std::string>(instrument)),
        running_(running),
        mail_box_(mail_box),
        tick_containter_(std::move(tick_containter)),
        event_before_close_market_seconds_(event_before_close_market_seconds),
        event_before_close_market_near_seconds_(
            event_before_close_market_near_seconds) {
    it_ = tick_containter_.begin();
  }

  void StreamNext() {
    if (current_tick_index_ >= it_->second) {
      current_tick_index_ = 0;
      ++it_;
    }

    if (it_ == tick_containter_.end()) {
      *running_ = false;
      return;
    }

    Tick* tick = &it_->first.get()[current_tick_index_];

    if (IsNextMarketOpen(tick)) {
      TradingTime trading_time = GetTradingTime(tick->timestamp);
      mail_box_->Send(BeforeTradingAtom::value, trading_time);
      UpdateNextBeforeCloseMarketEventTimeStamp(trading_time, tick->timestamp);
    } else if (NeedsSeedBeforeCLoseMarketEvent(tick)) {
      mail_box_->Send(BeforeCloseMarketAtom::value);
      next_event_before_close_market_timestamp_ = 0;
    } else if (NeedsSendCloseMarketNearEvent(tick)) {
      mail_box_->Send(CloseMarketNearAtom::value);
      next_event_before_close_market_near_timestamp_ = 0;
    } else {
      auto null_deleter = [](Tick* tick) {};
      // event_queue_->push_back(std::make_shared<TickEvent>(std::shared_ptr<Tick>(
      //     &it_->first.get()[current_tick_index_], null_deleter)));
      auto tick_data = std::make_shared<TickData>();
      tick_data->instrument = instrument_;
      tick_data->tick = std::shared_ptr<Tick>(tick, null_deleter);

      mail_box_->Send(std::move(tick_data));
      ++current_tick_index_;
    }
    previous_time_stamp_ = tick->timestamp;
  }

 private:
  bool IsNextMarketOpen(Tick* tick) const {
    if (previous_time_stamp_ == 0) {
      return true;
    }
    return (tick->timestamp - previous_time_stamp_) / 1000.0 > 3 * 3600;
  }

  bool NeedsSeedBeforeCLoseMarketEvent(Tick* tick) {
    return next_event_before_close_market_timestamp_ != 0 &&
           tick->timestamp > next_event_before_close_market_timestamp_;
  }

  TradingTime GetTradingTime(TimeStamp time_stamp) {
    boost::posix_time::ptime now(boost::gregorian::date(1970, 1, 1),
                                 boost::posix_time::milliseconds(time_stamp));
    boost::posix_time::ptime day(now.date(),
                                 boost::posix_time::time_duration(9, 0, 0));
    boost::posix_time::ptime night(now.date(),
                                   boost::posix_time::time_duration(21, 0, 0));

    TradingTime trading_time = TradingTime::kDay;
    if (std::abs((now - day).total_seconds()) < 600) {
    } else if (std::abs((now - night).total_seconds()) < 600) {
      trading_time = TradingTime::kNight;
    } else {
      BOOST_ASSERT(false);
    }
    return trading_time;
  }

  void UpdateNextBeforeCloseMarketEventTimeStamp(TradingTime trading_time,
                                                 TimeStamp current_timestamp) {
    TimeStamp close_market_timestamp = 0;
    boost::posix_time::ptime close_market_pt;
    if (trading_time == TradingTime::kDay) {
      boost::posix_time::ptime pt(
          boost::gregorian::date(1970, 1, 1),
          boost::posix_time::milliseconds(current_timestamp));
      close_market_pt = boost::posix_time::ptime(
          pt.date(), boost::posix_time::time_duration(15, 0, 0));

    } else {
      boost::posix_time::ptime pt(
          boost::gregorian::date(1970, 1, 1),
          boost::posix_time::milliseconds(current_timestamp));
      close_market_pt = boost::posix_time::ptime(
          pt.date(), boost::posix_time::time_duration(23, 30, 0));
    }
    next_event_before_close_market_timestamp_ =
        ptime_to_timestamp(close_market_pt) -
        event_before_close_market_seconds_ * 1000;

    next_event_before_close_market_near_timestamp_ =
        ptime_to_timestamp(close_market_pt) -
        event_before_close_market_near_seconds_ * 1000;
  }

  bool NeedsSendCloseMarketNearEvent(Tick* tick) {
    return next_event_before_close_market_near_timestamp_ != 0 &&
           tick->timestamp > next_event_before_close_market_near_timestamp_;
  }

  std::vector<std::pair<std::shared_ptr<Tick>, int64_t> > tick_containter_;
  std::vector<std::pair<std::shared_ptr<Tick>, int64_t> >::const_iterator it_;
  std::shared_ptr<std::string> instrument_;
  int current_tick_index_ = 0;
  bool* running_;
  MailBox* mail_box_;
  TimeStamp previous_time_stamp_ = 0;
  int event_before_close_market_seconds_ = 0;
  int event_before_close_market_near_seconds_ = 0;
  TimeStamp next_event_before_close_market_timestamp_ = 0;
  TimeStamp next_event_before_close_market_near_timestamp_ = 0;
};

#endif  // BACKTESTING_PRICE_HANDLER_H
