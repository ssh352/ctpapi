#include "gtest/gtest.h"
#include "strategy_fixture.h"

#include "follow_strategy/delayed_open_strategy.h"
#include "unittest_helper.h"

const std::string master_account_id = "master";
const std::string slave_account_id = "slave";
const int delayed_open_after_seconds = 60;
const std::string defalut_instrument_id = "I1";
int default_market_tick_qty = 10;

class DelayedOpenStrategyFixture : public StrategyFixture {
 public:
  void MasterNewOpenAndFill(const std::string& order_id,
                            OrderDirection direction,
                            double price,
                            int qty,
                            int trading_qty) {
    MasterNewOpenOrder(order_id, direction, price, qty);
    MasterTradedOrder(order_id, trading_qty);
  }

  void MasterNewCloseAndFill(const std::string& order_id,
                             OrderDirection direction,
                             double price,
                             int qty,
                             int trading_qty) {
    MasterNewCloseOrder(order_id, direction, price, qty);
    MasterTradedOrder(order_id, trading_qty);
  }

  void MasterNewOpenOrder(const std::string& order_id,
                          OrderDirection direction,
                          double price,
                          int qty) {
    Send(CTASignalAtom::value,
         MakeNewOpenOrder(master_account_id, order_id, defalut_instrument_id,
                          direction, price, qty));
  }

  void MasterNewCloseOrder(const std::string& order_id,
                           OrderDirection direction,
                           double price,
                           int qty) {
    Send(CTASignalAtom::value,
         MakeNewCloseOrder(master_account_id, order_id, defalut_instrument_id,
                           direction, price, qty));
  }

  void MasterTradedOrder(const std::string& order_id, int trading_qty) {
    Send(CTASignalAtom::value,
         MakeTradedOrder(master_account_id, order_id, trading_qty));
  }

  void MasterTradedOrderWithTradingPrice(const std::string& order_id,
                                         double trading_price,
                                         int trading_qty) {
    Send(CTASignalAtom::value, MakeTradedOrder(master_account_id, order_id,
                                               trading_price, trading_qty));
  }

  void MasterCancelOrder(const std::string& order_id) {
    Send(CTASignalAtom::value, MakeCanceledOrder(master_account_id, order_id));
  }

  // void NewOpenAndFill(const std::string& order_id,
  //                    OrderDirection direction,
  //                    double price,
  //                    int qty,
  //                    int trading_qty) {
  //  NewOpenOrder(order_id, direction, price, qty);
  //  TradedOrder(order_id, trading_qty);
  //}

  // void NewCloseAndFill(const std::string& order_id,
  //                     OrderDirection direction,
  //                     double price,
  //                     int qty,
  //                     int trading_qty) {
  //  NewCloseOrder(order_id, direction, price, qty);
  //  TradedOrder(order_id, trading_qty);
  //}

  // void NewOpenOrder(const std::string& order_id,
  //                  OrderDirection direction,
  //                  double price,
  //                  int qty) {
  //  Send(MakeNewOpenOrder(slave_account_id, order_id, defalut_instrument_id,
  //                        direction, price, qty));
  //}

  // void NewCloseOrder(const std::string& order_id,
  //                   OrderDirection direction,
  //                   double price,
  //                   int qty) {
  //  Send(MakeNewCloseOrder(slave_account_id, order_id, defalut_instrument_id,
  //                         direction, price, qty));
  //}

  void TradedOrder(const std::string& order_id, int trading_qty) {
    Send(MakeTradedOrder(slave_account_id, order_id, trading_qty));
  }

  void TradedOrderWithTradingPrice(const std::string& order_id,
                                   double trading_price,
                                   int trading_qty) {
    Send(MakeTradedOrder(slave_account_id, order_id, trading_price,
                         trading_qty));
  }

  void MarketTick(double last_price) {
    Send(MakeTick(defalut_instrument_id, last_price, default_market_tick_qty,
                  now_timestamp_));
  }

  void MarketTick(double last_price, double bid_price, double ask_price) {
    Send(MakeTick(defalut_instrument_id, last_price, bid_price, ask_price,
                  default_market_tick_qty, now_timestamp_));
  }

  void RiskEvent_CloseMarketNear() { Send(CloseMarketNearAtom::value); }

 protected:
  virtual void SetUp() override {
    DelayedOpenStrategy<UnittestMailBox>::StrategyParam param{
        delayed_open_after_seconds, 0};
    CreateStrategy<DelayedOpenStrategy<UnittestMailBox> >(
        master_account_id, slave_account_id, std::move(param),
        defalut_instrument_id);
  }
};

TEST_F(DelayedOpenStrategyFixture, DelayedOpen) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 88.8, 10);

  EXPECT_FALSE(PopupRntOrder<InputOrder>());

  MasterTradedOrder("0", 5);

  EXPECT_FALSE(PopupRntOrder<InputOrder>());

  ElapseMillisecond(delayed_open_after_seconds * 1000 - 1);

  MarketTick(88.8);

  EXPECT_FALSE(PopupRntOrder<InputOrder>());

  ElapseMillisecond(1);

  MarketTick(88.8);
  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ(88.8, input_order->price_);
    EXPECT_EQ(input_order->qty_, 5);
  }
}

TEST_F(DelayedOpenStrategyFixture, OpenOrderTwice) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 88.8, 10);

  EXPECT_FALSE(PopupRntOrder<InputOrder>());

  MasterTradedOrder("0", 6);

  EXPECT_FALSE(PopupRntOrder<InputOrder>());

  ElapseMillisecond(1);

  MasterTradedOrder("0", 4);

  ElapseMillisecond(delayed_open_after_seconds * 1000 - 1);

  MarketTick(88.8);

  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ(88.8, input_order->price_);
    EXPECT_EQ(6, input_order->qty_);
    EXPECT_EQ("0", input_order->order_id);
  }

  ElapseMillisecond(1);

  MarketTick(88.8);

  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ(88.8, input_order->price_);
    EXPECT_EQ(4, input_order->qty_);
    EXPECT_EQ("1", input_order->order_id);
  }
}

TEST_F(DelayedOpenStrategyFixture, Close) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);

  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(88.8);
  Clear();

  // NewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);
  TradedOrder("0", 10);

  MasterNewCloseOrder("1", OrderDirection::kSell, 123.1, 10);

  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ(OrderDirection::kSell, input_order->order_direction_);
    EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);
    EXPECT_EQ(10, input_order->qty_);
  }
}

TEST_F(DelayedOpenStrategyFixture, PartiallyClose) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);

  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(88.8);
  Clear();

  TradedOrder("0", 10);

  MasterNewCloseOrder("1", OrderDirection::kSell, 123.1, 5);

  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ(OrderDirection::kSell, input_order->order_direction_);
    EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);
    EXPECT_EQ(5, input_order->qty_);
  }
}

TEST_F(DelayedOpenStrategyFixture, DonotCloseUntilFillOpenOrder) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);

  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(88.8);
  Clear();
  // NewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);
  // TradedOrder("0", 10);

  MasterNewCloseOrder("1", OrderDirection::kSell, 123.1, 10);

  {
    auto input_order = PopupRntOrder<InputOrder>();
    EXPECT_FALSE(input_order);
  }
}

TEST_F(DelayedOpenStrategyFixture, CancelOpenOrderCase1) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);

  ElapseSeconds(delayed_open_after_seconds);

  MarketTick(88.8);
  // NewOpenOrder("0", OrderDirection::kBuy, 88.8, 10);

  Clear();

  MasterNewCloseOrder("1", OrderDirection::kSell, 88.8, 10);

  EXPECT_FALSE(PopupRntOrder<InputOrder>());

  MasterTradedOrder("1", 10);
  auto cancel_order = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel_order);
  EXPECT_EQ("0", cancel_order->order_id);
}

TEST_F(DelayedOpenStrategyFixture, CancelOpenOrderCase2) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);

  ElapseSeconds(delayed_open_after_seconds);

  MarketTick(88.8);

  // NewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 4);
  TradedOrder("0", 4);
  Clear();

  MasterNewCloseOrder("1", OrderDirection::kSell, 88.8, 10);

  {
    auto order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(order);
    EXPECT_EQ(4, order->qty_);
    EXPECT_EQ("1", order->order_id);
  }

  // NewCloseOrder("1", OrderDirection::kSell, 88.8, 4);

  MasterTradedOrder("1", 10);

  {
    auto cancel_order = PopupRntOrder<CancelOrderSignal>();
    ASSERT_TRUE(cancel_order);
    EXPECT_EQ("0", cancel_order->order_id);
  }
}

TEST_F(DelayedOpenStrategyFixture, CancelOpenOrderCase3) {
  // Prepare
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(88.8);

  // NewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 4);
  TradedOrder("0", 4);
  Clear();

  MasterNewCloseOrder("1", OrderDirection::kSell, 88.8, 10);

  {
    auto order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(order);
    EXPECT_EQ(4, order->qty_);
    EXPECT_EQ("1", order->order_id);
  }

  // NewCloseOrder("1", OrderDirection::kSell, 88.8, 4);
  MasterTradedOrder("1", 9);

  // 1(long) 1(closeing)
  // ==========================
  // 4(long) 6(opening)

  {
    auto cancel_order = PopupRntOrder<CancelOrderSignal>();
    ASSERT_TRUE(cancel_order);
    EXPECT_EQ("0", cancel_order->order_id);
  }

  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ("2", input_order->order_id);
    EXPECT_EQ(1, input_order->qty_);
  }
}

TEST_F(DelayedOpenStrategyFixture, MultiUnfillOrderForCancel_Sell_Case1) {
  MasterNewOpenAndFill("0", OrderDirection::kSell, 88.8, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(88.8);
  Clear();
  // NewOpenOrder("0", OrderDirection::kSell, 88.8, 10);

  MasterNewOpenAndFill("1", OrderDirection::kSell, 80.0, 5, 5);

  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(78.0);
  // NewOpenOrder("1", OrderDirection::kSell, 80.0, 5);
  Clear();

  MasterNewCloseAndFill("2", OrderDirection::kBuy, 74.0, 10, 4);

  auto cancel = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel);
  EXPECT_EQ("0", cancel->order_id);

  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect_);
  EXPECT_EQ(OrderDirection::kSell, input_order->order_direction_);
  EXPECT_EQ(88.8, input_order->price_);
  EXPECT_EQ(6, input_order->qty_);
}

TEST_F(DelayedOpenStrategyFixture, MultiUnfillOrderForCancel_Sell_Case2) {
  // Prepare status
  MasterNewOpenAndFill("0", OrderDirection::kSell, 80.8, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(79.8);
  Clear();
  // NewOpenOrder("0", OrderDirection::kSell, 80.8, 10);

  MasterNewOpenAndFill("1", OrderDirection::kSell, 81.0, 5, 5);

  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(78.0);
  // NewOpenOrder("1", OrderDirection::kSell, 81.0, 5);
  Clear();

  // OrderBook:
  // 81.0, 80.8 |
  // EXPECT
  MasterNewCloseAndFill("2", OrderDirection::kBuy, 74.0, 10, 4);

  auto cancel = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel);
  EXPECT_EQ("1", cancel->order_id);

  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect_);
  EXPECT_EQ(OrderDirection::kSell, input_order->order_direction_);
  EXPECT_EQ(81.0, input_order->price_);
  EXPECT_EQ(1, input_order->qty_);
}

TEST_F(DelayedOpenStrategyFixture, MultiUnfillOrder_Buy_Cancel_Lessers) {
  // Prepare Data
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 80.8, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(81.8);
  Clear();
  // NewOpenOrder("0", OrderDirection::kBuy, 80.8, 10);

  MasterNewOpenAndFill("1", OrderDirection::kBuy, 81.0, 5, 5);

  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(82.0);
  // NewOpenOrder("1", OrderDirection::kBuy, 81.0, 5);
  Clear();

  // OrderBook
  // | 81.0 80.8
  MasterNewCloseAndFill("2", OrderDirection::kSell, 74.0, 10, 4);

  auto cancel = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel);
  EXPECT_EQ("0", cancel->order_id);

  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(80.8, input_order->price_);
  EXPECT_EQ(6, input_order->qty_);
}

TEST_F(DelayedOpenStrategyFixture,
       MultiUnfillOrder_Buy_Cancel_All_Reopen_leaves) {
  // Prepare Data
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 80.8, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(81.8);
  Clear();
  // NewOpenOrder("0", OrderDirection::kBuy, 80.8, 10);

  MasterNewOpenAndFill("1", OrderDirection::kBuy, 81.0, 5, 5);

  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(82.8);
  // NewOpenOrder("1", OrderDirection::kBuy, 81.0, 5);
  Clear();

  // OrderBook
  // | 81.0 80.8
  MasterNewCloseAndFill("2", OrderDirection::kSell, 74.0, 15, 14);

  {
    auto cancel = PopupRntOrder<CancelOrderSignal>();
    ASSERT_TRUE(cancel);
    EXPECT_EQ("0", cancel->order_id);
  }

  {
    auto cancel = PopupRntOrder<CancelOrderSignal>();
    ASSERT_TRUE(cancel);
    EXPECT_EQ("1", cancel->order_id);
  }

  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(81.0, input_order->price_);
  EXPECT_EQ(1, input_order->qty_);
}

TEST_F(DelayedOpenStrategyFixture, CancelOpeningOrderTwice) {
  // Prepare Data
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 80.8, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(80.8);
  Clear();
  // NewOpenOrder("0", OrderDirection::kBuy, 80.8, 10);
  Clear();

  MasterNewCloseAndFill("1", OrderDirection::kSell, 80.0, 10, 7);

  {
    auto cancel = PopupRntOrder<CancelOrderSignal>();
    ASSERT_TRUE(cancel);
    EXPECT_EQ("0", cancel->order_id);
  }

  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect_);
    EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
    EXPECT_EQ(80.8, input_order->price_);
    EXPECT_EQ(3, input_order->qty_);
    EXPECT_EQ("1", input_order->order_id);
  }

  // NewCloseOrder("1", OrderDirection::kBuy, 80.8, 3);

  MasterTradedOrder("1", 3);

  {
    auto cancel = PopupRntOrder<CancelOrderSignal>();
    ASSERT_TRUE(cancel);
    EXPECT_EQ("1", cancel->order_id);
  }

  EXPECT_FALSE(PopupRntOrder<InputOrder>());
}

TEST_F(DelayedOpenStrategyFixture, OppositeOpening) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(123.0);
  TradedOrder("0", 10);
  Clear();

  MasterNewOpenOrder("1", OrderDirection::kSell, 123.0, 10);
  auto input_order = PopupRntOrder<InputOrder>();
  EXPECT_TRUE(input_order);
  EXPECT_EQ("1", input_order->order_id);
  EXPECT_EQ(OrderDirection::kSell, input_order->order_direction_);
  EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect_);
  EXPECT_EQ(123.0, input_order->price_);
  EXPECT_EQ(10, input_order->qty_);
}

TEST_F(DelayedOpenStrategyFixture, OppositeOpening_Cancel) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(123.0);
  Clear();

  MasterNewOpenOrder("1", OrderDirection::kSell, 123.0, 10);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("1", 10);
  auto cancel_order = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel_order);
  EXPECT_EQ("0", cancel_order->order_id);
}

TEST_F(DelayedOpenStrategyFixture, OppositeOpening_Cancel_Bed_Order) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(124.0);

  MasterNewOpenAndFill("1", OrderDirection::kBuy, 120.1, 5, 5);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(125.0);

  Clear();

  MasterNewOpenOrder("2", OrderDirection::kSell, 123.0, 4);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("2", 4);

  {
    auto cancel_order = PopupRntOrder<CancelOrderSignal>();
    ASSERT_TRUE(cancel_order);
    EXPECT_EQ("1", cancel_order->order_id);
  }

  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ("2", input_order->order_id);
    EXPECT_EQ(1, input_order->qty_);
    EXPECT_EQ(120.1, input_order->price_);
  }
}

TEST_F(DelayedOpenStrategyFixture, CloseBeforeOpening_DontInputOrder) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);
  ElapseMillisecond(1);
  MasterNewCloseAndFill("1", OrderDirection::kSell, 123.0, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(120.1);

  EXPECT_FALSE(PopupRntOrder<InputOrder>());
}

TEST_F(DelayedOpenStrategyFixture, CloseBeforeOpening_OnlyNeedOpengOneOrder) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);
  ElapseMillisecond(1);
  MasterNewOpenAndFill("1", OrderDirection::kBuy, 120.1, 5, 5);
  ElapseMillisecond(1);
  MasterNewCloseAndFill("2", OrderDirection::kSell, 121.1, 15, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(124.2, 124.2, 120.0);

  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ("0", input_order->order_id);
  EXPECT_EQ(123.1, input_order->price_);
  EXPECT_EQ(5, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
}

TEST_F(DelayedOpenStrategyFixture, RemovePendingOpenAndCancelOrder) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(125.1);
  Clear();
  MasterNewOpenAndFill("1", OrderDirection::kBuy, 120.1, 5, 5);
  ElapseMillisecond(1);
  MasterNewCloseAndFill("2", OrderDirection::kSell, 121.1, 15, 6);

  auto cancel_order = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel_order);
  EXPECT_EQ("0", cancel_order->order_id);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ("1", input_order->order_id);
  EXPECT_EQ(123.1, input_order->price_);
  EXPECT_EQ(9, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);

  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(121.1);
  EXPECT_FALSE(PopupRntOrder<InputOrder>());
}

TEST_F(DelayedOpenStrategyFixture, MinusOpening) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);
  // ElapseSeconds(delayed_open_after_seconds);
  ElapseMillisecond(1);
  MasterNewCloseAndFill("1", OrderDirection::kSell, 121.1, 10, 1);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(125.1);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ("0", input_order->order_id);
  EXPECT_EQ(123.1, input_order->price_);
  EXPECT_EQ(9, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
}

TEST_F(DelayedOpenStrategyFixture, RishMananger_CloseOrder_kSell) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(123.1);
  TradedOrder("0", 10);
  ElapseMillisecond(1);
  MarketTick(128.1, 127.0, 129.2);
  MasterNewCloseAndFill("1", OrderDirection::kSell, 121.1, 10, 10);
  Clear();
  RiskEvent_CloseMarketNear();

  auto cancel_order = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel_order);
  EXPECT_EQ("1", cancel_order->order_id);

  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(127.0, input_order->price_);
  EXPECT_EQ(10, input_order->qty_);
}

TEST_F(DelayedOpenStrategyFixture, RishMananger_PartiallyCloseOrder_kSell) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 123.1, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(123.1);
  TradedOrder("0", 10);
  ElapseMillisecond(1);
  MarketTick(128.1, 127.0, 129.2);
  MasterNewCloseAndFill("1", OrderDirection::kSell, 121.1, 10, 7);
  Clear();
  RiskEvent_CloseMarketNear();

  auto cancel_order = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel_order);
  EXPECT_EQ("1", cancel_order->order_id);

  do {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ(121.1, input_order->price_);
    EXPECT_EQ(3, input_order->qty_);
  } while (0);

  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(127.0, input_order->price_);
  EXPECT_EQ(7, input_order->qty_);
}

TEST_F(DelayedOpenStrategyFixture, RishMananger_CloseOrder_kBuy) {
  MasterNewOpenAndFill("0", OrderDirection::kSell, 123.1, 10, 10);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(123.1);
  TradedOrder("0", 10);
  ElapseMillisecond(1);
  MarketTick(128.1, 127.0, 129.2);
  MasterNewCloseAndFill("1", OrderDirection::kBuy, 121.1, 10, 10);
  Clear();
  RiskEvent_CloseMarketNear();

  auto cancel_order = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel_order);
  EXPECT_EQ("1", cancel_order->order_id);

  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(129.2, input_order->price_);
  EXPECT_EQ(10, input_order->qty_);
}
