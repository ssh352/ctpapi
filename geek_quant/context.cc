#include "context.h"

OrderEventType Context::HandlertnOrder(const OrderData& rtn_order) {
  account_position_mgr_[rtn_order.account_id()].HandleRtnOrder(
      rtn_order, &account_close_corr_orders_mgr_[rtn_order.account_id()]);

  return account_order_mgr_[rtn_order.account_id()].HandleRtnOrder(rtn_order);
}

std::vector<OrderQuantity> Context::GetQuantitys(
    const std::string& account_id,
    std::vector<std::string> orders) {
  if (orders.empty()) {
    return {};
  }
  return account_position_mgr_[account_id].GetQuantitys(
      account_order_mgr_[account_id].GetOrderInstrument(orders.at(0)), orders);
}

int Context::GetPositionCloseableQuantity(const std::string& account_id,
                                          const std::string& instrument,
                                          OrderDirection direction) {
  return account_position_mgr_[account_id].GetPositionCloseableQuantity(
      instrument, direction);
}

std::vector<std::pair<std::string, int> > Context::GetCorrOrderQuantiys(
    const std::string& account_id,
    const std::string& order_id) {
  return account_close_corr_orders_mgr_[account_id].GetCorrOrderQuantiys(
      order_id);
}

std::vector<std::string> Context::GetCloseCorrOrderIds(
    const std::string& account_id,
    const std::string& order_id) {
  return account_close_corr_orders_mgr_[account_id].GetCloseCorrOrderIds(
      order_id);
}

bool Context::IsUnfillOrder(const std::string& slave_account_id,
                            const std::string& order_id) const {
  if (account_order_mgr_.find(slave_account_id) == account_order_mgr_.end()) {
    return false;
  }
  return account_order_mgr_.at(slave_account_id).IsUnfillOrder(order_id);
}
