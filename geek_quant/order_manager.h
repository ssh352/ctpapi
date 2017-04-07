#ifndef FOLLOW_TRADE_ORDER_MANAGER_H
#define FOLLOW_TRADE_ORDER_MANAGER_H
#include "geek_quant/caf_defines.h"

class OrderManager {
public:

  void HandleRtnOrder(const RtnOrderData& rtn_order);
  void AfterHandleRtnOrder();
};


#endif // FOLLOW_TRADE_ORDER_MANAGER_H



