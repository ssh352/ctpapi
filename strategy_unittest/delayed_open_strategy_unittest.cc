#include "gtest/gtest.h"
#include "strategy_fixture.h"

#include "follow_strategy/delayed_open_strategy.h"
#include "unittest_helper.h"

TEST_F(StrategyFixture, DelayedOpen) {
  std::string master_account_id = "master";
  std::string slave_account_id = "slave";
  int delayed_open_after_seconds = 60;
  CreateStrategy<DelayedOpenStrategy<UnittestMailBox> >(
      master_account_id, slave_account_id, delayed_open_after_seconds);
  {
    Send(CTASignalAtom::value,
         MakeNewOpenOrder(master_account_id, "0", "I1", OrderDirection::kBuy,
                          88.8, 10, 0));
  }

  EXPECT_FALSE(PopupRntOrder<InputOrder>());
  Send(CTASignalAtom::value, MakeTradedOrder(master_account_id, "0", 5, 0));
  EXPECT_FALSE(PopupRntOrder<InputOrder>());

  Send(MakeTick("I1", 88.8, 10, delayed_open_after_seconds * 1000 - 1));
  EXPECT_FALSE(PopupRntOrder<InputOrder>());

  Send(MakeTick("I1", 88.8, 10, delayed_open_after_seconds * 1000));
  {
    auto input_order = PopupRntOrder<InputOrder>();
    EXPECT_TRUE(input_order);
    EXPECT_EQ(88.8, input_order->price_);
    EXPECT_EQ(input_order->qty_, 5);
  }
}

TEST_F(StrategyFixture, Close) {
  std::string master_account_id = "master";
  std::string slave_account_id = "slave";
  int delayed_open_after_seconds = 60;
  CreateStrategy<DelayedOpenStrategy<UnittestMailBox> >(
      master_account_id, slave_account_id, delayed_open_after_seconds);

  OpenAndFillOrder(master_account_id, slave_account_id, OrderDirection::kBuy,
                   10, 10, 10, delayed_open_after_seconds);

  Send(CTASignalAtom::value,
       MakeNewCloseOrder(master_account_id, "1", "I1", OrderDirection::kSell,
                         88.8, 10, 0));
  {
    auto input_order = PopupRntOrder<InputOrder>();
    EXPECT_TRUE(input_order);
    EXPECT_EQ(OrderDirection::kSell, input_order->order_direction_);
    EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);
    EXPECT_EQ(10, input_order->qty_);
  }
}

TEST_F(StrategyFixture, CancelOpenOrderCase1) {
  std::string master_account_id = "master";
  std::string slave_account_id = "slave";
  int delayed_open_after_seconds = 60;
  CreateStrategy<DelayedOpenStrategy<UnittestMailBox> >(
      master_account_id, slave_account_id, delayed_open_after_seconds);
  TimeStamp timestamp = 0;

  Send(CTASignalAtom::value,
       MakeNewOpenOrder(master_account_id, "0", "I1", OrderDirection::kBuy,
                        88.8, 10, 0));
  Send(CTASignalAtom::value,
       MakeTradedOrder(master_account_id, "0", 10, timestamp));

  timestamp = delayed_open_after_seconds * 1000;
  Send(MakeTick("I1", 88.8, 10, timestamp));

  Send(MakeNewOpenOrder(slave_account_id, "0", "I1", OrderDirection::kBuy, 88.8,
                        10, timestamp));
  Clear();

  Send(CTASignalAtom::value,
       MakeNewCloseOrder(master_account_id, "1", "I1", OrderDirection::kSell,
                         88.8, 10, timestamp));
  EXPECT_FALSE(PopupRntOrder<InputOrder>());

  Send(CTASignalAtom::value,
       MakeTradedOrder(master_account_id, "1", 10, timestamp));

  {
    auto cancel_order = PopupRntOrder<CancelOrderSignal>();
    EXPECT_TRUE(cancel_order);
    EXPECT_EQ("0", cancel_order->order_id);
  }
}

TEST_F(StrategyFixture, CancelOpenOrderCase2) {
  std::string master_account_id = "master";
  std::string slave_account_id = "slave";
  int delayed_open_after_seconds = 60;
  CreateStrategy<DelayedOpenStrategy<UnittestMailBox> >(
      master_account_id, slave_account_id, delayed_open_after_seconds);
  TimeStamp timestamp = 0;

  Send(CTASignalAtom::value,
       MakeNewOpenOrder(master_account_id, "0", "I1", OrderDirection::kBuy,
                        88.8, 10, 0));
  Send(CTASignalAtom::value,
       MakeTradedOrder(master_account_id, "0", 10, timestamp));

  timestamp = delayed_open_after_seconds * 1000;
  Send(MakeTick("I1", 88.8, 10, timestamp));

  Send(MakeNewOpenOrder(slave_account_id, "0", "I1", OrderDirection::kBuy, 88.8,
                        10, timestamp));

  Send(MakeTradedOrder(slave_account_id, "0", 4, timestamp));
  Clear();

  Send(CTASignalAtom::value,
       MakeNewCloseOrder(master_account_id, "1", "I1", OrderDirection::kSell,
                         88.8, 10, timestamp));
  {
    auto order = PopupRntOrder<InputOrder>();
    EXPECT_EQ(4, order->qty_);
    EXPECT_EQ("1", order->order_id);
  }

  Send(MakeNewCloseOrder(slave_account_id, "1", "I1", OrderDirection::kSell,
                         88.8, 4, timestamp));

  Send(CTASignalAtom::value,
       MakeTradedOrder(master_account_id, "1", 10, timestamp));

  {
    auto cancel_order = PopupRntOrder<CancelOrderSignal>();
    EXPECT_TRUE(cancel_order);
    EXPECT_EQ("0", cancel_order->order_id);
  }
}

TEST_F(StrategyFixture, CancelOpenOrderCase3) {
  std::string master_account_id = "master";
  std::string slave_account_id = "slave";
  int delayed_open_after_seconds = 60;
  CreateStrategy<DelayedOpenStrategy<UnittestMailBox> >(
      master_account_id, slave_account_id, delayed_open_after_seconds);
  TimeStamp timestamp = 0;

  Send(CTASignalAtom::value,
       MakeNewOpenOrder(master_account_id, "0", "I1", OrderDirection::kBuy,
                        88.8, 10, 0));
  Send(CTASignalAtom::value,
       MakeTradedOrder(master_account_id, "0", 10, timestamp));

  timestamp = delayed_open_after_seconds * 1000;
  Send(MakeTick("I1", 88.8, 10, timestamp));

  Send(MakeNewOpenOrder(slave_account_id, "0", "I1", OrderDirection::kBuy, 88.8,
                        10, timestamp));

  Send(MakeTradedOrder(slave_account_id, "0", 4, timestamp));
  Clear();

  Send(CTASignalAtom::value,
       MakeNewCloseOrder(master_account_id, "1", "I1", OrderDirection::kSell,
                         88.8, 10, timestamp));
  {
    auto order = PopupRntOrder<InputOrder>();
    EXPECT_EQ(4, order->qty_);
    EXPECT_EQ("1", order->order_id);
  }

  Send(MakeNewCloseOrder(slave_account_id, "1", "I1", OrderDirection::kSell,
                         88.8, 4, timestamp));

  Send(CTASignalAtom::value,
       MakeTradedOrder(master_account_id, "1", 9, timestamp));

  {
    auto cancel_order = PopupRntOrder<CancelOrderSignal>();
    EXPECT_TRUE(cancel_order);
    EXPECT_EQ("0", cancel_order->order_id);
  }

  {
    auto input_order = PopupRntOrder<InputOrder>();
    EXPECT_EQ("2", input_order->order_id);
    EXPECT_EQ(1, input_order->qty_);
  }
}
