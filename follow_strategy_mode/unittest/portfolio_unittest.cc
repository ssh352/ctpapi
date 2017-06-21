#include "follow_strategy_mode/unittest/follow_strategy_servcie_sync_fixture.h"

TEST_F(FollowStragetyServiceSyncFixture, Profolio) {
  service->InitPositions(kMasterAccountID,
                         {{"1", "ta1709", OrderDirection::kSell, 4}});
  service->InitPositions(kSlaveAccountID,
                         {{"1001", "ta1709", OrderDirection::kSell, 4}});
  service->InitPositions(kMasterAccountID, {{"2", "abc", OrderDirection::kBuy, 4}});
  service->InitPositions(kSlaveAccountID, {{"1002", "abc", OrderDirection::kBuy, 4}});
  OpenAndFilledOrder("1001", 10, 10, 10);

  PushNewCloseOrderForMaster("1002");

  PushNewCloseOrderForSlave("1002");

  auto mprofolios = service->context().GetAccountPortfolios(kMasterAccountID);
  auto sprofolios = service->context().GetAccountPortfolios(kSlaveAccountID);
  EXPECT_EQ(2, mprofolios.size());
  EXPECT_EQ(2, sprofolios.size());
}
