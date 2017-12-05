#include "gtest/gtest.h"
#include "ctp_instrument_broker_test.h"

class CloseTodayAwareTest : public CTPInstrumentBrokerTest {
 public:
  CloseTodayAwareTest() {
    broker_.SetPositionEffectStrategy<
        GenericCTPPositionEffectStrategy,
        CloseTodayAwareCTPPositionEffectFlagStrategy>();
  }
};

TEST_F(CloseTodayAwareTest, CloseYesterday) {
  broker_.InitPosition(std::make_pair(10, 5), std::make_pair(0, 0));

  MakeCloseOrderRequest("abc", OrderDirection::kSell, 1.2, 10);

  auto enter_order = PopupOrder<CTPEnterOrder>();
  ASSERT_TRUE(enter_order);
  EXPECT_EQ(CTPPositionEffect::kClose, enter_order->position_effect);
  EXPECT_EQ(OrderDirection::kSell, enter_order->direction);
  EXPECT_EQ(10, enter_order->qty);
  EXPECT_EQ(1.2, enter_order->price);
  EXPECT_EQ("I1", enter_order->instrument);
  EXPECT_EQ("0", enter_order->order_id);
}

TEST_F(CloseTodayAwareTest, kCloseToday) {
  broker_.InitPosition(std::make_pair(0, 10), std::make_pair(0, 0));

  MakeCloseOrderRequest("abc", OrderDirection::kSell, 1.2, 10);

  auto enter_order = PopupOrder<CTPEnterOrder>();
  ASSERT_TRUE(enter_order);
  EXPECT_EQ(CTPPositionEffect::kCloseToday, enter_order->position_effect);
  EXPECT_EQ(OrderDirection::kSell, enter_order->direction);
  EXPECT_EQ(10, enter_order->qty);
  EXPECT_EQ(1.2, enter_order->price);
  EXPECT_EQ("I1", enter_order->instrument);
  EXPECT_EQ("0", enter_order->order_id);
}

TEST_F(CloseTodayAwareTest, CloseOrderNoEnoughYesterdayQty) {
  broker_.InitPosition(std::make_pair(5, 10), std::make_pair(0, 0));
  MakeCloseOrderRequest("abc", OrderDirection::kSell, 1.2, 10);
  {
    auto enter_order = PopupOrder<CTPEnterOrder>();
    ASSERT_TRUE(enter_order);
    EXPECT_EQ(CTPPositionEffect::kClose, enter_order->position_effect);
    EXPECT_EQ(OrderDirection::kSell, enter_order->direction);
    EXPECT_EQ(5, enter_order->qty);
    EXPECT_EQ("0", enter_order->order_id);
  }

  {
    auto enter_order = PopupOrder<CTPEnterOrder>();
    ASSERT_TRUE(enter_order);
    EXPECT_EQ(CTPPositionEffect::kCloseToday, enter_order->position_effect);
    EXPECT_EQ(OrderDirection::kSell, enter_order->direction);
    EXPECT_EQ(5, enter_order->qty);
    EXPECT_EQ("1", enter_order->order_id);
  }
}

TEST_F(CloseTodayAwareTest,
       CloseOrderNoEnoughYesterdayQtyShouldOnlyOneRtnOrder) {
  broker_.InitPosition(std::make_pair(5, 10), std::make_pair(0, 0));
  MakeCloseOrderRequest("abc", OrderDirection::kSell, 1.2, 10);
  Clear();
  SimulateCTPNewCloseOrderField("0", OrderDirection::kSell, 1.2, 5);

  EXPECT_FALSE(PopupOrder<std::shared_ptr<OrderField>>());
  SimulateCTPNewCloseOrderField("1", OrderDirection::kSell, 1.2, 5);
  auto order = PopupOrder<std::shared_ptr<OrderField>>();
  ASSERT_TRUE(order);
  EXPECT_EQ("abc", (*order)->order_id);
  EXPECT_EQ(PositionEffect::kClose, (*order)->position_effect);
  EXPECT_EQ(OrderDirection::kBuy, (*order)->position_effect_direction);
  EXPECT_EQ(OrderDirection::kSell, (*order)->direction);
  EXPECT_EQ(OrderStatus::kActive, (*order)->status);
  EXPECT_EQ(10, (*order)->qty);
  EXPECT_EQ(10, (*order)->leaves_qty);
  EXPECT_EQ(0.0, (*order)->trading_price);
  EXPECT_EQ(0, (*order)->trading_qty);
}

TEST_F(CloseTodayAwareTest, CloseOrderNoEnoughYesterdayQtyThenRecv) {
  broker_.InitPosition(std::make_pair(5, 10), std::make_pair(0, 0));
  MakeCloseOrderRequest("abc", OrderDirection::kSell, 1.2, 10);
  SimulateCTPNewCloseOrderField("0", OrderDirection::kSell, 1.2, 5);
  SimulateCTPNewCloseOrderField("1", OrderDirection::kSell, 1.2, 5);
  Clear();
  SimulateCTPTradedOrderField("0", 5);
  {
    auto order = PopupOrder<std::shared_ptr<OrderField>>();
    ASSERT_TRUE(order);
    EXPECT_EQ("abc", (*order)->order_id);
    EXPECT_EQ(PositionEffect::kClose, (*order)->position_effect);
    EXPECT_EQ(OrderDirection::kBuy, (*order)->position_effect_direction);
    EXPECT_EQ(OrderDirection::kSell, (*order)->direction);
    EXPECT_EQ(OrderStatus::kActive, (*order)->status);
    EXPECT_EQ(1.2, (*order)->trading_price);
    EXPECT_EQ(10, (*order)->qty);
    EXPECT_EQ(5, (*order)->trading_qty);
    EXPECT_EQ(5, (*order)->leaves_qty);
  }

  SimulateCTPTradedOrderField("1", 5);
  {
    auto order = PopupOrder<std::shared_ptr<OrderField>>();
    ASSERT_TRUE(order);
    EXPECT_EQ("abc", (*order)->order_id);
    EXPECT_EQ(PositionEffect::kClose, (*order)->position_effect);
    EXPECT_EQ(OrderDirection::kBuy, (*order)->position_effect_direction);
    EXPECT_EQ(OrderDirection::kSell, (*order)->direction);
    EXPECT_EQ(OrderStatus::kAllFilled, (*order)->status);
    EXPECT_EQ(1.2, (*order)->trading_price);
    EXPECT_EQ(10, (*order)->qty);
    EXPECT_EQ(5, (*order)->trading_qty);
    EXPECT_EQ(0, (*order)->leaves_qty);
  }
}

TEST_F(CloseTodayAwareTest, OpenHasOppositionPositionOrder) {
  broker_.InitPosition(std::make_pair(0, 0), std::make_pair(5, 0));
  MakeOpenOrderRequest("abc", OrderDirection::kBuy, 1.2, 5);
  {
    auto enter_order = PopupOrder<CTPEnterOrder>();
    ASSERT_TRUE(enter_order);
    EXPECT_EQ(CTPPositionEffect::kClose, enter_order->position_effect);
    EXPECT_EQ(OrderDirection::kBuy, enter_order->direction);
    EXPECT_EQ(5, enter_order->qty);
    EXPECT_EQ("0", enter_order->order_id);
  }
}
