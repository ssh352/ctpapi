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

class AbstractStrategy {
 public:
  virtual void HandleCloseMarket() = 0;
  virtual void HandleTick(const std::shared_ptr<TickData>& tick) = 0;
  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) = 0;
};

class MyStrategy : public AbstractStrategy {
 public:
  MyStrategy(AbstractEventFactory* event_factory,
             std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>
                 cta_signal_container,
             int delayed_input_order_minute,
             int cancel_order_after_minute,
             int backtesting_position_effect);

  virtual void HandleTick(const std::shared_ptr<TickData>& tick) override;

  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) override;

  // Inherited via AbstractStrategy
  virtual void HandleCloseMarket() override;

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
  std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>> keep_memory_;

  std::list<std::shared_ptr<CTATransaction>> delay_input_order_;

  std::set<std::shared_ptr<OrderField>, CompareOrderId> unfill_orders_;

  int delayed_input_order_minute_ = 0;
  int cancel_order_after_minute_ = 0;
  int backtesting_position_effect_ = 0;
};

#endif  // BACKTESTING_STRATEGY_H
