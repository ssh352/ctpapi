#include "follow_strategy_mode/orders_context.h"
#include <boost/lexical_cast.hpp>
#include "follow_strategy_mode/order_util.h"

OrdersContext::OrdersContext(std::string account_id)
    : account_id_(std::move(account_id)) {}

OrderEventType OrdersContext::HandleRtnOrder(const OrderData& rtn_order) {
  account_position_mgr_.HandleRtnOrder(rtn_order,
                                       &account_close_corr_orders_mgr_);

  return account_order_mgr_.HandleRtnOrder(rtn_order);
}

void OrdersContext::InitPositions(std::vector<OrderPosition> positions) {
  for (auto pos : positions) {
    std::string order_id =
        "0" + pos.instrument +
        boost::lexical_cast<std::string>(static_cast<int>(pos.order_direction));
    account_position_mgr_.AddQuantity(
        pos.instrument,
        {order_id, pos.order_direction, false, pos.quantity, pos.quantity});
  }
}

boost::optional<OrderData> OrdersContext::GetOrderData(
    const std::string& order_id) const {
  return account_order_mgr_.order_data(order_id);
}

std::vector<AccountPortfolio> OrdersContext::GetAccountPortfolios() const {
  std::vector<AccountPosition> positions = GetAccountPositions();
  std::vector<std::tuple<std::string, OrderDirection, bool, int>> orders =
      GetUnfillOrders();
  std::vector<AccountPortfolio> protfolios;
  for (auto position : positions) {
    auto it =
        std::find_if(protfolios.begin(), protfolios.end(), [=](auto protfolio) {
          return position.instrument == protfolio.instrument &&
                 position.direction == protfolio.direction;
        });
    if (it != protfolios.end()) {
      // Error
    } else {
      protfolios.push_back(
          {position.instrument, position.direction, position.closeable, 0, 0});
    }
  }

  for (auto order : orders) {
    if (std::get<2>(order)) {
      // Open
      auto it = std::find_if(
          protfolios.begin(), protfolios.end(), [=](auto protfolio) {
            return std::get<0>(order) == protfolio.instrument &&
                   std::get<1>(order) == protfolio.direction;
          });
      if (it != protfolios.end()) {
        it->open = std::get<3>(order);
      } else {
        protfolios.push_back(
            {std::get<0>(order), std::get<1>(order), 0, std::get<3>(order), 0});
      }
    } else {
      // Close
      auto it = std::find_if(
          protfolios.begin(), protfolios.end(), [=](auto protfolio) {
            return std::get<0>(order) == protfolio.instrument &&
                   std::get<1>(order) != protfolio.direction;
          });
      if (it != protfolios.end()) {
        it->close = std::get<3>(order);
      } else {
        // Error
      }
    }
  }
  return protfolios;
}

const std::string& OrdersContext::account_id() const {
  return account_id_;
}

std::vector<OrderQuantity> OrdersContext::GetQuantitys(
    std::vector<std::string> orders) const {
  if (orders.empty()) {
    return {};
  }

  return account_position_mgr_.GetQuantitys(
      account_order_mgr_.GetOrderInstrument(orders.at(0)), orders);
}

std::vector<OrderQuantity> OrdersContext::GetQuantitysIf(
    const std::string& instrument,
    std::function<bool(const OrderQuantity&)> cond) const {
  return account_position_mgr_.GetQuantitysIf(instrument, cond);
}

int OrdersContext::GetCloseableQuantityWithOrderDirection(
    const std::string& instrument,
    OrderDirection direction) const {
  return account_position_mgr_.GetCloseableQuantityWithOrderDirection(
      instrument, direction);
}

std::vector<std::pair<std::string, int>> OrdersContext::GetCorrOrderQuantiys(
    const std::string& order_id) {
  return account_close_corr_orders_mgr_.GetCorrOrderQuantiys(order_id);
}

std::vector<std::string> OrdersContext::GetCloseCorrOrderIds(
    const std::string& order_id) {
  return account_close_corr_orders_mgr_.GetCloseCorrOrderIds(order_id);
}

int OrdersContext::ActiveOrderCount(const std::string& instrument,
                                    OrderDirection direction) const {
  return account_order_mgr_.ActiveOrderCount(instrument, direction);
}

std::vector<std::string> OrdersContext::ActiveOrderIds(
    const std::string& instrument,
    OrderDirection direction) const {
  return account_order_mgr_.ActiveOrderIds(instrument, direction);
}

bool OrdersContext::IsActiveOrder(const std::string& order_id) const {
  return account_order_mgr_.IsActiveOrder(order_id);
}

int OrdersContext::GetCloseableQuantity(const std::string& order_id) const {
  return account_position_mgr_.GetCloseableQuantityWithInstrument(order_id);
}

bool OrdersContext::IsOppositeOpen(const std::string& instrument,
                                   OrderDirection direction) const {
  return account_position_mgr_.GetCloseableQuantityWithOrderDirection(
             instrument, OppositeOrderDirection(direction)) != 0;
}

std::vector<AccountPosition> OrdersContext::GetAccountPositions() const {
  return account_position_mgr_.GetAccountPositions();
}

std::vector<std::tuple<std::string, OrderDirection, bool, int>>
OrdersContext::GetUnfillOrders() const {
  return account_order_mgr_.GetUnfillOrders();
}
