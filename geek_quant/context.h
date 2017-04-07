#ifndef FOLLOW_TRADE_CONTEXT_H
#define FOLLOW_TRADE_CONTEXT_H
#include "geek_quant/caf_defines.h" 
#include "geek_quant/order_manager.h"

class Context {
 public:
  void HandlertnOrder(
                      const RtnOrderData& rtn_order);

 private:
  std::map<std::string, OrderManager> account_order_mgr_;
};

#endif  // FOLLOW_TRADE_CONTEXT_H
