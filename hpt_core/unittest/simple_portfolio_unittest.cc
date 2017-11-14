#include "gtest/gtest.h"
#include <memory>
#include <unordered_map>
#include "hpt_core/simply_portfolio.h"
#include "order_util.h"

namespace {

class SimplyPortfolioFixture : public testing::Test {
 public:
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
    order->direction = direction;
    order->position_effect_direction =
        AdjustDirectionByPositionEffect(position_effect, direction);
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
    auto order =
        MakeOrderField(order_id, instrument, position_effect, direction,
                       OrderStatus::kActive, price, qty, 0, qty);
    order_containter_.insert({order_id, order});
    return std::move(order);
  }

  auto MakeTradedOrder(const std::string& order_id, double traded_qty) {
    BOOST_ASSERT(order_containter_.find(order_id) != order_containter_.end());
    auto order = std::make_shared<OrderField>(*order_containter_.at(order_id));
    order->status = traded_qty == order->qty ? OrderStatus::kAllFilled
                                             : OrderStatus::kActive;
    order->trading_qty = traded_qty;
    order->leaves_qty = order->qty - traded_qty;
    order_containter_[order_id] = order;
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
    BOOST_ASSERT(order_containter_.find(order_id) != order_containter_.end());
    auto order = std::make_shared<OrderField>(*order_containter_.at(order_id));
    order->status = OrderStatus::kCanceled;
    return std::move(order);
  }

  auto MakeActionOrder(const std::string& order_id,
                       double new_price,
                       int new_qty) {
    BOOST_ASSERT(order_containter_.find(order_id) != order_containter_.end());
    auto order = std::make_shared<OrderField>(*order_containter_.at(order_id));
    order->leaves_qty = new_qty;
    order->qty = new_qty;
    order->input_price = new_price;
    order_containter_[order_id] = order;
    return std::move(order);
  }

 protected:
  std::unordered_map<std::string, std::shared_ptr<OrderField>>
      order_containter_;
};

TEST_F(SimplyPortfolioFixture, CancelOpenOrder) {
  SimplyPortfolio portfolio;
  portfolio.HandleOrder(
      MakeNewOpenOrder("0", "I1", OrderDirection::kBuy, 1.2, 1));
  portfolio.HandleOrder(MakeCanceledOrder("0"));
  EXPECT_EQ(0, portfolio.GetFrozenQty("I1", OrderDirection::kBuy));
  EXPECT_EQ(0, portfolio.GetFrozenQty("I1", OrderDirection::kSell));
  EXPECT_EQ(0, portfolio.GetPositionCloseableQty("I1", OrderDirection::kBuy));
  EXPECT_EQ(0, portfolio.GetPositionCloseableQty("I1", OrderDirection::kSell));
}

TEST_F(SimplyPortfolioFixture, ActionOpenOrderWithQty) {
  SimplyPortfolio portfolio;
  portfolio.HandleOrder(
      MakeNewOpenOrder("0", "I1", OrderDirection::kBuy, 1.2, 10));
  portfolio.HandleOrder(MakeActionOrder("0", 1.2, 8));

  EXPECT_EQ(8, portfolio.UnfillOpenQty("I1", OrderDirection::kBuy));
}

TEST_F(SimplyPortfolioFixture, ActionCloseOrderWithQty) {
  SimplyPortfolio portfolio;
  portfolio.HandleOrder(
      MakeNewOpenOrder("0", "I1", OrderDirection::kBuy, 1.2, 10));
  portfolio.HandleOrder(MakeTradedOrder("0", 10));

  portfolio.HandleOrder(
      MakeNewCloseOrder("1", "I1", OrderDirection::kSell, 1.2, 10));
  portfolio.HandleOrder(MakeActionOrder("1", 1.2, 8));
  EXPECT_EQ(8, portfolio.GetFrozenQty("I1", OrderDirection::kBuy));
  EXPECT_EQ(2, portfolio.GetPositionCloseableQty("I1", OrderDirection::kBuy));
}
}  // namespace
