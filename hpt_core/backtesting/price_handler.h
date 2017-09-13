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
      std::vector<std::pair<std::shared_ptr<Tick>, int64_t> > tick_containter)
      : instrument_(std::make_shared<std::string>(instrument)),
        running_(running),
        mail_box_(mail_box),
        tick_containter_(std::move(tick_containter)) {
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
      mail_box_->Send(BeforeTradingAtom::value,
                      GetTradingTime(tick->timestamp));
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
    current_time_stamp_ = tick->timestamp;
  }

 private:
  bool IsNextMarketOpen(Tick* tick) const {
    if (current_time_stamp_ == 0) {
      return true;
    }
    return (tick->timestamp - current_time_stamp_) / 1000.0 > 3 * 3600;
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

  std::vector<std::pair<std::shared_ptr<Tick>, int64_t> > tick_containter_;
  std::vector<std::pair<std::shared_ptr<Tick>, int64_t> >::const_iterator it_;
  std::shared_ptr<std::string> instrument_;
  int current_tick_index_ = 0;
  bool* running_;
  MailBox* mail_box_;
  TimeStamp current_time_stamp_ = 0;
};

#endif  // BACKTESTING_PRICE_HANDLER_H
