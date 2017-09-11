#include "common/api_struct.h"
#ifndef follow_strategy_STRATEGY_ENTER_ORDER_OBSERVABLE_H
#define follow_strategy_STRATEGY_ENTER_ORDER_OBSERVABLE_H

class StrategyEnterOrderObservable {
 public:
  class Observer {
   public:
    virtual void OpenOrder(const std::string& strategy_id,
                           const std::string& instrument,
                           const std::string& order_id,
                           OrderDirection direction,
                           double price,
                           int quantity) = 0;

    virtual void CloseOrder(const std::string& strategy_id,
                            const std::string& instrument,
                            const std::string& order_id,
                            OrderDirection direction,
                            PositionEffect position_effect,
                            double price,
                            int quantity) = 0;

    virtual void CancelOrder(const std::string& strategy_id,
                             const std::string& order_id) = 0;
  };
  virtual void Subscribe(Observer* observer) = 0;
};

#endif  // follow_strategy_STRATEGY_ENTER_ORDER_OBSERVABLE_H
