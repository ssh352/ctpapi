#ifndef follow_strategy_ENTER_ORDER_OBSERVER_H
#define follow_strategy_ENTER_ORDER_OBSERVER_H
#include "common/api_struct.h"

class EnterOrderObserver {
 public:
  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_id,
                         OrderDirection direction,
                         double price,
                         int quantity) = 0;

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_id,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) = 0;

  virtual void CancelOrder(const std::string& order_id) = 0;
};

#endif  // follow_strategy_ENTER_ORDER_OBSERVER_H
