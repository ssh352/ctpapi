#ifndef FOLLOW_TRADE_TRADE_ORDER_DELEGATE_H
#define FOLLOW_TRADE_TRADE_ORDER_DELEGATE_H
#include "geek_quant/caf_defines.h"

class TradeOrderDelegate {
 public:
  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         double price,
                         int quantity) = 0;

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) = 0;

  virtual void CancelOrder(const std::string& order_no) = 0;


};

#endif  // FOLLOW_TRADE_TRADE_ORDER_DELEGATE_H
