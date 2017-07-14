#ifndef FOLLOW_TRADE_SERVER_CTP_ENTER_ORDER_PROXY_H
#define FOLLOW_TRADE_SERVER_CTP_ENTER_ORDER_PROXY_H
#include "follow_strategy_mode/enter_order_observer.h"

class CTPEnterOrderProxy : public EnterOrderObserver {
 public:
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
};

#endif  // FOLLOW_TRADE_SERVER_CTP_ENTER_ORDER_PROXY_H
