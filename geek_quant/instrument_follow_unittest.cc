#include "gtest/gtest.h"
#include "geek_quant/instrument_follow.h"
#include "geek_quant/caf_defines.h"

class InstrumentFollowFixture : public testing::Test {
 public:
 protected:
  OrderRtnData MakeOrderRtnData(const std::string& order_no,
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

  void OpenAndFillOrder(InstrumentFollow* instrument_follow,
                        int open_volume,
                        int fill_open_volume,
                        int fill_follow_open_volume,
                        OrderDirection order_direction = OrderDirection::kODBuy,
                        const std::string& order_no = "0001",
                        double order_price = 1234.1,
                        std::string&& instrument = "abc") {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;
    instrument_follow->HandleOrderRtnForTrader(
        MakeOrderRtnData(order_no, order_direction, OrderStatus::kOSOpening,
                         open_volume),
        &enter_order, &cancel_order_no_list);

    instrument_follow->HandleOrderRtnForTrader(
        MakeOrderRtnData(order_no, order_direction, OrderStatus::kOSOpened,
                         fill_open_volume),
        &enter_order, &cancel_order_no_list);

    instrument_follow->HandleOrderRtnForFollow(
        MakeOrderRtnData(order_no, order_direction, OrderStatus::kOSOpened,
                         fill_follow_open_volume),
        &enter_order, &cancel_order_no_list);
  }

 private:
  virtual void TestBody() override {}
};

//////////////////////////////////////////////////////////////////////////
// Test Open Order
TEST_F(InstrumentFollowFixture, FollowOpenBuy) {
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

TEST_F(InstrumentFollowFixture, FollowOpenSell) {
  InstrumentFollow instrument_follow;
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

//////////////////////////////////////////////////////////////////////////
// Test Close Order
TEST_F(InstrumentFollowFixture, CloseCase1) {
  InstrumentFollow instrument_follow;

  OpenAndFillOrder(&instrument_follow, 10, 10, 10);

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
  InstrumentFollow instrument_follow;

  OpenAndFillOrder(&instrument_follow, 10, 10, 5);

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

  InstrumentFollow instrument_follow;

  OpenAndFillOrder(&instrument_follow, 10, 10, 6);

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
  InstrumentFollow instrument_follow;
  OpenAndFillOrder(&instrument_follow, 10, 10, 6);

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
  InstrumentFollow instrument_follow;
  OpenAndFillOrder(&instrument_follow, 10, 10, 6);

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


//////////////////////////////////////////////////////////////////////////
// Cancel

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

TEST_F(InstrumentFollowFixture, CancelCase1) {
  InstrumentFollow instrument_follow;

  OpenAndFillOrder(&instrument_follow, 10, 5, 0);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0002", OrderDirection::kODSell,
                       OrderStatus::kOSCloseing),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("", enter_order.instrument);
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
}

TEST_F(InstrumentFollowFixture, CancelCase2) {
  InstrumentFollow instrument_follow;

  OpenAndFillOrder(&instrument_follow, 10, 5, 0);

  EnterOrderData enter_order;
  std::vector<std::string> cancel_order_no_list;

  instrument_follow.HandleOrderRtnForTrader(
      MakeOrderRtnData("0001", OrderDirection::kODSell,
                       OrderStatus::kOSCanceling),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("", enter_order.instrument);
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
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

//////////////////////////////////////////////////////////////////////////
// Open Reverse Order

TEST_F(InstrumentFollowFixture, OpenReverseOrderCase1) {
  InstrumentFollow instrument_follow;

  OpenAndFillOrder(&instrument_follow, 10, 10, 10);

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
  InstrumentFollow instrument_follow;

  OpenAndFillOrder(&instrument_follow, 10, 10, 6);

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
