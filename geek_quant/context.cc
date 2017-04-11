#include "context.h"

OrderEventType Context::HandlertnOrder(const OrderData& rtn_order) {
  return account_order_mgr_[rtn_order.account_id()].HandleRtnOrder(rtn_order);
}
std::vector<std::string> Context::GetCorrOrderNosWithOrderId(
    const std::string& account_id,
    const std::string& order_id) {
  return account_position_mgr_[account_id].GetCorrOrderNoWithOrderId(order_id);
}

std::vector<OrderQuantity> Context::GetQuantitysWithOrderIds(
    const std::string& account_id,
    std::vector<std::string> orders) {
  return account_position_mgr_[account_id].GetQuantitysWithOrderIds(orders);
}

int Context::GetPositionCloseableQuantity(const std::string& account_id,
                                          const std::string& instrument) {
  return account_position_mgr_[account_id].GetPositionCloseableQuantity(
      instrument);
}

std::vector<OrderQuantity> Context::GetCorrOrderQuantiysWithOrderNo(
    const std::string& account_id,
    const std::string& order_id) {
  return account_position_mgr_[account_id].GetCorrOrderQuantiysWithOrderNo(
      order_id);
}
