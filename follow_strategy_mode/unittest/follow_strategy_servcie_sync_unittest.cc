#include "follow_strategy_servcie_fixture.h"
#include "gtest/gtest.h"

TEST_F(FollowStragetyServiceFixture, CloseYesterdayPosition) {
  master_context_.InitPositions({{"abc", OrderDirection::kBuy, 10}});
  slave_context_.InitPositions({{"abc", OrderDirection::kBuy, 10}});

  auto ret = PushNewCloseOrderForMaster("1", OrderDirection::kSell, 10);
  auto order_insert = std::get<0>(ret);
  EXPECT_EQ(order_insert.direction, OrderDirection::kSell);
  EXPECT_EQ(order_insert.position_effect, PositionEffect::kClose);
  EXPECT_EQ("1", order_insert.order_no);
  EXPECT_EQ(0, std::get<1>(ret).size());
}

TEST_F(FollowStragetyServiceFixture, ClosePositionForSHFECase1) {
  InitDefaultOrderExchangeId(kSHFEExchangeId);
  master_context_.InitPositions({{"abc", OrderDirection::kBuy, 10}});
  slave_context_.InitPositions({{"abc", OrderDirection::kBuy, 10}});
  OpenAndFilledOrder("1");

  auto ret = PushNewCloseOrderForMaster("2", OrderDirection::kSell, 10,
                                        8888.9, PositionEffect::kCloseToday);
  auto order_insert = std::get<0>(ret);
  EXPECT_EQ(order_insert.direction, OrderDirection::kSell);
  EXPECT_EQ(order_insert.position_effect, PositionEffect::kCloseToday);
  EXPECT_EQ("2", order_insert.order_no);
  EXPECT_EQ(0, std::get<1>(ret).size());
}

TEST_F(FollowStragetyServiceFixture, ClosePositionForSHFECase2) {
  InitDefaultOrderExchangeId(kSHFEExchangeId);
  master_context_.InitPositions({{"abc", OrderDirection::kBuy, 10}});
  slave_context_.InitPositions({{"abc", OrderDirection::kBuy, 10}});
  OpenAndFilledOrder("1");

  auto ret = PushNewCloseOrderForMaster("2", OrderDirection::kSell, 10,
                                        8888.9, PositionEffect::kClose);
  auto order_insert = std::get<0>(ret);
  EXPECT_EQ(order_insert.direction, OrderDirection::kSell);
  EXPECT_EQ(order_insert.position_effect, PositionEffect::kClose);
  EXPECT_EQ("2", order_insert.order_no);
  EXPECT_EQ(0, std::get<1>(ret).size());
}

TEST_F(FollowStragetyServiceFixture, CloseAllPositionCase1) {
  slave_context_.InitPositions({{"abc", OrderDirection::kBuy, 8}});
  OpenAndFilledOrder("1");

  {
    auto ret = PushNewCloseOrderForMaster("2", OrderDirection::kSell, 10,
                                          8888.9, PositionEffect::kClose);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ(10, std::get<0>(ret).quantity);

    (void)PushNewCloseOrderForSlave(order_insert.order_no,
                                    OrderDirection::kSell, 10);
  }

  {
    auto ret = PushCloseOrderForMaster("2", OrderDirection::kSell);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ(8, order_insert.quantity);
    EXPECT_EQ("3", order_insert.order_no);
  }
}

TEST_F(FollowStragetyServiceFixture, CloseAllPositionCase2) {
  InitDefaultOrderExchangeId(kSHFEExchangeId);
  slave_context_.InitPositions({{"abc", OrderDirection::kBuy, 8}});
  OpenAndFilledOrder("1");

  {
    auto ret = PushNewCloseOrderForMaster("2", OrderDirection::kSell, 10,
                                          8888.9, PositionEffect::kCloseToday);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ(10, std::get<0>(ret).quantity);
    EXPECT_EQ(PositionEffect::kCloseToday, order_insert.position_effect);

    (void)PushNewCloseOrderForSlave(order_insert.order_no,
                                    OrderDirection::kSell, 10,
                                    PositionEffect::kCloseToday);
  }

  {
    auto ret = PushCloseOrderForMaster("2", OrderDirection::kSell);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ(8, order_insert.quantity);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(PositionEffect::kClose, order_insert.position_effect);
    EXPECT_EQ(OrderPriceType::kMarket, order_insert.price_type);
  }
}

TEST_F(FollowStragetyServiceFixture, SyncCancelPartyFillClose) {
  master_context_.InitPositions({{"abc", OrderDirection::kBuy, 4}});
  OpenAndFilledOrder("1", 2, 2, 2);
  {
    auto ret = PushNewCloseOrderForMaster("2", OrderDirection::kSell, 4);
    EXPECT_EQ("", std::get<0>(ret).order_no);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }

  (void)PushCloseOrderForMaster("2", OrderDirection::kSell, 1, 4);

  {
    auto ret = PushCancelOrderForMaster("2", OrderDirection::kSell,
                                        PositionEffect::kClose, 1, 4);
    EXPECT_EQ("", std::get<0>(ret).order_no);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }
  {
    auto ret = PushNewCloseOrderForMaster("3", OrderDirection::kSell, 4);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("2", order_insert.order_no);
    EXPECT_EQ(1, order_insert.quantity);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }
}
