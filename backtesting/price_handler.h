#ifndef BACKTESTING_PRICE_HANDLER_H
#define BACKTESTING_PRICE_HANDLER_H
#include <boost/format.hpp>
#include "mail_box.h"
#include "event.h"
#include "tick_series_data_base.h"

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

    auto null_deleter = [](Tick* tick) {};

    // event_queue_->push_back(std::make_shared<TickEvent>(std::shared_ptr<Tick>(
    //     &it_->first.get()[current_tick_index_], null_deleter)));
    auto tick_data = std::make_shared<TickData>();
    tick_data->instrument = instrument_;
    tick_data->tick = std::shared_ptr<Tick>(
        &it_->first.get()[current_tick_index_], null_deleter);

    mail_box_->Send(std::move(tick_data));
    ++current_tick_index_;
  }

  std::vector<std::pair<std::shared_ptr<Tick>, int64_t> > tick_containter_;
  std::vector<std::pair<std::shared_ptr<Tick>, int64_t> >::const_iterator it_;
  std::shared_ptr<std::string> instrument_;
  int current_tick_index_ = 0;
  bool* running_;
  MailBox* mail_box_;
};

#endif  // BACKTESTING_PRICE_HANDLER_H
