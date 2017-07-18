#include "gtest/gtest.h"
#include "follow_strategy_servcie_fixture.h"
#include "follow_strategy_mode/defines.h"

TEST_F(FollowStragetyServiceFixture, Profolio) {
  master_context_.InitPositions(
                         {{"ta1709", OrderDirection::kSell, 4}});
  slave_context_.InitPositions(
                         {{"ta1709", OrderDirection::kSell, 4}});
  master_context_.InitPositions( {{"abc", OrderDirection::kBuy, 4}});
  slave_context_.InitPositions( {{"abc", OrderDirection::kBuy, 4}});
  OpenAndFilledOrder("1", 10, 10, 10);

  PushNewCloseOrderForMaster("2");

  PushNewCloseOrderForSlave("2");

  auto mprofolios = master_context_.GetAccountPortfolios();
  auto sprofolios = slave_context_.GetAccountPortfolios();
  EXPECT_EQ(2, mprofolios.size());
  EXPECT_EQ(2, sprofolios.size());
}
