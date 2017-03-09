#include "gtest/gtest.h"
#include "geek_quant/instrument_follow.h"
#include "geek_quant/caf_defines.h"

class InstrumentFollowFixture : public testing::Test {
 public:
 protected:
  OrderRtnData MakeOrderRtnData(const std::string&& order_no,
                                OrderDirection order_direction,
                                OrderStatus order_status,
                                double order_price = 1234.1,
                                int volume = 10,
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
      MakeOrderRtnData("0002", OrderDirection::kODSell, OrderStatus::kOSCloseing),
      &enter_order, &cancel_order_no_list);

  EXPECT_EQ("abc", enter_order.instrument);
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(OrderDirection::kODSell, enter_order.order_direction);
  EXPECT_EQ(EnterOrderAction::kEOAClose, enter_order.action);
  EXPECT_EQ(1234.1, enter_order.order_price);
  EXPECT_EQ(10, enter_order.volume);
}
