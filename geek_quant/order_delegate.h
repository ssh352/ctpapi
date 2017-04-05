#ifndef FOLLOW_TRADE_ORDER_DELEGATE_H
#define FOLLOW_TRADE_ORDER_DELEGATE_H

class OrderDelegate {
public:
  virtual void EnterOrder() = 0;

  virtual void CancelOrder() = 0;
};

#endif // FOLLOW_TRADE_ORDER_DELEGATE_H



