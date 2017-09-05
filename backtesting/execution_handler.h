#ifndef BACKTESTING_EXECUTION_HANDLER_H
#define BACKTESTING_EXECUTION_HANDLER_H
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assert.hpp>
#include <set>
#include "common/api_struct.h"
#include "backtesting/tick_series_data_base.h"
#include "backtesting/mail_box.h"

struct LimitOrder {
  std::string instrument;
  std::string order_id;
  OrderDirection direction;
  PositionEffect position_effect;
  double price;
  int qty;
};

template <class KeyCompare>
class ComparePrice {
 public:
  using is_transparent = void;

  bool operator()(const LimitOrder& l, const LimitOrder& r) const {
    return compare_(l.price, r.price);
  }

  bool operator()(double price, const LimitOrder& r) const {
    return compare_(price, r.price);
  }

  bool operator()(const LimitOrder& l, double price) const {
    return compare_(l.price, price);
  }

 private:
  KeyCompare compare_;
};

class AbstractExecutionHandler {
 public:
  virtual void HandleTick(const std::shared_ptr<TickData>& tick) = 0;

  virtual void HandlerInputOrder(const InputOrder& input_order) = 0;

  virtual void HandleCancelOrder(const CancelOrder& cancel_order) = 0;
};

class SimulatedExecutionHandler : public AbstractExecutionHandler {
 public:
  SimulatedExecutionHandler(MailBox* mail_box);

  virtual void HandleTick(const std::shared_ptr<TickData>& tick) override;

  virtual void HandlerInputOrder(const InputOrder& input_order) override;

  virtual void HandleCancelOrder(const CancelOrder& cancel_order) override;

  void EnqueueRtnOrderEvent(const std::string& instrument,
                            const std::string& order_id,
                            OrderDirection direction,
                            PositionEffect position_effect,
                            double input_price,
                            double price,
                            int qty);

  void EnqueueCancelOrderEvent(const LimitOrder& limit_order);

 private:
  double GetFillPrice() const;
  std::shared_ptr<Tick> current_tick_;
  MailBox* mail_box_;
  uint64_t order_id_seq_ = 0.0;
  std::multiset<LimitOrder, ComparePrice<std::greater<double>>>
      long_limit_orders_;
  std::multiset<LimitOrder, ComparePrice<std::less<double>>>
      short_limit_orders_;
};

#endif  // BACKTESTING_EXECUTION_HANDLER_H
