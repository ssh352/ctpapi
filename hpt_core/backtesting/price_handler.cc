#include "price_handler.h"
#include "bft_core/make_message.h"
#include "caf_common/caf_atom_defines.h"
#include "hpt_core/time_util.h"

PriceHandler::PriceHandler(
    const std::string& instrument,
    bool* running,
    bft::ChannelDelegate* mail_box,
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
  *running = !tick_containter_.empty();
}

bool PriceHandler::NeedsSendDaySettleEvent(TimeStamp timestamp) const {
  if (timestamp == 0) {
    return false;
  }
  boost::posix_time::ptime pt(boost::gregorian::date(1970, 1, 1),
                              boost::posix_time::milliseconds(timestamp));
  auto day_close_market_pt = boost::posix_time::ptime(
      pt.date(), boost::posix_time::time_duration(15, 0, 0));
  return abs((pt - day_close_market_pt).total_seconds()) < 30 * 60;
}

bool PriceHandler::NeedsSendCloseMarketNearEvent(Tick* tick) {
  return next_event_before_close_market_near_timestamp_ != 0 &&
         tick->timestamp > next_event_before_close_market_near_timestamp_;
}

void PriceHandler::UpdateNextBeforeCloseMarketEventTimeStamp(
    TradingTime trading_time,
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

TradingTime PriceHandler::GetTradingTime(TimeStamp time_stamp) const {
  boost::posix_time::ptime now(boost::gregorian::date(1970, 1, 1),
                               boost::posix_time::milliseconds(time_stamp));
  boost::posix_time::ptime day(now.date(),
                               boost::posix_time::time_duration(9, 0, 0));
  boost::posix_time::ptime night(now.date(),
                                 boost::posix_time::time_duration(21, 0, 0));

  TradingTime trading_time = TradingTime::kUndefined;
  if (std::abs((now - day).total_seconds()) < 600) {
    trading_time = TradingTime::kDay;
  } else if (std::abs((now - night).total_seconds()) < 600) {
    trading_time = TradingTime::kNight;
  } else {
    // BOOST_ASSERT(false);
  }
  return trading_time;
}

bool PriceHandler::NeedsSeedBeforeCloseMarketEvent(Tick* tick) {
  return next_event_before_close_market_timestamp_ != 0 &&
         tick->timestamp > next_event_before_close_market_timestamp_;
}

bool PriceHandler::IsNextMarketOpen(Tick* tick) const {
  if (previous_time_stamp_ == 0) {
    return true;
  }
  return (tick->timestamp - previous_time_stamp_) / 1000.0 > 3 * 3600 &&
         GetTradingTime(tick->timestamp) != TradingTime::kUndefined;
}

void PriceHandler::StreamNext() {
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
    if (NeedsSendDaySettleEvent(previous_time_stamp_)) {
      mail_box_->Send(bft::MakeMessage(DaySettleAtom::value));
    }
    TradingTime trading_time = GetTradingTime(tick->timestamp);
    BOOST_ASSERT(trading_time != TradingTime::kUndefined);
    mail_box_->Send(bft::MakeMessage(BeforeTradingAtom::value, trading_time));
    UpdateNextBeforeCloseMarketEventTimeStamp(trading_time, tick->timestamp);
  } else if (NeedsSeedBeforeCloseMarketEvent(tick)) {
    mail_box_->Send(bft::MakeMessage(BeforeCloseMarketAtom::value));
    next_event_before_close_market_timestamp_ = 0;
  } else if (NeedsSendCloseMarketNearEvent(tick)) {
    mail_box_->Send(bft::MakeMessage(CloseMarketNearAtom::value));
    next_event_before_close_market_near_timestamp_ = 0;
  } else {
    auto null_deleter = [](Tick* tick) {};
    // event_queue_->push_back(std::make_shared<TickEvent>(std::shared_ptr<Tick>(
    //     &it_->first.get()[current_tick_index_], null_deleter)));
    auto tick_data = std::make_shared<TickData>();
    tick_data->instrument = instrument_;
    tick_data->tick = std::shared_ptr<Tick>(tick, null_deleter);

    mail_box_->Send(bft::MakeMessage(std::move(tick_data)));
    ++current_tick_index_;
  }
  previous_time_stamp_ = tick->timestamp;
}
