#include "follow_strategy_mode/close_corr_orders_manager.h"
#include "follow_strategy_mode/order_util.h"

std::vector<std::pair<std::string, int> >
CloseCorrOrdersManager::GetCorrOrderQuantiys(
    const std::string& order_id) const {
  if (close_corr_orders_.find(order_id) == close_corr_orders_.end()) {
    return {};

  }
  return close_corr_orders_.at(order_id);
}

std::vector<std::string> CloseCorrOrdersManager::GetCloseCorrOrderIds(
    const std::string& order_id) const {
  std::vector<std::string> order_ids;
  if (close_corr_orders_.find(order_id) == close_corr_orders_.end()) {
    return order_ids;
  }

  for (auto order_quantity : close_corr_orders_.at(order_id)) {
    order_ids.push_back(order_quantity.first);
  }

  return order_ids;
}

bool CloseCorrOrdersManager::IsNewCloseOrder(const OrderField& rtn_order) const {
  return IsCloseOrder(rtn_order.position_effect) &&
         rtn_order.traded_qty == 0 &&
         close_corr_orders_.find(rtn_order.order_id) ==
             close_corr_orders_.end();
}

void CloseCorrOrdersManager::AddCloseCorrOrders(
    const std::string& order_id,
    std::vector<std::pair<std::string, int> > corr_orders) {
  close_corr_orders_.insert_or_assign(order_id, std::move(corr_orders));
}
