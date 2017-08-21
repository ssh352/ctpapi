#ifndef BACKTESTING_PRICE_HANDLER_H
#define BACKTESTING_PRICE_HANDLER_H
#include <boost/format.hpp>
#include "event.h"
#include "tick_series_data_base.h"

class PriceHandler {
 public:
  PriceHandler(const std::string& market,
               const std::string& instrument,
               bool* running,
               AbstractEventFactory* tick_event_factory)
      : instrument_(std::make_shared<std::string>(instrument)),
        running_(running),
        tick_event_factory_(tick_event_factory) {
    TickSeriesDataBase ts_db("d:/ts_futures.h5");
    tick_containter_ = ts_db.ReadRange(
        str(boost::format("/%s/%s") % market % instrument),
        boost::posix_time::time_from_string("2016-12-01 09:00:00"),
        boost::posix_time::time_from_string("2017-07-31 15:00:00"));

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

    auto null_deleter = [](Tick* tick) {};

    // event_queue_->push_back(std::make_shared<TickEvent>(std::shared_ptr<Tick>(
    //     &it_->first.get()[current_tick_index_], null_deleter)));
    auto tick_data = std::make_shared<TickData>();
    tick_data->instrument = instrument_;
    tick_data->tick = std::shared_ptr<Tick>(
        &it_->first.get()[current_tick_index_], null_deleter);

    tick_event_factory_->EnqueueTickEvent(std::move(tick_data));
    ++current_tick_index_;
  }

  std::vector<std::pair<std::unique_ptr<Tick[]>, int64_t> > tick_containter_;
  std::vector<std::pair<std::unique_ptr<Tick[]>, int64_t> >::const_iterator it_;
  std::shared_ptr<std::string> instrument_;
  int current_tick_index_ = 0;
  bool* running_;
  AbstractEventFactory* tick_event_factory_;
};

#endif  // BACKTESTING_PRICE_HANDLER_H
