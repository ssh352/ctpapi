#include "follow_strategy_mode/context.h"
#include "follow_strategy_mode/order_util.h"

Context::Context(int start_order_id_seq)
    : order_id_mananger_(start_order_id_seq) {}

OrderEventType Context::HandlertnOrder(const OrderData& rtn_order) {
  account_position_mgr_[rtn_order.account_id()].HandleRtnOrder(
      rtn_order, &account_close_corr_orders_mgr_[rtn_order.account_id()]);

  return account_order_mgr_[rtn_order.account_id()].HandleRtnOrder(rtn_order);
}

void Context::InitPositions(const std::string& account_id,
                            std::vector<OrderPosition> positions) {
  // std::string order_id;
  // OrderDirection direction;
  // bool is_today_quantity;
  // int quantity;
  // int closeable_quantity;
  for (auto pos : positions) {
    account_position_mgr_[account_id].AddQuantity(
        pos.instrument,
        {order_id_mananger_.GetOrderId(pos.instrument, pos.order_direction),
         pos.order_direction, false, pos.quantity, pos.quantity});
  }
}

boost::optional<OrderData> Context::GetOrderData(
    const std::string& account_id,
    const std::string& order_no) const {
  if (account_order_mgr_.find(account_id) == account_order_mgr_.end()) {
    return {};
  }
  return account_order_mgr_.at(account_id).order_data(order_no);
}

std::map<std::string, std::vector<AccountPortfolio> >
Context::GetAccountProfolios() const {
  std::map<std::string, std::vector<AccountPortfolio> > account_protfolios;
  return account_protfolios;
}

std::vector<OrderQuantity> Context::GetQuantitys(
    const std::string& account_id,
    std::vector<std::string> orders) const {
  if (orders.empty()) {
    return {};
  }

  if (account_position_mgr_.find(account_id) == account_position_mgr_.end() ||
      account_order_mgr_.find(account_id) == account_order_mgr_.end()) {
    return {};
  }
  return account_position_mgr_.at(account_id)
      .GetQuantitys(
          account_order_mgr_.at(account_id).GetOrderInstrument(orders.at(0)),
          orders);
}

std::vector<OrderQuantity> Context::GetQuantitysIf(
    const std::string& account_id,
    const std::string& instrument,
    std::function<bool(const OrderQuantity&)> cond) const {
  if (account_position_mgr_.find(account_id) == account_position_mgr_.end() ||
      account_order_mgr_.find(account_id) == account_order_mgr_.end()) {
    return {};
  }
  return account_position_mgr_.at(account_id).GetQuantitysIf(instrument, cond);
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
      .GetCloseableQuantityWithInstrument(order_id);
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

std::string Context::GenerateOrderId() {
  return order_id_mananger_.GenerateOrderId();
}

OrderData Context::AdjustOrder(OrderData rtn_order) {
  return order_id_mananger_.AdjustOrder(std::move(rtn_order));
}
