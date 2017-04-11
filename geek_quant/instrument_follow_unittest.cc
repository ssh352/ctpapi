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
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);
  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ(OrderDirection::kBuy, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kOpen, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(10, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, FollowOpenSell) {
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kSell, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);
  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ(OrderDirection::kSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kOpen, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(10, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, OpenOrderCase1) {
  OpenAndFillOrder(10, 10, 10);
  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;
    instrument_follow.HandleOrderRtnForTrader(
        MakeRtnOrderData("0002", OrderDirection::kSell, OldOrderStatus::kOpening),
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
      MakeRtnOrderData("0002", OrderDirection::kSell, OldOrderStatus::kCloseing),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kClose, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(10, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, CloseCase2) {
  OpenAndFillOrder(10, 10, 5);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0002", OrderDirection::kSell, OldOrderStatus::kCloseing),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kClose, enter_order.action);
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
      MakeRtnOrderData("0002", OrderDirection::kSell, OldOrderStatus::kCloseing,
                       4),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
  EXPECT_EQ("", enter_order.order_no);

  cancel_order_no_list.clear();

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0003", OrderDirection::kSell, OldOrderStatus::kCloseing,
                       6),
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
      MakeRtnOrderData("0002", OrderDirection::kSell, OldOrderStatus::kCloseing,
                       5),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(1, enter_order.volume);

  cancel_order_no_list.clear();
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0003", OrderDirection::kSell, OldOrderStatus::kCloseing,
                       5),
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
      MakeRtnOrderData("0002", OrderDirection::kSell, OldOrderStatus::kCloseing,
                       1),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
  EXPECT_EQ("", enter_order.order_no);

  cancel_order_no_list.clear();
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0003", OrderDirection::kSell, OldOrderStatus::kCloseing,
                       3),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(0, cancel_order_no_list.size());
  EXPECT_EQ("", enter_order.order_no);

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0004", OrderDirection::kSell, OldOrderStatus::kCloseing,
                       4),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(0, cancel_order_no_list.size());
  EXPECT_EQ("0004", enter_order.order_no);
  EXPECT_EQ(4, enter_order.volume);

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0005", OrderDirection::kSell, OldOrderStatus::kCloseing,
                       2),
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
      MakeRtnOrderData("0002", OrderDirection::kSell, OldOrderStatus::kCloseing),
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
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kBuy,
                       OldOrderStatus::kOpenCanceled),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
}

TEST_F(InstrumentFollowFixture, CancelOpenCase2) {
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpened, 5),
      &enter_order, &cancel_order_no_list);

  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;
    instrument_follow.HandleOrderRtnForTrader(
        MakeRtnOrderData("0001", OrderDirection::kBuy,
                         OldOrderStatus::kOpenCanceled),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ(0, cancel_order_no_list.size());
  }

  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;
    instrument_follow.HandleOrderRtnForFollow(
        MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpening),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ(1, cancel_order_no_list.size());
    EXPECT_EQ("0001", cancel_order_no_list.at(0));
  }
}

TEST_F(InstrumentFollowFixture, CancelOpenCalse3) {
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kSell, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0001", OrderDirection::kSell, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kSell,
                       OldOrderStatus::kOpenCanceled),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
}

TEST_F(InstrumentFollowFixture, CancelCloseCase1) {
  OpenAndFillOrder(10, 10, 0);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0002", OrderDirection::kSell, OldOrderStatus::kCloseing),
      &enter_order, &cancel_order_no_list);
  EXPECT_EQ("", enter_order.instrument);
  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
}

TEST_F(InstrumentFollowFixture, CancelCloseCase2) {
  OpenAndFillOrder(10, 10, 10);

  TraderOrderRtn("0002", OldOrderStatus::kCloseing, 10, OrderDirection::kSell);
  FollowerOrderRtn("0002", OldOrderStatus::kCloseing, 10, OrderDirection::kSell);
  auto ret = TraderOrderRtn("0002", OldOrderStatus::kCloseCanceled, 10,
                            OrderDirection::kSell);
  EnterOrderData& enter_order = ret.first;
  std::vector<std::string>& cancel_order_no_list = ret.second;
  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0002", cancel_order_no_list.at(0));
}

TEST_F(InstrumentFollowFixture, CancelCloseCase3) {
  OpenAndFillOrder(2, 2, 2);

  TraderOrderRtn("0002", OldOrderStatus::kCloseing, 1, OrderDirection::kSell);
  FollowerOrderRtn("0002", OldOrderStatus::kCloseing, 1, OrderDirection::kSell);
  TraderOrderRtn("0002", OldOrderStatus::kCloseCanceled, 1, OrderDirection::kSell);
  TraderOrderRtn("0003", OldOrderStatus::kCloseing, 1, OrderDirection::kSell);

  auto ret = FollowerOrderRtn("0002", OldOrderStatus::kCloseCanceled, 1,
                              OrderDirection::kSell);

  EnterOrderData& enter_order = ret.first;
  std::vector<std::string>& cancel_order_no_list = ret.second;
  EXPECT_EQ(0, cancel_order_no_list.size());
  // EXPECT_EQ("0003", enter_order.order_no);
  // EXPECT_EQ(1, enter_order.volume);
}

// Mutl Open Order with one Close
TEST_F(InstrumentFollowFixture, FillMutilOrder) {
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  // Open 0001
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpened),
      &enter_order, &cancel_order_no_list);

  // Open 0002
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0002", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0002", OrderDirection::kBuy, OldOrderStatus::kOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0002", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0002", OrderDirection::kBuy, OldOrderStatus::kOpened),
      &enter_order, &cancel_order_no_list);

  // Close
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0003", OrderDirection::kSell, OldOrderStatus::kCloseing,
                       20),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("0003", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kClose, enter_order.action);
  EXPECT_EQ(20, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, PartFillMutilOrder) {
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  // Open 0001
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpened),
      &enter_order, &cancel_order_no_list);

  // Open 0002
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0002", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0002", OrderDirection::kBuy, OldOrderStatus::kOpened),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0002", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0002", OrderDirection::kBuy, OldOrderStatus::kOpened, 6),
      &enter_order, &cancel_order_no_list);

  // Close
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0003", OrderDirection::kSell, OldOrderStatus::kCloseing,
                       20),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0002", cancel_order_no_list.at(0));
  EXPECT_EQ("0003", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kClose, enter_order.action);
  EXPECT_EQ(16, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, CancelMutlOrder) {
  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  // follow_strategy_service.HandleRtnOrder();
  // Open 0001
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpened),
      &enter_order, &cancel_order_no_list);

  // Open 0002
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0002", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0002", OrderDirection::kBuy, OldOrderStatus::kOpened),
      &enter_order, &cancel_order_no_list);

  // prevent write above code
  enter_order.order_no = "";
  // Close
  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0003", OrderDirection::kSell, OldOrderStatus::kCloseing,
                       20),
      &enter_order, &cancel_order_no_list);

  // Open 0001
  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0001", OrderDirection::kBuy, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  // Open 0002
  instrument_follow.HandleOrderRtnForFollow(
      MakeRtnOrderData("0002", OrderDirection::kBuy, OldOrderStatus::kOpening),
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
      MakeRtnOrderData("0002", OrderDirection::kSell, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kOpen, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(10, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, OpenReverseOrderCase2) {
  OpenAndFillOrder(10, 10, 6);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeRtnOrderData("0002", OrderDirection::kSell, OldOrderStatus::kOpening),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kOpen, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(6, enter_order.volume);
}

TEST_F(InstrumentFollowFixture, OpenReverseOrderCase3) {
  TraderOrderRtn("0001", OldOrderStatus::kOpening, 10);
  TraderOrderRtn("0001", OldOrderStatus::kOpened, 5);
  FollowerOrderRtn("0001", OldOrderStatus::kOpening, 10);
  // Reverse Opening

  {
    auto ret =
        TraderOrderRtn("0002", OldOrderStatus::kOpening, 5, OrderDirection::kSell);
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
  OpenAndFillOrder(10, 10, 10, OrderDirection::kSell, "0002");

  // Close Same way Order
  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;

    instrument_follow.HandleOrderRtnForTrader(
        MakeRtnOrderData("0003", OrderDirection::kSell, OldOrderStatus::kCloseing),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ("abc", enter_order.instrument);
    EXPECT_EQ("0003", enter_order.order_no);
    EXPECT_EQ(OrderDirection::kSell, enter_order.order_direction);
    EXPECT_EQ(EnterOrderAction::kClose, enter_order.action);
    EXPECT_EQ(1234.1, enter_order.order_price);
    EXPECT_EQ(10, enter_order.volume);
  }

  // Close Reverse Order
  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;

    instrument_follow.HandleOrderRtnForTrader(
        MakeRtnOrderData("0004", OrderDirection::kBuy, OldOrderStatus::kCloseing),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ("abc", enter_order.instrument);
    EXPECT_EQ("0004", enter_order.order_no);
    EXPECT_EQ(OrderDirection::kBuy, enter_order.order_direction);
    EXPECT_EQ(EnterOrderAction::kClose, enter_order.action);
    EXPECT_EQ(1234.1, enter_order.order_price);
    EXPECT_EQ(10, enter_order.volume);
  }
}

TEST_F(InstrumentFollowFixture, OpenReverseOrderThenCloseCase2) {
  OpenAndFillOrder(10, 10, 10);

  // Open Reverse Order
  OpenAndFillOrder(10, 10, 10, OrderDirection::kSell, "0002");

  // Close Reverse Order
  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;

    instrument_follow.HandleOrderRtnForTrader(
        MakeRtnOrderData("0003", OrderDirection::kBuy, OldOrderStatus::kCloseing),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ("abc", enter_order.instrument);
    EXPECT_EQ("0003", enter_order.order_no);
    EXPECT_EQ(OrderDirection::kBuy, enter_order.order_direction);
    EXPECT_EQ(EnterOrderAction::kClose, enter_order.action);
    EXPECT_EQ(1234.1, enter_order.order_price);
    EXPECT_EQ(10, enter_order.volume);
  }

  // Close Same way Order
  {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;

    instrument_follow.HandleOrderRtnForTrader(
        MakeRtnOrderData("0004", OrderDirection::kSell, OldOrderStatus::kCloseing),
        &enter_order, &cancel_order_no_list);

    EXPECT_EQ("abc", enter_order.instrument);
    EXPECT_EQ("0004", enter_order.order_no);
    EXPECT_EQ(OrderDirection::kSell, enter_order.order_direction);
    EXPECT_EQ(EnterOrderAction::kClose, enter_order.action);
    EXPECT_EQ(1234.1, enter_order.order_price);
    EXPECT_EQ(10, enter_order.volume);
  }
}
