#include "context.h"



OrderEventType Context::HandlertnOrder(const OrderData& rtn_order) {
  return account_order_mgr_[rtn_order.account_id()].HandleRtnOrder(rtn_order);
}
