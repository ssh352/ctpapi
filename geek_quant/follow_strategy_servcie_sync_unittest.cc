#include "geek_quant/follow_strategy_servcie_sync_fixture.h"

TEST_F(FollowStragetyServiceSyncFixture, CloseYesterdayPosition) {
  service->InitPositions(kMasterAccountID, {{"abc", OrderDirection::kBuy, 10}});
  service->InitPositions(kSlaveAccountID, {{"abc", OrderDirection::kBuy, 10}});

  auto ret = PushNewCloseOrderForMaster("1", OrderDirection::kSell, 10);
  auto order_insert = std::get<0>(ret);
  EXPECT_EQ(order_insert.direction, OrderDirection::kSell);
  EXPECT_EQ(order_insert.position_effect, PositionEffect::kClose);
  EXPECT_EQ("1001", order_insert.order_no);
  EXPECT_EQ(0, std::get<1>(ret).size());
}

TEST_F(FollowStragetyServiceSyncFixture, ClosePositionForSHFECase1) {
  InitDefaultOrderExchangeId(kSHFEExchangeId);
  service->InitPositions(kMasterAccountID, {{"abc", OrderDirection::kBuy, 10}});
  service->InitPositions(kSlaveAccountID, {{"abc", OrderDirection::kBuy, 10}});
  OpenAndFilledOrder("1001");

  auto ret = PushNewCloseOrderForMaster("1002", OrderDirection::kSell, 10,
                                        8888.9, PositionEffect::kCloseToday);
  auto order_insert = std::get<0>(ret);
  EXPECT_EQ(order_insert.direction, OrderDirection::kSell);
  EXPECT_EQ(order_insert.position_effect, PositionEffect::kCloseToday);
  EXPECT_EQ("1002", order_insert.order_no);
  EXPECT_EQ(0, std::get<1>(ret).size());
}

TEST_F(FollowStragetyServiceSyncFixture, ClosePositionForSHFECase2) {
  InitDefaultOrderExchangeId(kSHFEExchangeId);
  service->InitPositions(kMasterAccountID, {{"abc", OrderDirection::kBuy, 10}});
  service->InitPositions(kSlaveAccountID, {{"abc", OrderDirection::kBuy, 10}});
  OpenAndFilledOrder("1001");

  auto ret = PushNewCloseOrderForMaster("1002", OrderDirection::kSell, 10,
                                        8888.9, PositionEffect::kClose);
  auto order_insert = std::get<0>(ret);
  EXPECT_EQ(order_insert.direction, OrderDirection::kSell);
  EXPECT_EQ(order_insert.position_effect, PositionEffect::kClose);
  EXPECT_EQ("1002", order_insert.order_no);
  EXPECT_EQ(0, std::get<1>(ret).size());
}

TEST_F(FollowStragetyServiceSyncFixture, CloseAllPositionCase1) {
  service->InitPositions(kSlaveAccountID, {{"abc", OrderDirection::kBuy, 8}});
  OpenAndFilledOrder("1001");

  {
    auto ret = PushNewCloseOrderForMaster("1002", OrderDirection::kSell, 10,
                                          8888.9, PositionEffect::kClose);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ(10, std::get<0>(ret).quantity);

    (void)PushNewCloseOrderForSlave(order_insert.order_no,
                                    OrderDirection::kSell, 10);
  }

  {
    auto ret = PushCloseOrderForMaster("1002", OrderDirection::kSell);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ(8, order_insert.quantity);
    EXPECT_EQ("1003", order_insert.order_no);
  }
}
