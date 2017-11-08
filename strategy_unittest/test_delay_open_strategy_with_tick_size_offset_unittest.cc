
#include "gtest/gtest.h"
#include "strategy_fixture.h"

#include "follow_strategy/delayed_open_strategy_ex.h"
#include "unittest_helper.h"
#include "default_delay_open_strategy_ex_fixture.h"

const static std::string master_account_id = "master";
const static std::string slave_account_id = "slave";
const static int delayed_open_after_seconds = 60;
const static std::string defalut_instrument_id = "I1";
const static int default_market_tick_qty = 10;

class TestDelayOpenStrategyWithTickSizeOffset
    : public DefaultDelayedOpenStrategyExFixture {
 protected:
  TestDelayOpenStrategyWithTickSizeOffset()
      : DefaultDelayedOpenStrategyExFixture(master_account_id,
                                            slave_account_id,
                                            defalut_instrument_id,
                                            default_market_tick_qty) {}
  virtual void SetUp() override {
    DelayedOpenStrategyEx::StrategyParam param{delayed_open_after_seconds, 5.0};
    CreateStrategy<DelayOpenStrategyAgent<UnittestMailBox> >(std::move(param));
  }
};


TEST_F(TestDelayOpenStrategyWithTickSizeOffset, OpenWithTickSizeOffset) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 15.0, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  ElapseSeconds(1);
  MarketTick(9.0);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(OrderDirection::kBuy, input_order->direction);
  EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect);
  EXPECT_EQ(10.0, input_order->price);
  EXPECT_EQ(10, input_order->qty);
  EXPECT_EQ("0", input_order->order_id);
}
