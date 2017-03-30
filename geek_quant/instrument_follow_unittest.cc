#include "gtest/gtest.h"
#include "geek_quant/instrument_follow_fixture.h"

class InstrumentFollowFixture : public InstrumentFollowBaseFixture {
 protected:
  virtual void SetUp() override { instrument_follow.SyncComplete(); }
};

//////////////////////////////////////////////////////////////////////////
// Test Open Order
TEST_F(InstrumentFollowFixture, FollowOpenBuy) {
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

TEST_F(InstrumentFollowFixture, FollowOpenSell) {
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODSell,
                       OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);
  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ(OrderDirection::kODSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kEOAOpen, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(10, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, OpenOrderCase1) {
  OpenAndFillOrder(10, 10, 10);
  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;
    instrument_follow.HandleOrderRtnForTrader(
        MakeOrderRtnData("0002", OrderDirection::kODSell,
                         OrderStatus::kOSOpening),
        &enter_order, &cancel_order_no_list);
    EXPECT_EQ("0002", enter_order.order_no);
  }
}

//////////////////////////////////////////////////////////////////////////
// Test Close Order
TEST_F(InstrumentFollowFixture, CloseCase1) {
  OpenAndFillOrder(10, 10, 10);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

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

TEST_F(InstrumentFollowFixture, CloseCase2) {
  OpenAndFillOrder(10, 10, 5);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kODSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kEOAClose, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(5, enter_order.volume);
  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
}

TEST_F(InstrumentFollowFixture, CloseCase3) {
  // Trader Fully Fill, then Trader Part Fill and Follower has Part Fill

  OpenAndFillOrder(10, 10, 6);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;
  // Follow left 4 unfill
  // Trade part close 4 volume left 6 volume
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing, 4),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
  EXPECT_EQ("", enter_order.order_no);

  cancel_order_no_list.clear();

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0003", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing, 6),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(0, cancel_order_no_list.size());
  EXPECT_EQ("0003", enter_order.order_no);
  EXPECT_EQ(6, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, CloseCase4) {
  // Trader Fully Fill, then Trader Part Fill and Follower has Part Fill

  OpenAndFillOrder(10, 10, 6);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  // Follow left 4 unfill
  // Trade part close 5 volume left 5 volume
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing, 5),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(1, enter_order.volume);

  cancel_order_no_list.clear();
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0003", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing, 5),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(0, cancel_order_no_list.size());
  EXPECT_EQ("0003", enter_order.order_no);
  EXPECT_EQ(5, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, CloseCase5) {
  OpenAndFillOrder(10, 10, 6);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing, 1),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
  EXPECT_EQ("", enter_order.order_no);

  cancel_order_no_list.clear();
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0003", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing, 3),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(0, cancel_order_no_list.size());
  EXPECT_EQ("", enter_order.order_no);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0004", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing, 4),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(0, cancel_order_no_list.size());
  EXPECT_EQ("0004", enter_order.order_no);
  EXPECT_EQ(4, enter_order.volume);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0005", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing, 2),
      &enter_order, &cancel_order_no_list);
  EXPECT_EQ(0, cancel_order_no_list.size());
  EXPECT_EQ("0005", enter_order.order_no);
  EXPECT_EQ(2, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, CloseCase6) {
  OpenAndFillOrder(10, 5, 0);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("", enter_order.instrument);
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
}

//////////////////////////////////////////////////////////////////////////
// Cancel

TEST_F(InstrumentFollowFixture, CancelOpenCalse1) {
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy,
                       OrderStatus::kOSOpenCanceled),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
}

TEST_F(InstrumentFollowFixture, CancelOpenCase2) {
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpened,
                       5),
      &enter_order, &cancel_order_no_list);

  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;
    instrument_follow.HandleOrderRtnForTrader(
        MakeOrderRtnData("0001", OrderDirection::kODBuy,
                         OrderStatus::kOSOpenCanceled),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ(0, cancel_order_no_list.size());
  }

  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;
    instrument_follow.HandleOrderRtnForFollow(
        MakeOrderRtnData("0001", OrderDirection::kODBuy,
                         OrderStatus::kOSOpening),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ(1, cancel_order_no_list.size());
    EXPECT_EQ("0001", cancel_order_no_list.at(0));
  }
}

TEST_F(InstrumentFollowFixture, CancelOpenCalse3) {
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODSell,
                       OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0001", OrderDirection::kODSell,
                       OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODSell,
                       OrderStatus::kOSOpenCanceled),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
}

TEST_F(InstrumentFollowFixture, CancelCloseCase1) {
  OpenAndFillOrder(10, 10, 0);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;
  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing),
      &enter_order, &cancel_order_no_list);
  EXPECT_EQ("", enter_order.instrument);
  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
}

TEST_F(InstrumentFollowFixture, CancelCloseCase2) {
  OpenAndFillOrder(10, 10, 10);

  TraderOrderRtn("0002", kOSCloseing, 10, kODSell);
  FollowerOrderRtn("0002", kOSCloseing, 10, kODSell);
  auto ret = TraderOrderRtn("0002", kOSCloseCanceled, 10, kODSell);
  EnterOrderData& enter_order = ret.first;
  std::vector<std::string>& cancel_order_no_list = ret.second;
  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0002", cancel_order_no_list.at(0));

}

TEST_F(InstrumentFollowFixture, CancelCloseCase3) {
  OpenAndFillOrder(2, 2, 2);

  TraderOrderRtn("0002", kOSCloseing, 1, kODSell);
  FollowerOrderRtn("0002", kOSCloseing, 1, kODSell);
  TraderOrderRtn("0002", kOSCloseCanceled, 1, kODSell);
  TraderOrderRtn("0003", kOSCloseing, 1, kODSell);

  auto ret = FollowerOrderRtn("0002", kOSCloseCanceled, 1, kODSell);

  EnterOrderData& enter_order = ret.first;
  std::vector<std::string>& cancel_order_no_list = ret.second;
  EXPECT_EQ(0, cancel_order_no_list.size());
  EXPECT_EQ("0003", enter_order.order_no);
  EXPECT_EQ(1, enter_order.volume);
}


// Mutl Open Order with one Close
TEST_F(InstrumentFollowFixture, FillMutilOrder) {
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
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
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
      MakeOrderRtnData("0002", OrderDirection::kODBuy, OrderStatus::kOSOpening),
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
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
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
      MakeOrderRtnData("0002", OrderDirection::kODBuy, OrderStatus::kOSOpening),
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

  // Open 0001
  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0001", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  // Open 0002
  instrument_follow.HandleOrderRtnForFollow(
      MakeOrderRtnData("0002", OrderDirection::kODBuy, OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(2, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
  EXPECT_EQ("0002", cancel_order_no_list.at(1));
  EXPECT_EQ("", enter_order.order_no);
}

//////////////////////////////////////////////////////////////////////////
// Open Reverse Order

TEST_F(InstrumentFollowFixture, OpenReverseOrderCase1) {
  OpenAndFillOrder(10, 10, 10);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell,
                       OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kODSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kEOAOpen, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(10, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, OpenReverseOrderCase2) {
  OpenAndFillOrder(10, 10, 6);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell,
                       OrderStatus::kOSOpening),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kODSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kEOAOpen, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(6, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, OpenReverseOrderCase3) {
  TraderOrderRtn("0001", kOSOpening, 10);
  TraderOrderRtn("0001", kOSOpened, 5);
  FollowerOrderRtn("0001", kOSOpening, 10);
  // Reverse Opening

  {
    auto ret = TraderOrderRtn("0002", kOSOpening, 5, kODSell);
    EnterOrderData& enter_order = ret.first;
    std::vector<std::string>& cancel_order_no_list = ret.second;
    EXPECT_EQ("", enter_order.order_no);
    EXPECT_EQ(1, cancel_order_no_list.size());
    EXPECT_EQ("0001", cancel_order_no_list.at(0));
  }
}

TEST_F(InstrumentFollowFixture, OpenReverseOrderThenCloseCase1) {
  OpenAndFillOrder(10, 10, 10);

  // Open Reverse Order
  OpenAndFillOrder(10, 10, 10, kODSell, "0002");

  // Close Same way Order
  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;

    instrument_follow.HandleOrderRtnForTrader(
        MakeOrderRtnData("0003", OrderDirection::kODSell,
                         OrderStatus::kOSCloseing),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ("abc", enter_order.instrument);
    EXPECT_EQ("0003", enter_order.order_no);
    EXPECT_EQ(OrderDirection::kODSell, enter_order.order_direction);
    EXPECT_EQ(EnterOrderAction::kEOAClose, enter_order.action);
    EXPECT_EQ(1234.1, enter_order.order_price);
    EXPECT_EQ(10, enter_order.volume);
  }

  // Close Reverse Order
  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;

    instrument_follow.HandleOrderRtnForTrader(
        MakeOrderRtnData("0004", OrderDirection::kODBuy,
                         OrderStatus::kOSCloseing),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ("abc", enter_order.instrument);
    EXPECT_EQ("0004", enter_order.order_no);
    EXPECT_EQ(OrderDirection::kODBuy, enter_order.order_direction);
    EXPECT_EQ(EnterOrderAction::kEOAClose, enter_order.action);
    EXPECT_EQ(1234.1, enter_order.order_price);
    EXPECT_EQ(10, enter_order.volume);
  }
}

TEST_F(InstrumentFollowFixture, OpenReverseOrderThenCloseCase2) {
  OpenAndFillOrder(10, 10, 10);

  // Open Reverse Order
  OpenAndFillOrder(10, 10, 10, kODSell, "0002");

  // Close Reverse Order
  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;

    instrument_follow.HandleOrderRtnForTrader(
        MakeOrderRtnData("0003", OrderDirection::kODBuy,
                         OrderStatus::kOSCloseing),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ("abc", enter_order.instrument);
    EXPECT_EQ("0003", enter_order.order_no);
    EXPECT_EQ(OrderDirection::kODBuy, enter_order.order_direction);
    EXPECT_EQ(EnterOrderAction::kEOAClose, enter_order.action);
    EXPECT_EQ(1234.1, enter_order.order_price);
    EXPECT_EQ(10, enter_order.volume);
  }

  // Close Same way Order
  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;

    instrument_follow.HandleOrderRtnForTrader(
        MakeOrderRtnData("0004", OrderDirection::kODSell,
                         OrderStatus::kOSCloseing),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ("abc", enter_order.instrument);
    EXPECT_EQ("0004", enter_order.order_no);
    EXPECT_EQ(OrderDirection::kODSell, enter_order.order_direction);
    EXPECT_EQ(EnterOrderAction::kEOAClose, enter_order.action);
    EXPECT_EQ(1234.1, enter_order.order_price);
    EXPECT_EQ(10, enter_order.volume);
  }
}
