#ifndef BACKTESTING_STRATEGY_H
#define BACKTESTING_STRATEGY_H
#include <boost/format.hpp>
#include <list>
#include <set>
#include "common/api_struct.h"
#include "event.h"
#include "execution_handler.h"
#include "event_factory.h"

class AbstractExecutionHandler;
class InputOrderEvent : public AbstractEvent {
 public:
  InputOrderEvent(AbstractExecutionHandler* execution_handler,
                  std::string instrument,
                  PositionEffect position_effect,
                  OrderDirection order_direction,
                  double price,
                  int qty,
                  TimeStamp timestamp)
      : execution_handler_(execution_handler),
        instrument_(std::move(instrument)),
        position_effect_(position_effect),
        order_direction_(order_direction),
        price_(price),
        qty_(qty),
        timestamp_(timestamp) {}

  virtual void Do() override {
    execution_handler_->HandlerInputOrder(instrument_, position_effect_,
                                          order_direction_, price_, qty_,
                                          timestamp_);
  }

 private:
  AbstractExecutionHandler* execution_handler_;
  std::string instrument_;
  PositionEffect position_effect_;
  OrderDirection order_direction_;
  double price_;
  int qty_;
  TimeStamp timestamp_;
};

class AbstractStrategy {
 public:
  virtual void HandleCloseMarket() = 0;
  virtual void HandleTick(const std::shared_ptr<TickData>& tick) = 0;
  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) = 0;
};

class MyStrategy : public AbstractStrategy {
 public:
  MyStrategy(AbstractEventFactory* event_factory,
             std::vector<std::pair<std::unique_ptr<CTATransaction[]>, int64_t>>
                 cta_signal_container,
             int delayed_input_order_minute,
             int cancel_order_after_minute);

  virtual void HandleTick(const std::shared_ptr<TickData>& tick) override;

  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) override;

  // Inherited via AbstractStrategy
  virtual void HandleCloseMarket();

 private:
  class CompareOrderId {
   public:
    using is_transparent = void;
    bool operator()(const std::shared_ptr<OrderField>& l,
                    const std::shared_ptr<OrderField>& r) const {
      return l->order_id < r->order_id;
    }

    bool operator()(const std::string& order_id,
                    const std::shared_ptr<OrderField>& r) const {
      return order_id < r->order_id;
    }

    bool operator()(const std::shared_ptr<OrderField>& l,
                    const std::string& order_id) const {
      return l->order_id < order_id;
    }
    bool operator()(const std::shared_ptr<OrderField>& l,
                    TimeStamp timestamp) const {
      return l->input_timestamp < timestamp;
    }
    bool operator()(TimeStamp timestamp,
                    const std::shared_ptr<OrderField>& l) const {
      return timestamp < l->input_timestamp;
    }
  };
  AbstractEventFactory* event_factory_;
  std::list<std::shared_ptr<CTATransaction>> transactions_;
  std::list<std::shared_ptr<CTATransaction>>::iterator range_beg_it_;
  std::vector<std::pair<std::unique_ptr<CTATransaction[]>, int64_t>>
      keep_memory_;

  std::list<std::shared_ptr<CTATransaction>> delay_input_order_;

  std::set<std::shared_ptr<OrderField>, CompareOrderId> unfill_orders_;

  int delayed_input_order_minute_ = 0;
  int cancel_order_after_minute_ = 0;
};

#endif  // BACKTESTING_STRATEGY_H
