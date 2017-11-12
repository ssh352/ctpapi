#include "gtest/gtest.h"
#include <memory>
#include <unordered_map>
#include "hpt_core/simply_portfolio.h"

namespace {
static std::unordered_map<std::string, std::shared_ptr<OrderField>>
    g_order_containter;

std::shared_ptr<OrderField> MakeOrderField(const std::string& order_id,
                                           const std::string& instrument,
                                           PositionEffect position_effect,
                                           OrderDirection direction,
                                           OrderStatus status,
                                           double price,
                                           double leaves_qty,
                                           double traded_qty,
                                           double qty) {
  auto order = std::make_shared<OrderField>();
  order->order_id = order_id;
  order->position_effect = position_effect;
  order->position_effect_direction = direction;
  order->status = status;
  order->instrument_id = instrument;
  order->input_price = price;
  order->leaves_qty = leaves_qty;
  order->trading_qty = traded_qty;
  order->qty = qty;
  return std::move(order);
}

auto MakeNewOrder(const std::string& order_id,
                  const std::string& instrument,
                  PositionEffect position_effect,
                  OrderDirection direction,
                  double price,
                  double qty) {
  auto order = MakeOrderField(order_id, instrument, position_effect, direction,
                              OrderStatus::kActive, price, qty, 0, qty);
  g_order_containter.insert({order_id, order});
  return std::move(order);
}

auto MakeTradedOrder(const std::string& order_id, double traded_qty) {
  BOOST_ASSERT(g_order_containter.find(order_id) != g_order_containter.end());
  auto order = std::make_shared<OrderField>(*g_order_containter.at(order_id));
  order->status =
      traded_qty == order->qty ? OrderStatus::kAllFilled : OrderStatus::kActive;
  order->trading_qty = traded_qty;
  order->leaves_qty = order->qty - traded_qty;
  g_order_containter[order_id] = order;
  return std::move(order);
}

auto MakeNewOpenOrder(const std::string& order_id,
                      const std::string& instrument,
                      OrderDirection direction,
                      double price,
                      double qty) {
  return MakeNewOrder(order_id, instrument, PositionEffect::kOpen, direction,
                      price, qty);
}

auto MakeNewCloseOrder(
    const std::string& order_id,
    const std::string& instrument,
    OrderDirection direction,
    double price,
    double qty,
    PositionEffect position_effect = PositionEffect::kClose) {
  return MakeNewOrder(order_id, instrument, position_effect, direction, price,
                      qty);
}

auto MakeCanceledOrder(const std::string& order_id) {
  BOOST_ASSERT(g_order_containter.find(order_id) != g_order_containter.end());
  auto order = std::make_shared<OrderField>(*g_order_containter.at(order_id));
  order->status = OrderStatus::kCanceled;
  return std::move(order);
}

TEST(TestSimplePortfolio, CancelOpenOrder) {
  SimplyPortfolio portfolio;
  portfolio.HandleOrder(
      MakeNewOpenOrder("0", "I1", OrderDirection::kBuy, 1.2, 1));
  portfolio.HandleOrder(MakeCanceledOrder("0"));
  EXPECT_EQ(0, portfolio.GetFrozenQty("I1", OrderDirection::kBuy));
  EXPECT_EQ(0, portfolio.GetFrozenQty("I1", OrderDirection::kSell));
  EXPECT_EQ(0, portfolio.GetPositionCloseableQty("I1", OrderDirection::kBuy));
  EXPECT_EQ(0, portfolio.GetPositionCloseableQty("I1", OrderDirection::kSell));
}
}  // namespace
