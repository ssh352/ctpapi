#include "follow_strategy_mode/unittest/follow_strategy_servcie_sync_fixture.h"

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

TEST_F(FollowStragetyServiceSyncFixture, CloseAllPositionCase2) {
  InitDefaultOrderExchangeId(kSHFEExchangeId);
  service->InitPositions(kSlaveAccountID, {{"abc", OrderDirection::kBuy, 8}});
  OpenAndFilledOrder("1001");

  {
    auto ret = PushNewCloseOrderForMaster("1002", OrderDirection::kSell, 10,
                                          8888.9, PositionEffect::kCloseToday);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ(10, std::get<0>(ret).quantity);
    EXPECT_EQ(PositionEffect::kCloseToday, order_insert.position_effect);

    (void)PushNewCloseOrderForSlave(order_insert.order_no,
                                    OrderDirection::kSell, 10,
                                    PositionEffect::kCloseToday);
  }

  {
    auto ret = PushCloseOrderForMaster("1002", OrderDirection::kSell);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ(8, order_insert.quantity);
    EXPECT_EQ("1003", order_insert.order_no);
    EXPECT_EQ(PositionEffect::kClose, order_insert.position_effect);
    EXPECT_EQ(OrderPriceType::kMarket, order_insert.price_type);
  }
}

TEST_F(FollowStragetyServiceSyncFixture, CancelPartyFillClose) {
  service->InitPositions(kMasterAccountID, {{"abc", OrderDirection::kBuy, 4}});
  OpenAndFilledOrder("1001", 2, 2, 2);
  {
    auto ret = PushNewCloseOrderForMaster("1002", OrderDirection::kSell, 4);
    EXPECT_EQ("", std::get<0>(ret).order_no);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }

  (void)PushCloseOrderForMaster("1002", OrderDirection::kSell, 1, 4);

  {
    auto ret = PushCancelOrderForMaster("1002", OrderDirection::kSell,
                                        PositionEffect::kClose, 1, 4);
    EXPECT_EQ("", std::get<0>(ret).order_no);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }
  {
    auto ret = PushNewCloseOrderForMaster("1003", OrderDirection::kSell, 4);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("1003", order_insert.order_no);
    EXPECT_EQ(1, order_insert.quantity);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }
}
