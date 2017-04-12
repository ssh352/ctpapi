#include "geek_quant/context.h"
#include "geek_quant/order_util.h"

OrderEventType Context::HandlertnOrder(const OrderData& rtn_order) {
  account_position_mgr_[rtn_order.account_id()].HandleRtnOrder(
      rtn_order, &account_close_corr_orders_mgr_[rtn_order.account_id()]);

  return account_order_mgr_[rtn_order.account_id()].HandleRtnOrder(rtn_order);
}

std::vector<OrderQuantity> Context::GetQuantitys(
    const std::string& account_id,
    std::vector<std::string> orders) const {
  if (orders.empty()) {
    return {};
  }

  if (account_position_mgr_.find(account_id) == account_position_mgr_.end() ||
      account_order_mgr_.find(account_id) == account_order_mgr_.end()) {
    return{};
  }
  return account_position_mgr_.at(account_id).GetQuantitys(
      account_order_mgr_.at(account_id).GetOrderInstrument(orders.at(0)), orders);
}

int Context::GetCloseableQuantityWithOrderDirection(
    const std::string& account_id,
    const std::string& instrument,
    OrderDirection direction) const {
  if (account_position_mgr_.find(account_id) == account_position_mgr_.end()) {
    return 0;
  }
  return account_position_mgr_.at(account_id)
      .GetCloseableQuantityWithOrderDirection(instrument, direction);
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

int Context::ActiveOrderCount(const std::string& account_id,
                              const std::string& instrument,
                              OrderDirection direction) const {
  if (account_position_mgr_.find(account_id) == account_position_mgr_.end()) {
    return 0;
  }
  return account_order_mgr_.at(account_id)
      .ActiveOrderCount(instrument, direction);
}

std::vector<std::string> Context::ActiveOrderIds(
    const std::string& account_id,
    const std::string& instrument,
    OrderDirection direction) const {
  if (account_position_mgr_.find(account_id) == account_position_mgr_.end()) {
    return {};
  }
  return account_order_mgr_.at(account_id)
      .ActiveOrderIds(instrument, direction);
}

bool Context::IsActiveOrder(const std::string& slave_account_id,
                            const std::string& order_id) const {
  if (account_order_mgr_.find(slave_account_id) == account_order_mgr_.end()) {
    return false;
  }
  return account_order_mgr_.at(slave_account_id).IsActiveOrder(order_id);
}

int Context::GetCloseableQuantity(const std::string& account_id,
                                  const std::string& order_id) const {
  // account_close_corr_orders_mgr_
  if (account_position_mgr_.find(account_id) == account_position_mgr_.end()) {
    return 0;
  }

  return account_position_mgr_.at(account_id)
      .GetCloseableQuantityWithInstrument(
          account_order_mgr_.at(account_id).GetOrderInstrument(order_id),
          order_id);
}

bool Context::IsOppositeOpen(const std::string& account_id,
                             const std::string& instrument,
                             OrderDirection direction) const {
  if (account_position_mgr_.find(account_id) == account_position_mgr_.end()) {
    return false;
  }
  return account_position_mgr_.at(account_id)
             .GetCloseableQuantityWithOrderDirection(
                 instrument, OppositeOrderDirection(direction)) != 0;
}

