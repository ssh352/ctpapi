#include "context.h"



void Context::HandlertnOrder(const RtnOrderData& rtn_order) {
  account_order_mgr_[rtn_order.account_id].HandleRtnOrder(rtn_order);
}
