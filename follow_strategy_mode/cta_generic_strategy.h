#ifndef FOLLOW_STRATEGY_MODE_CTA_GENERIC_STRATEGY_H
#define FOLLOW_STRATEGY_MODE_CTA_GENERIC_STRATEGY_H
#include "strategy_enter_order_observable.h"
#include "enter_order_observer.h"

class CTAGenericStrategy : public StrategyEnterOrderObservable,
                           public EnterOrderObserver {
 public:
  virtual void Subscribe(
      StrategyEnterOrderObservable::Observer* observer) override;

  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         OrderPriceType price_type,
                         double price,
                         int quantity) override;

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          OrderPriceType price_type,
                          double price,
                          int quantity) override;

  virtual void CancelOrder(const std::string& order_no) override;

 private:
  StrategyEnterOrderObservable::Observer* observer_;
  std::string strategy_id_;
};

#endif  // FOLLOW_STRATEGY_MODE_CTA_GENERIC_STRATEGY_H
