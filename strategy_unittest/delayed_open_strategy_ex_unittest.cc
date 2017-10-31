#include "gtest/gtest.h"
#include "strategy_fixture.h"

#include "follow_strategy/delayed_open_strategy_ex.h"
#include "unittest_helper.h"
#include "hpt_core/portfolio.h"

const static std::string master_account_id = "master";
const static std::string slave_account_id = "slave";
const static int delayed_open_after_seconds = 60;
const static std::string defalut_instrument_id = "I1";
const static int default_market_tick_qty = 10;

class DelayedOpenStrategyExFixture : public StrategyFixture {
 public:
  template <typename... Ts>
  void MasterNewOpenOrder(const std::string& order_id,
                          OrderDirection direction,
                          double price,
                          int qty,
                          Ts&&... params) {
    Send(MakeNewOpenOrder(master_account_id, order_id, defalut_instrument_id,
                          direction, price, qty),
         CTAPositionQty{std::forward<Ts>(params)...});
  }

  template <typename... Ts>
  void MasterNewCloseOrder(const std::string& order_id,
                           OrderDirection direction,
                           double price,
                           int qty,
                           Ts&&... params) {
    Send(MakeNewCloseOrder(master_account_id, order_id, defalut_instrument_id,
                           direction, price, qty),
         CTAPositionQty{std::forward<Ts>(params)...});
  }

  template <typename... Ts>
  void MasterTradedOrder(const std::string& order_id,
                         int trading_qty,
                         Ts&&... params) {
    Send(MakeTradedOrder(master_account_id, order_id, trading_qty),
         CTAPositionQty{std::forward<Ts>(params)...});
  }

  template <typename... Ts>
  void MasterTradedOrderWithTradingPrice(const std::string& order_id,
                                         double trading_price,
                                         int trading_qty,
                                         Ts&&... params) {
    Send(MakeTradedOrder(master_account_id, order_id, trading_qty),
         CTAPositionQty{std::forward<Ts>(params)...});
  }

  template <typename... Ts>
  void MasterCancelOrder(const std::string& order_id, Ts&&... params) {
    auto order = MakeCanceledOrder(master_account_id, order_id);
    Send(order, CTAPositionQty{std::forward<Ts>(params)...});
  }

  void MarketTick(double last_price) {
    Send(MakeTick(defalut_instrument_id, last_price, default_market_tick_qty,
                  now_timestamp_));
  }

  void MarketTick(double last_price, double bid_price, double ask_price) {
    Send(MakeTick(defalut_instrument_id, last_price, bid_price, ask_price,
                  default_market_tick_qty, now_timestamp_));
  }

  void TradedOrder(const std::string& order_id, int trading_qty) {
    Send(MakeTradedOrder(slave_account_id, order_id, trading_qty));
  }

  void TradedOrderWithTradingPrice(const std::string& order_id,
                                   double trading_price,
                                   int trading_qty) {
    Send(MakeTradedOrder(slave_account_id, order_id, trading_price,
                         trading_qty));
  }

 protected:
  virtual void SetUp() override {
    DelayedOpenStrategyEx::StrategyParam param{delayed_open_after_seconds, 0};
    CreateStrategy<DelayOpenStrategyAgent<UnittestMailBox> >(
        std::move(param), defalut_instrument_id);
  }
};

TEST_F(DelayedOpenStrategyExFixture, Open_Order) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.2);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect_);
  EXPECT_EQ(1.1, input_order->price_);
  EXPECT_EQ(10, input_order->qty_);
  EXPECT_EQ("0", input_order->order_id);
}

TEST_F(DelayedOpenStrategyExFixture, Closeing_Fully_Position) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.2);
  TradedOrder("0", 10);
  Clear();
  MasterNewCloseOrder("1", OrderDirection::kBuy, 1.1, 10, 10, 10);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(1.1, input_order->price_);
  EXPECT_EQ(10, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);
}

TEST_F(DelayedOpenStrategyExFixture, CTAFullyCloseNoDelayHadOpeningHadPos) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.2);
  TradedOrder("0", 4);
  Clear();
  MasterNewCloseOrder("1", OrderDirection::kBuy, 1.1, 10, 10, 10);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(1.1, input_order->price_);
  EXPECT_EQ(4, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);
  ASSERT_FALSE(PopupRntOrder<CancelOrderSignal>());

  MasterTradedOrder("1", 10, 0, 0);
  auto cancel_order = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel_order);
  EXPECT_EQ("0", cancel_order->order_id);
}
TEST_F(DelayedOpenStrategyExFixture, CTAFullyCloseHadDelayHadOpeningHadPos) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  MasterTradedOrder("0", 10, 10, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.2);
  TradedOrder("0", 4);

  MasterNewOpenOrder("1", OrderDirection::kBuy, 1.1, 5, 10, 0);
  MasterTradedOrder("1", 5, 15, 0);
  ElapseMillisecond(1);
  MarketTick(1.2);
  Clear();

  MasterNewCloseOrder("2", OrderDirection::kBuy, 1.1, 15, 15, 15);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(1.1, input_order->price_);
  EXPECT_EQ(4, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);
  ASSERT_FALSE(PopupRntOrder<CancelOrderSignal>());

  MasterTradedOrder("2", 15, 0, 0);
  auto cancel_order = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel_order);
  EXPECT_EQ("0", cancel_order->order_id);

  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.3);
  EXPECT_FALSE(PopupRntOrder<InputOrder>());
}

TEST_F(DelayedOpenStrategyExFixture, CTAPartiallyCloseQtyLessOrEqualToThanPos) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.2);
  TradedOrder("0", 10);
  Clear();

  MasterNewCloseOrder("1", OrderDirection::kBuy, 1.0, 4, 10, 4);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(1.0, input_order->price_);
  EXPECT_EQ(4, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);

  ASSERT_FALSE(PopupRntOrder<CancelOrderSignal>());
}

TEST_F(DelayedOpenStrategyExFixture,
       CTAPartiallyCloseNoDelaySingleOpeningAndQtyGreaterToThanPos) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.2);
  TradedOrder("0", 6);
  Clear();

  MasterNewCloseOrder("1", OrderDirection::kBuy, 1.0, 7, 10, 7);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(1.0, input_order->price_);
  EXPECT_EQ(6, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);

  MasterTradedOrder("1", 7, 3, 0);
  auto cancel_order = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel_order);
  EXPECT_EQ("0", cancel_order->order_id);
  EXPECT_EQ(1, cancel_order->qty);
}

TEST_F(DelayedOpenStrategyExFixture,
       CTAPartiallyCloseNoDelayMultiOpeningAndQtyGreaterToThanPos_Ascending) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  MasterTradedOrder("0", 10, 10, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.2);
  TradedOrder("0", 4);

  MasterNewOpenOrder("1", OrderDirection::kBuy, 1.6, 5, 10, 0);
  MasterTradedOrder("1", 5, 15, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.7);
  TradedOrder("1", 3);
  Clear();

  MasterNewCloseOrder("2", OrderDirection::kBuy, 1.3, 11, 15, 11);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(1.3, input_order->price_);
  EXPECT_EQ(7, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);

  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("2", 11, 4, 0);
  auto cancel_order = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel_order);
  EXPECT_EQ("0", cancel_order->order_id);
  EXPECT_EQ(4, cancel_order->qty);

  EXPECT_FALSE(PopupRntOrder<CancelOrderSignal>());
}

TEST_F(DelayedOpenStrategyExFixture,
       CTAPartiallyCloseNoDelayMultiOpeningAndQtyGreaterToThanPos_Descending) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.6, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.7);
  TradedOrder("0", 4);
  Clear();

  MasterNewOpenOrder("1", OrderDirection::kBuy, 1.2, 5, 10, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("1", 5, 15, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.3);
  TradedOrder("1", 3);
  Clear();

  MasterNewCloseOrder("2", OrderDirection::kBuy, 1.3, 11, 15, 11);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(1.3, input_order->price_);
  EXPECT_EQ(7, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);

  ASSERT_FALSE(PopupRntOrder<CancelOrderSignal>());
  MasterTradedOrder("2", 11, 4, 0);

  {
    auto cancel_order = PopupRntOrder<CancelOrderSignal>();
    ASSERT_TRUE(cancel_order);
    EXPECT_EQ("1", cancel_order->order_id);
    EXPECT_EQ(2, cancel_order->qty);
  }
  {
    auto cancel_order = PopupRntOrder<CancelOrderSignal>();
    ASSERT_TRUE(cancel_order);
    EXPECT_EQ("0", cancel_order->order_id);
    EXPECT_EQ(2, cancel_order->qty);
  }
}

TEST_F(DelayedOpenStrategyExFixture,
       CTAPartiallyCloseSingleDelayNoOpeningAndQtyGreaterToThanPos) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  ElapseMillisecond(1);
  MarketTick(1.2);
  Clear();

  MasterNewCloseOrder("1", OrderDirection::kBuy, 1.0, 4, 10, 4);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("1", 4, 6, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.3);

  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ("0", input_order->order_id);
  EXPECT_EQ(1.1, input_order->price_);
  EXPECT_EQ(6, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect_);
}
TEST_F(DelayedOpenStrategyExFixture,
       CTAPartiallyCloseMultiDelayNoOpeningAndQtyGreaterToThanPos_Descending) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.6, 4, 0, 0);
  MasterTradedOrder("0", 4, 4, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.7);
  TradedOrder("0", 4);

  MasterNewOpenOrder("1", OrderDirection::kBuy, 1.4, 10, 4, 0);
  MasterTradedOrder("1", 10, 14, 0);
  ElapseMillisecond(1);
  MarketTick(1.3);

  MasterNewOpenOrder("2", OrderDirection::kBuy, 1.0, 5, 14, 0);
  MasterTradedOrder("2", 5, 19, 0);
  ElapseMillisecond(1);
  MarketTick(1.3);
  Clear();

  MasterNewCloseOrder("3", OrderDirection::kBuy, 1.3, 7, 19, 7);
  auto input_order = PopupRntOrder<InputOrder>();
  ASSERT_TRUE(input_order);
  EXPECT_EQ(1.3, input_order->price_);
  EXPECT_EQ(4, input_order->qty_);
  EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
  EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);
  MasterTradedOrder("3", 7, 12, 0);

  EXPECT_FALSE(PopupRntOrder<CancelOrderSignal>());
  EXPECT_FALSE(PopupRntOrder<InputOrder>());
  ElapseSeconds(delayed_open_after_seconds);

  MarketTick(1.6);
  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ(1.4, input_order->price_);
    EXPECT_EQ(10, input_order->qty_);
    EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
    EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect_);
  }

  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ(1.0, input_order->price_);
    EXPECT_EQ(2, input_order->qty_);
    EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
    EXPECT_EQ(PositionEffect::kOpen, input_order->position_effect_);
  }
}

TEST_F(DelayedOpenStrategyExFixture,
       CTAPartiallyCloseMultiDelayMultiOpeningAndQtyGreaterToThanPos) {}

TEST_F(DelayedOpenStrategyExFixture, CancelCloseOrder) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.1, 10, 0, 0);
  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  MasterTradedOrder("0", 10, 10, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.2);
  TradedOrder("0", 10);
  MasterNewCloseOrder("1", OrderDirection::kBuy, 1.0, 4, 10, 4);
  Clear();

  MasterCancelOrder("1", 10, 0);
  auto cancel = PopupRntOrder<CancelOrderSignal>();
  ASSERT_TRUE(cancel);
  EXPECT_EQ("1", cancel->order_id);
  EXPECT_EQ(4, cancel->qty);
}

TEST_F(DelayedOpenStrategyExFixture, OutstandingOrderForSameDirection) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.6, 4, 0, 0);
  MasterTradedOrder("0", 4, 4, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.7);
  TradedOrder("0", 4);
  Clear();

  auto_reply_new_rtn_order = false;
  MasterNewCloseOrder("1", OrderDirection::kBuy, 1.3, 1, 4, 1);
  MasterNewCloseOrder("2", OrderDirection::kBuy, 1.5, 3, 4, 4);

  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ("1", input_order->order_id);
    EXPECT_EQ(1.3, input_order->price_);
    EXPECT_EQ(1, input_order->qty_);
    EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
    EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);
  }

  ASSERT_FALSE(PopupRntOrder<InputOrder>());
  SendAndClearPendingReplyRtnOrder();
  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ(1.5, input_order->price_);
    EXPECT_EQ(3, input_order->qty_);
    EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
    EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);
  }

}
TEST_F(DelayedOpenStrategyExFixture, OutstandingOrderForDifferentDirection) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 1.6, 4, 0, 0);
  MasterTradedOrder("0", 4, 4, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.7);
  TradedOrder("0", 4);

  MasterNewOpenOrder("1", OrderDirection::kSell, 1.6, 5, 0, 0);
  MasterTradedOrder("1", 5, 5, 0);
  ElapseSeconds(delayed_open_after_seconds);
  MarketTick(1.7);
  TradedOrder("1", 5);
  Clear();

  MasterNewCloseOrder("2", OrderDirection::kBuy, 1.3, 4, 4, 4);
  MasterNewCloseOrder("3", OrderDirection::kSell, 1.5, 5, 5, 5);
  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ("2", input_order->order_id);
    EXPECT_EQ(1.3, input_order->price_);
    EXPECT_EQ(4, input_order->qty_);
    EXPECT_EQ(OrderDirection::kBuy, input_order->order_direction_);
    EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);
  }
  {
    auto input_order = PopupRntOrder<InputOrder>();
    ASSERT_TRUE(input_order);
    EXPECT_EQ(1.5, input_order->price_);
    EXPECT_EQ(5, input_order->qty_);
    EXPECT_EQ(OrderDirection::kSell, input_order->order_direction_);
    EXPECT_EQ(PositionEffect::kClose, input_order->position_effect_);
  }
}
