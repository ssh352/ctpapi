#include "gtest/gtest.h"
#include "follow_strategy_mode/unittest/follow_strategy_servcie_fixture.h"

// Test Open
TEST_F(FollowStragetyServiceFixture, OpenBuy) {
  auto ret = PushNewOpenOrderForMaster();

  OrderInsertForTest order_insert = std::get<0>(ret);

  EXPECT_EQ("1", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(10, order_insert.quantity);
  EXPECT_EQ(OrderDirection::kBuy, order_insert.direction);
}

TEST_F(FollowStragetyServiceFixture, OpenSell) {
  auto ret = PushNewOpenOrderForMaster("1", OrderDirection::kSell, 20);

  OrderInsertForTest order_insert = std::get<0>(ret);

  EXPECT_EQ("1", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(20, order_insert.quantity);
  EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
}

TEST_F(FollowStragetyServiceFixture, IncreaseOpenOrder) {
  OpenAndFilledOrder("1");

  auto order = std::get<0>(PushNewOpenOrderForMaster("2"));
  EXPECT_EQ("2", order.order_no);
}

// Test Close

TEST_F(FollowStragetyServiceFixture, CloseOrderCase1) {
  OpenAndFilledOrder("1");

  auto order_insert =
      std::get<0>(PushNewCloseOrderForMaster("2", OrderDirection::kSell, 10));
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(10, order_insert.quantity);
}

TEST_F(FollowStragetyServiceFixture, CloseOrderCase2) {
  OpenAndFilledOrder("1", 10, 10, 5);

  auto ret = PushNewCloseOrderForMaster("2");

  auto order_insert = std::get<0>(ret);
  EXPECT_EQ("2", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(5, order_insert.quantity);

  auto cancels = std::get<1>(ret);
  EXPECT_EQ(1, cancels.size());
  EXPECT_EQ("1", cancels.at(0));
}

TEST_F(FollowStragetyServiceFixture, CloseOrderCase3) {
  OpenAndFilledOrder("1", 10, 10, 6);

  {
    auto ret = PushNewCloseOrderForMaster("2", OrderDirection::kSell, 4);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);

    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }

  (void)PushCancelOrderForSlave("1", OrderDirection::kBuy,
                                PositionEffect::kOpen, 10, 6);

  {
    auto ret = PushNewCloseOrderForMaster("3", OrderDirection::kSell, 6);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(6, order_insert.quantity);

    auto cancels = std::get<1>(ret);
    EXPECT_EQ(0, cancels.size());
  }
}

TEST_F(FollowStragetyServiceFixture, CloseOrderCase4) {
  OpenAndFilledOrder("1", 10, 10, 6);

  {
    auto ret = PushNewCloseOrderForMaster("2", OrderDirection::kSell, 5);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("2", order_insert.order_no);
    EXPECT_EQ(1, order_insert.quantity);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }

  (void)PushCancelOrderForSlave("1", OrderDirection::kBuy,
                                PositionEffect::kOpen, 10, 6);

  {
    service->HandleRtnOrder(MakeSlaveOrderData("2", OrderDirection::kSell,
                                              PositionEffect::kClose,
                                              OrderStatus::kActive, 0, 1));
  }

  {
    auto ret = PushNewCloseOrderForMaster("3", OrderDirection::kSell, 5);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(5, order_insert.quantity);

    auto cancels = std::get<1>(ret);
    EXPECT_EQ(0, cancels.size());
  }
}

TEST_F(FollowStragetyServiceFixture, CloseOrderCase5) {
  OpenAndFilledOrder("1", 10, 10, 6);

  {
    auto ret = PushNewCloseOrderForMaster("2", OrderDirection::kSell, 1);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }

  (void)PushCancelOrderForSlave("1", OrderDirection::kBuy,
                                PositionEffect::kOpen, 6, 10);

  {
    auto ret = PushNewCloseOrderForMaster("3", OrderDirection::kSell, 3);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(0, cancels.size());
  }

  {
    auto ret = PushNewCloseOrderForMaster("4", OrderDirection::kSell, 4);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("4", order_insert.order_no);
    EXPECT_EQ(4, order_insert.quantity);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(0, cancels.size());
  }

  {
    service->HandleRtnOrder(MakeSlaveOrderData("4", OrderDirection::kSell,
                                              PositionEffect::kClose,
                                              OrderStatus::kActive, 0, 4));
  }

  {
    auto ret = PushNewCloseOrderForMaster("5", OrderDirection::kSell, 2);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("5", order_insert.order_no);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(2, order_insert.quantity);

    auto cancels = std::get<1>(ret);
    EXPECT_EQ(0, cancels.size());
  }
}

TEST_F(FollowStragetyServiceFixture, CloseOrderCase6) {
  OpenAndFilledOrder("1");
  (void)PushNewCloseOrderForMaster("2", OrderDirection::kSell, 10);
  (void)PushNewCloseOrderForSlave("2", OrderDirection::kSell, 10);
  (void)PushCancelOrderForMaster("2", OrderDirection::kSell,
                                 PositionEffect::kClose);
  (void)PushNewCloseOrderForMaster("3", OrderDirection::kSell, 10, 8888.8);

  {
    auto ret = PushCancelOrderForSlave("2", OrderDirection::kSell,
                                       PositionEffect::kClose);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(8888.8, order_insert.price);
  }
}

// Cancel Order
TEST_F(FollowStragetyServiceFixture, CancelOrderCase1) {
  PushOpenOrder("1");
  {
    auto ret = PushCancelOrderForMaster("1");

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);

    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }
}

TEST_F(FollowStragetyServiceFixture, CancelOrderCase2) {
  OpenAndFilledOrder("1", 10, 5, 0);

  {
    auto ret = PushCancelOrderForMaster("1", OrderDirection::kBuy,
                                        PositionEffect::kOpen, 5, 10);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }
}

TEST_F(FollowStragetyServiceFixture, CancelOrderCase3) {
  service->HandleRtnOrder(MakeMasterOrderData("1", OrderDirection::kBuy,
                                             PositionEffect::kOpen,
                                             OrderStatus::kActive, 0, 10));

  service->HandleRtnOrder(MakeMasterOrderData("1", OrderDirection::kBuy,
                                             PositionEffect::kOpen,
                                             OrderStatus::kCancel, 0, 10));

  (void)PopOrderInsert();

  service->HandleRtnOrder(MakeSlaveOrderData("1", OrderDirection::kBuy,
                                            PositionEffect::kOpen,
                                            OrderStatus::kActive, 0, 10));
  {
    auto ret = PopOrderEffectForTest();

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }
}

// Multi Open Order With One Close

TEST_F(FollowStragetyServiceFixture, FillMultiOrder) {
  OpenAndFilledOrder("1");
  OpenAndFilledOrder("2");

  {
    auto ret =
        PushNewCloseOrderForMaster("3", OrderDirection::kSell, 20, 6666.6);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(6666.6, order_insert.price);
    EXPECT_EQ(20, order_insert.quantity);
  }
}

TEST_F(FollowStragetyServiceFixture, PartFillMultiOrder) {
  OpenAndFilledOrder("1");
  OpenAndFilledOrder("2", 10, 10, 6);
  {
    auto ret =
        PushNewCloseOrderForMaster("3", OrderDirection::kSell, 20, 6666.6);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(6666.6, order_insert.price);
    EXPECT_EQ(16, order_insert.quantity);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("2", cancels.at(0));
  }
}

/*


TEST_F(FollowStragetyServiceFixture, CancelMultiOrder) {
  PushOpenOrderForMaster("1");
  PushOpenOrderForMaster("2");

  (void)PushNewCloseOrderForMaster("3", OrderDirection::kSell, 20, 6666.6);

  {
    auto ret = PushOpenOrderForSlave("1");
    EXPECT_EQ("", std::get<0>(ret).order_no);
    EXPECT_EQ(1, std::get<1>(ret).size());
    EXPECT_EQ("1", std::get<1>(ret).at(0));
  }

  {
    auto ret = PushOpenOrderForSlave("2");
    EXPECT_EQ("", std::get<0>(ret).order_no);
    EXPECT_EQ(1, std::get<1>(ret).size());
    EXPECT_EQ("2", std::get<1>(ret).at(0));
  }
}
*/

// Open Opposite Order

TEST_F(FollowStragetyServiceFixture, OpenOppositeOrderCase1) {
  OpenAndFilledOrder("1");

  {
    auto ret = PushOpenOrderForMaster("2", 10, OrderDirection::kSell);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("2", order_insert.order_no);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }
}

TEST_F(FollowStragetyServiceFixture, OpenOppositeOrderCase2) {
  OpenAndFilledOrder("1", 10, 10, 6);

  {
    auto ret = PushOpenOrderForMaster("2", 10, OrderDirection::kSell);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("2", order_insert.order_no);
    EXPECT_EQ(6, order_insert.quantity);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(1, std::get<1>(ret).size());
    EXPECT_EQ("1", std::get<1>(ret).at(0));
  }
}

TEST_F(FollowStragetyServiceFixture, OpenOppositeOrderCase3) {
  OpenAndFilledOrder("1", 10, 5, 0);

  {
    auto ret = PushOpenOrderForMaster("2", 5, OrderDirection::kSell);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }
  {
    auto ret = PushCancelOrderForMaster("1", OrderDirection::kBuy,
                                        PositionEffect::kClose, 5, 10);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    auto cancel_orders = std::get<1>(ret);
    EXPECT_EQ(1, cancel_orders.size());
    EXPECT_EQ("1", cancel_orders.at(0));
  }
}

TEST_F(FollowStragetyServiceFixture, OpenOppositeThenCloseCase1) {
  OpenAndFilledOrder("1", 10, 10, 10, OrderDirection::kBuy);
  OpenAndFilledOrder("2", 10, 10, 10, OrderDirection::kSell);

  {
    auto ret = PushNewCloseOrderForMaster("3", OrderDirection::kSell, 10);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(PositionEffect::kClose, order_insert.position_effect);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }

  (void)PushNewCloseOrderForSlave("3", OrderDirection::kSell, 10);
  {
    auto ret = PushNewCloseOrderForMaster("4", OrderDirection::kBuy, 10);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("4", order_insert.order_no);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(PositionEffect::kClose, order_insert.position_effect);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }
}

TEST_F(FollowStragetyServiceFixture, OpenOppositeThenCloseCase2) {
  OpenAndFilledOrder("1", 10, 10, 10, OrderDirection::kBuy);
  OpenAndFilledOrder("2", 10, 10, 10, OrderDirection::kSell);
  {
    auto ret = PushNewCloseOrderForMaster("3", OrderDirection::kBuy, 10);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(PositionEffect::kClose, order_insert.position_effect);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }

  (void)PushNewCloseOrderForSlave("3", OrderDirection::kBuy, 10);

  {
    auto ret = PushNewCloseOrderForMaster("4", OrderDirection::kSell, 10);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("4", order_insert.order_no);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(PositionEffect::kClose, order_insert.position_effect);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }

}
