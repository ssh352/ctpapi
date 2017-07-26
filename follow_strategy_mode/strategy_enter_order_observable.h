#include "defines.h"
#ifndef FOLLOW_STRATEGY_MODE_STRATEGY_ENTER_ORDER_OBSERVABLE_H
#define FOLLOW_STRATEGY_MODE_STRATEGY_ENTER_ORDER_OBSERVABLE_H

class StrategyEnterOrderObservable {
 public:
  class Observer {
   public:
    virtual void OpenOrder(const std::string& strategy_id,
                           const std::string& instrument,
                           const std::string& order_no,
                           OrderDirection direction,
                           double price,
                           int quantity) = 0;

    virtual void CloseOrder(const std::string& strategy_id,
                            const std::string& instrument,
                            const std::string& order_no,
                            OrderDirection direction,
                            PositionEffect position_effect,
                            double price,
                            int quantity) = 0;

    virtual void CancelOrder(const std::string& strategy_id,
                             const std::string& order_no) = 0;
  };
  virtual void Subscribe(Observer* observer) = 0;
};

#endif  // FOLLOW_STRATEGY_MODE_STRATEGY_ENTER_ORDER_OBSERVABLE_H
