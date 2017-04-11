#ifndef FOLLOW_TRADE_CONTEXT_H
#define FOLLOW_TRADE_CONTEXT_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/order_manager.h"
#include "geek_quant/position_manager.h"

class Context {
 public:
  OrderEventType HandlertnOrder(const OrderData& rtn_order);

  std::vector<std::string> GetCorrOrderNosWithOrderId(
      const std::string& account_id,
      const std::string& order_id);

  std::vector<OrderQuantity> GetQuantitysWithOrderIds(
      const std::string& account_id,
      std::vector<std::string> order_ids);

  int GetPositionCloseableQuantity(const std::string& account_id,
                                   const std::string& instrument);

  std::vector<OrderQuantity> GetCorrOrderQuantiysWithOrderNo(
      const std::string& account_id,
      const std::string& order_id);

 private:
  std::map<std::string, OrderManager> account_order_mgr_;
  std::map<std::string, PositionManager> account_position_mgr_;
};

#endif  // FOLLOW_TRADE_CONTEXT_H
