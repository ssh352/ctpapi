#ifndef BACKTESTING_STRATEGY_H
#define BACKTESTING_STRATEGY_H
#include <boost/format.hpp>
#include "common/api_struct.h"
#include "event.h"
#include "execution_handler.h"
#include "cta_transaction_series_data_base.h"

class InputOrderEvent : public AbstractEvent {
 public:
  InputOrderEvent(AbstractExecutionHandler* execution_handler,
                  std::string instrument,
                  PositionEffect position_effect,
                  OrderDirection order_direction,
                  double price,
                  int qty)
      : execution_handler_(execution_handler),
        instrument_(std::move(instrument)),
        position_effect_(position_effect),
        order_direction_(order_direction),
        price_(price),
        qty_(qty) {}

  virtual void Do() override {
    execution_handler_->HandlerInputOrder(instrument_, position_effect_,
                                          order_direction_, price_, qty_);
  }

 private:
  AbstractExecutionHandler* execution_handler_;
  std::string instrument_;
  PositionEffect position_effect_;
  OrderDirection order_direction_;
  double price_;
  int qty_;
};

class AbstractStrategy {
 public:
  virtual void HandleTick(const std::shared_ptr<TickData>& tick) = 0;
  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) = 0;
};

class MyStrategy : public AbstractStrategy {
 public:
  MyStrategy(const std::string& instrument, AbstractEventFactory* event_factory)
      : event_factory_(event_factory) {
    CTATransactionSeriesDataBase cta_trasaction_series_data_base(
        "d:/cta_tstable.h5");
    keep_memory_ = cta_trasaction_series_data_base.ReadRange(
        str(boost::format("/%s") % instrument),
        boost::posix_time::time_from_string("2016-12-01 09:00:00"),
        boost::posix_time::time_from_string("2017-07-31 15:00:00"));

    auto null_deleter = [](CTATransaction*) {};
    for (auto& item : keep_memory_) {
      for (int i = 0; i < item.second; ++i) {
        transactions_.push_back(std::shared_ptr<CTATransaction>(
            &item.first.get()[i], null_deleter));
      }
    }
    range_beg_it_ = transactions_.begin();
  }

  virtual void HandleTick(const std::shared_ptr<TickData>& tick) override {
    if (range_beg_it_ == transactions_.end()) {
      return;
    }

    auto end_it =
        std::upper_bound(range_beg_it_, transactions_.end(), tick,
                         [](const std::shared_ptr<TickData>& tick,
                            const std::shared_ptr<CTATransaction>& tran) {
                           return tick->tick->timestamp < tran->timestamp;
                         });

    for (auto i = range_beg_it_; i != end_it; ++i) {
      event_factory_->EnqueueInputOrderEvent(
          *tick->instrument,
          (*i)->position_effect == 0 ? PositionEffect::kOpen
                                     : PositionEffect::kClose,
          (*i)->direction == 0 ? OrderDirection::kBuy : OrderDirection::kSell,
          (*i)->price, (*i)->qty);
    }

    range_beg_it_ = end_it;
  }

  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) override {}

 private:
  AbstractEventFactory* event_factory_;
  std::list<std::shared_ptr<CTATransaction>> transactions_;
  std::list<std::shared_ptr<CTATransaction>>::iterator range_beg_it_;
  std::vector<std::pair<std::unique_ptr<CTATransaction[]>, int64_t>>
      keep_memory_;
  
};

#endif  // BACKTESTING_STRATEGY_H
