#include "gtest/gtest.h"
#include "geek_quant/instrument_follow.h"
#include "geek_quant/caf_defines.h"

class InstrumentFollowFixture : public testing::Test {
 public:
 protected:
  OrderRtnData MakeOrderRtnData(const std::string&& order_no,
                                OrderDirection order_direction,
                                OrderStatus order_status,
                                int volume = 10,
                                double order_price = 1234.1,
                                const std::string&& instrument = "abc") {
    OrderRtnData order;
    order.order_no = std::move(order_no);
    order.order_direction = order_direction;
    order.order_status = order_status;
    order.order_price = order_price;
    order.volume = volume;
    order.instrument = std::move(instrument);
    return order;
  }

  virtual void SetUp() override {}

  virtual void TearDown() override {}

 private:
  virtual void TestBody() override {}
};

TEST_F(InstrumentFollowFixture, FollowOpen) {
  InstrumentFollow instrument_follow;
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);
  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ(OrderDirection::kODBuy, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kEOAOpen, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(10, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, FollowClose) {
  InstrumentFollow instrument_follow;
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kODSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kEOAClose, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(10, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, FollowCancel) {
  InstrumentFollow instrument_follow;
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODSell,
                       OrderStatus::kOSCanceling),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
}

TEST_F(InstrumentFollowFixture, FollowPartFill) {
  InstrumentFollow instrument_follow;
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened,
                       6),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kODSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kEOAClose, enter_order.action);
  EXPECT_EQ(6, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, PartFillCase1) {
  // Trader Fully Fill, then Trader Part Fill and Follower has Part Fill

  InstrumentFollow instrument_follow;
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened,
                       6),
      &enter_order, &cancel_order_no_list);

  // reset test prevent write value from above
  enter_order.order_no = "";

  // Follow left 4 unfill
  // Trade part close 4 volume left 6 volume
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell, OrderStatus::kOSCloseing, 4),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
  EXPECT_EQ("", enter_order.order_no);

}

TEST_F(InstrumentFollowFixture, PartFillCase2) {
  // Trader Fully Fill, then Trader Part Fill and Follower has Part Fill

  InstrumentFollow instrument_follow;
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened,
                       6),
      &enter_order, &cancel_order_no_list);

  // reset test prevent write value from above
  enter_order.order_no = "";

  // Follow left 4 unfill
  // Trade part close 5 volume left 5 volume
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell, OrderStatus::kOSCloseing, 5),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(1, enter_order.volume);
}

// Mutl Open Order with one Close
TEST_F(InstrumentFollowFixture, FillMutilOrder) {
  InstrumentFollow instrument_follow;
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  // Open 0001
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  // Open 0002
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0002", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  // Close
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0003", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing, 20),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("0003", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kODSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kEOAClose, enter_order.action);
  EXPECT_EQ(20, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, PartFillMutilOrder) {
  InstrumentFollow instrument_follow;
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  // Open 0001
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  // Open 0002
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0002", OrderDirection::kODBuy, OrderStatus::kOSOpened,
                       6),
      &enter_order, &cancel_order_no_list);

  // Close
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0003", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing, 20),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0002", cancel_order_no_list.at(0));
  EXPECT_EQ("0003", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kODSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kEOAClose, enter_order.action);
  EXPECT_EQ(16, enter_order.volume);
}


TEST_F(InstrumentFollowFixture, CancelMutlOrder) {
  InstrumentFollow instrument_follow;
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  // Open 0001
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  // Open 0002
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODBuy, OrderStatus::kOSOpened),
      &enter_order, &cancel_order_no_list);

  // prevent write above code
  enter_order.order_no = "";
  // Close
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0003", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing, 20),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(2, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
  EXPECT_EQ("0002", cancel_order_no_list.at(1));
  EXPECT_EQ("", enter_order.order_no);

}
