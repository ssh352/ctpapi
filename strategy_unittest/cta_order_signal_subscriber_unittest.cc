#include <tuple>
#include "gtest/gtest.h"
#include "unittest_helper.h"
#include "strategy_fixture.h"

#include "follow_strategy/cta_order_signal_subscriber.h"

static const std::string master_account_id = "master";
static const std::string defalut_instrument_id = "default instrument";

class CTAOrderSignalScriberFixture : public StrategyFixture {
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

 protected:
  virtual void SetUp() override {
    CreateStrategy<CTAOrderSignalSubscriber<UnittestMailBox> >(
        defalut_instrument_id);
  }
};

TEST_F(CTAOrderSignalScriberFixture, Opening_Fully_Order) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 88.8, 10);

  EXPECT_FALSE((PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>()));
  ElapseSeconds(1);

  MasterTradedOrder("0", 10);

  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
  ASSERT_TRUE(params);
  const auto& order = std::get<0>(*params);
  const auto& position_qty = std::get<1>(*params);

  EXPECT_EQ(10, order->trading_qty);
  EXPECT_EQ(0, order->leaves_qty);
  EXPECT_EQ(88.8, order->trading_price);

  EXPECT_EQ(10, position_qty.position);
  EXPECT_EQ(0, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture, Opening_Partially_Order) {
  MasterNewOpenOrder("0", OrderDirection::kBuy, 88.8, 10);

  EXPECT_FALSE((PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>()));
  ElapseSeconds(1);

  MasterTradedOrder("0", 5);

  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
  ASSERT_TRUE(params);
  const auto& order = std::get<0>(*params);
  const auto& position_qty = std::get<1>(*params);

  EXPECT_EQ(5, order->trading_qty);
  EXPECT_EQ(5, order->leaves_qty);
  EXPECT_EQ(88.8, order->trading_price);

  EXPECT_EQ(5, position_qty.position);
  EXPECT_EQ(0, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture, Opening_Fully_Oppisiton_Direction_Order) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);

  Clear();
  MasterNewOpenOrder("1", OrderDirection::kSell, 80.8, 10);

  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
  ASSERT_TRUE(params);
  const auto& order = std::get<0>(*params);
  const auto& position_qty = std::get<1>(*params);

  EXPECT_EQ(0, order->trading_qty);
  EXPECT_EQ(10, order->leaves_qty);
  EXPECT_EQ(10, order->qty);
  EXPECT_EQ(80.8, order->input_price);
  EXPECT_EQ(OrderDirection::kBuy, order->direction);
  EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  EXPECT_EQ(10, position_qty.position);
  EXPECT_EQ(10, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture,
       Opening_Partially_Oppisiton_Direction_Order) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);

  Clear();
  MasterNewOpenOrder("1", OrderDirection::kSell, 80.8, 6);

  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
  ASSERT_TRUE(params);
  const auto& order = std::get<0>(*params);
  const auto& position_qty = std::get<1>(*params);

  EXPECT_EQ(0, order->trading_qty);
  EXPECT_EQ(6, order->leaves_qty);
  EXPECT_EQ(6, order->qty);
  EXPECT_EQ(80.8, order->input_price);
  EXPECT_EQ(OrderDirection::kBuy, order->direction);
  EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  EXPECT_EQ(10, position_qty.position);
  EXPECT_EQ(6, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture,
       Opening_Over_Qty_Oppisiton_Direction_Order) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);

  Clear();
  MasterNewOpenOrder("1", OrderDirection::kSell, 80.8, 17);

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(0, order->trading_qty);
    EXPECT_EQ(10, order->leaves_qty);
    EXPECT_EQ(10, order->qty);
    EXPECT_EQ(80.8, order->input_price);
    EXPECT_EQ(OrderDirection::kBuy, order->direction);
    EXPECT_EQ(PositionEffect::kClose, order->position_effect);

    EXPECT_EQ(10, position_qty.position);
    EXPECT_EQ(10, position_qty.frozen);
  }

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(0, order->trading_qty);
    EXPECT_EQ(7, order->leaves_qty);
    EXPECT_EQ(7, order->qty);
    EXPECT_EQ(80.8, order->input_price);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);

    EXPECT_EQ(0, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }
}

TEST_F(CTAOrderSignalScriberFixture,
       Traded_All_Over_Qty_Oppisiton_Direction_Order) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);

  Clear();
  MasterNewOpenOrder("1", OrderDirection::kSell, 80.8, 17);
  Clear();
  MasterTradedOrder("1", 17);

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(10, order->trading_qty);
    EXPECT_EQ(0, order->leaves_qty);
    EXPECT_EQ(10, order->qty);
    EXPECT_EQ(80.8, order->input_price);
    EXPECT_EQ(80.8, order->trading_price);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(OrderDirection::kBuy, order->direction);
    EXPECT_EQ(PositionEffect::kClose, order->position_effect);

    EXPECT_EQ(0, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(7, order->trading_qty);
    EXPECT_EQ(0, order->leaves_qty);
    EXPECT_EQ(7, order->qty);
    EXPECT_EQ(80.8, order->input_price);
    EXPECT_EQ(80.8, order->trading_price);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);

    EXPECT_EQ(7, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }
}

TEST_F(
    CTAOrderSignalScriberFixture,
    Traded_Partially_Over_Qty_Oppisiton_Direction_Order_Qty_Greater_Than_Close_Qty) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);

  Clear();
  MasterNewOpenOrder("1", OrderDirection::kSell, 80.8, 17);
  Clear();
  MasterTradedOrder("1", 11);

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(10, order->trading_qty);
    EXPECT_EQ(0, order->leaves_qty);
    EXPECT_EQ(10, order->qty);
    EXPECT_EQ(80.8, order->input_price);
    EXPECT_EQ(80.8, order->trading_price);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(OrderDirection::kBuy, order->direction);
    EXPECT_EQ(PositionEffect::kClose, order->position_effect);

    EXPECT_EQ(0, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(1, order->trading_qty);
    EXPECT_EQ(6, order->leaves_qty);
    EXPECT_EQ(7, order->qty);
    EXPECT_EQ(80.8, order->input_price);
    EXPECT_EQ(80.8, order->trading_price);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);

    EXPECT_EQ(1, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }
}

TEST_F(
    CTAOrderSignalScriberFixture,
    Traded_Partially_Over_Qty_Oppisiton_Direction_Order_Qty_Less_Than_Close_Qty) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);

  Clear();
  MasterNewOpenOrder("1", OrderDirection::kSell, 80.8, 17);
  Clear();
  MasterTradedOrder("1", 8);

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(8, order->trading_qty);
    EXPECT_EQ(2, order->leaves_qty);
    EXPECT_EQ(10, order->qty);
    EXPECT_EQ(80.8, order->input_price);
    EXPECT_EQ(80.8, order->trading_price);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(OrderDirection::kBuy, order->direction);
    EXPECT_EQ(PositionEffect::kClose, order->position_effect);

    EXPECT_EQ(2, position_qty.position);
    EXPECT_EQ(2, position_qty.frozen);
  }

  EXPECT_FALSE((PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>()));
}

 TEST_F(CTAOrderSignalScriberFixture, ClosingOrder_Fully) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  Clear();
  MasterNewCloseOrder("1", OrderDirection::kBuy, 80.1, 10);
  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
  ASSERT_TRUE(params);
  const auto& order = std::get<0>(*params);
  const auto& position_qty = std::get<1>(*params);

  EXPECT_EQ(0, order->trading_price);
  EXPECT_EQ(10, order->leaves_qty);
  EXPECT_EQ(10, order->qty);
  EXPECT_EQ(80.1, order->input_price);
  EXPECT_EQ(0, order->trading_price);
  EXPECT_EQ(OrderStatus::kActive, order->status);
  EXPECT_EQ(OrderDirection::kBuy, order->direction);
  EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  EXPECT_EQ(10, position_qty.position);
  EXPECT_EQ(10, position_qty.frozen);
}

 TEST_F(CTAOrderSignalScriberFixture, ClosingOrder_Partially) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  Clear();
  MasterNewCloseOrder("1", OrderDirection::kBuy, 80.1, 7);
  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
  ASSERT_TRUE(params);
  const auto& order = std::get<0>(*params);
  const auto& position_qty = std::get<1>(*params);

  EXPECT_EQ(0, order->trading_qty);
  EXPECT_EQ(7, order->leaves_qty);
  EXPECT_EQ(7, order->qty);
  EXPECT_EQ(80.1, order->input_price);
  EXPECT_EQ(0, order->trading_price);
  EXPECT_EQ(OrderStatus::kActive, order->status);
  EXPECT_EQ(OrderDirection::kBuy, order->direction);
  EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  EXPECT_EQ(10, position_qty.position);
  EXPECT_EQ(7, position_qty.frozen);
}

 TEST_F(CTAOrderSignalScriberFixture, Traded_Fully_Close_Order) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  Clear();
  MasterNewCloseOrder("1", OrderDirection::kBuy, 80.1, 10);
  Clear();
  MasterTradedOrder("1", 10);

  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
  ASSERT_TRUE(params);
  const auto& order = std::get<0>(*params);
  const auto& position_qty = std::get<1>(*params);

  EXPECT_EQ(10, order->trading_qty);
  EXPECT_EQ(0, order->leaves_qty);
  EXPECT_EQ(10, order->qty);
  EXPECT_EQ(80.1, order->input_price);
  EXPECT_EQ(80.1, order->trading_price);
  EXPECT_EQ(OrderStatus::kAllFilled, order->status);
  EXPECT_EQ(OrderDirection::kBuy, order->direction);
  EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  EXPECT_EQ(0, position_qty.position);
  EXPECT_EQ(0, position_qty.frozen);
}

 TEST_F(CTAOrderSignalScriberFixture, Traded_Partially_Close_Order) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  Clear();
  MasterNewCloseOrder("1", OrderDirection::kBuy, 80.1, 10);
  Clear();
  MasterTradedOrder("1", 7);

  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
  ASSERT_TRUE(params);
  const auto& order = std::get<0>(*params);
  const auto& position_qty = std::get<1>(*params);

  EXPECT_EQ(7, order->trading_qty);
  EXPECT_EQ(3, order->leaves_qty);
  EXPECT_EQ(10, order->qty);
  EXPECT_EQ(80.1, order->input_price);
  EXPECT_EQ(80.1, order->trading_price);
  EXPECT_EQ(OrderStatus::kActive, order->status);
  EXPECT_EQ(OrderDirection::kBuy, order->direction);
  EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  EXPECT_EQ(3, position_qty.position);
  EXPECT_EQ(3, position_qty.frozen);
}

