#include "gtest/gtest.h"
//#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "follow_strategy/optimal_open_price_strategy.h"
#include "unittest_helper.h"
#include "default_delay_open_strategy_ex_fixture.h"
#include "follow_strategy/delay_open_strategy_agent.h"

const static std::string master_account_id = "master";
const static std::string slave_account_id = "slave";
const static int delayed_open_after_seconds = 0;
const static int wait_optimal_open_price_order_fill_seconds = 60;
const static std::string defalut_instrument_id = "ut1001";
const static std::string default_product_code = "ut";
const static int default_market_tick_qty = 10;

class TestOptimalOpenPriceWithOutDelayOpenSeconds
    : public DefaultDelayedOpenStrategyExFixture {
 protected:
  TestOptimalOpenPriceWithOutDelayOpenSeconds()
      : DefaultDelayedOpenStrategyExFixture(master_account_id,
                                            slave_account_id,
                                            defalut_instrument_id,
                                            default_market_tick_qty) {}
  virtual void SetUp() override {
    std::unordered_map<std::string, OptimalOpenPriceStrategy::StrategyParam>
        params;
    params.insert(
        {"i", OptimalOpenPriceStrategy::StrategyParam{
                  0, wait_optimal_open_price_order_fill_seconds, 0.1}});
    boost::property_tree::ptree child;
    child.put("DelayOpenOrderAfterSeconds", 0);
    child.put("WaitOptimalOpenPriceFillSeconds",
              wait_optimal_open_price_order_fill_seconds);
    child.put("OptimizeTickSize", 1);
    pt_.add_child("default", child);
    boost::property_tree::ptree product_info_pt;
    {
      boost::property_tree::ptree child;
      child.put("TickSize", 0.1);
      child.put("Exchange", "test");
      product_info_pt.add_child(default_product_code, child);
    }
    product_info_mananger_.Load(product_info_pt);
    CreateStrategy<
        DelayOpenStrategyAgent<UnittestMailBox, OptimalOpenPriceStrategy> >(
        &pt_, &product_info_mananger_, &log_);
    Send(ExchangeStatus::kContinous);
  }

 private:
  boost::property_tree::ptree pt_;
  ProductInfoMananger product_info_mananger_;
};

TEST_F(TestOptimalOpenPriceWithOutDelayOpenSeconds, Open_Order) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(OrderDirection::kBuy, input_order->direction);
  EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect);
  EXPECT_EQ(1.0, input_order->price);
  EXPECT_EQ(10, input_order->qty);
  EXPECT_EQ("0", input_order->order_id);
}

TEST_F(TestOptimalOpenPriceWithOutDelayOpenSeconds,
       DontActionOrderBecauseAllFill) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  TradedOrder("0", 10);
  Clear();
  ElapseSeconds(wait_optimal_open_price_order_fill_seconds);
  MarketTick(1.0);
  ASSERT_FALSE(PopupRntOrder<OrderAction>());
}

TEST_F(TestOptimalOpenPriceWithOutDelayOpenSeconds, CancelOrderOptimalOrder) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  MasterNewCloseOrder("1", OrderDirection::kSell, 1.2, 10, 10, 10);
  Clear();
  MasterTradedOrder("1", 10, 0, 0);

  auto cancel_order = PopupRntOrder<CancelOrder>();
  EXPECT_EQ("0", cancel_order->order_id);

  CancelOrderWithOrderNo("0");
  ElapseSeconds(wait_optimal_open_price_order_fill_seconds);
  MarketTick(1.0);
  ASSERT_FALSE(PopupRntOrder<OrderAction>());
}

TEST_F(TestOptimalOpenPriceWithOutDelayOpenSeconds, ActionOrder) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  Clear();
  ElapseSeconds(wait_optimal_open_price_order_fill_seconds);
  MarketTick(1.0);
  auto action_order = PopupRntOrder<OrderAction>();
  ASSERT_TRUE(action_order);
  EXPECT_EQ(1.1, action_order->new_price);
  EXPECT_EQ(1.0, action_order->old_price);
  EXPECT_EQ(0, action_order->old_qty);
  EXPECT_EQ(0, action_order->new_qty);
  EXPECT_EQ("0", action_order->order_id);
}

TEST_F(TestOptimalOpenPriceWithOutDelayOpenSeconds, PartiallyActionOrder) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  TradedOrder("0", 9);
  Clear();
  ElapseSeconds(wait_optimal_open_price_order_fill_seconds);
  MarketTick(1.0);
  auto action_order = PopupRntOrder<OrderAction>();
  ASSERT_TRUE(action_order);
  EXPECT_EQ(1.1, action_order->new_price);
  EXPECT_EQ(1.0, action_order->old_price);
  EXPECT_EQ(0, action_order->old_qty);
  EXPECT_EQ(0, action_order->new_qty);
  EXPECT_EQ("0", action_order->order_id);
}
