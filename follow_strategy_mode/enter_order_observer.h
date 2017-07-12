#ifndef FOLLOW_STRATEGY_MODE_ENTER_ORDER_OBSERVER_H
#define FOLLOW_STRATEGY_MODE_ENTER_ORDER_OBSERVER_H

class EnterOrderObserver {
 public:
  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         OrderPriceType price_type,
                         double price,
                         int quantity) = 0;

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          OrderPriceType price_type,
                          double price,
                          int quantity) = 0;

  virtual void CancelOrder(const std::string& order_no) = 0;
};

#endif  // FOLLOW_STRATEGY_MODE_ENTER_ORDER_OBSERVER_H
