#include <tuple>
#include "gtest/gtest.h"
#include "unittest_helper.h"
#include "strategy_fixture.h"

#include "follow_strategy/cta_order_signal_subscriber.h"

static const std::string master_account_id = "master";
static const std::string defalut_instrument_id = "default instrument";

class CTAOrderSignalScriberFixture : public StrategyFixture {
 public:
  CTAOrderSignalScriberFixture() : StrategyFixture("slave") {
    mail_box_.Subscribe(&CTAOrderSignalScriberFixture::HandleCTARtnOrderSignal,
                        this);
  }
  void MasterNewOpenAndFill(const std::string& order_id,
                            OrderDirection position_effect_direction,
                            double price,
                            int qty,
                            int trading_qty) {
    MasterNewOpenOrder(order_id, position_effect_direction, price, qty);
    MasterTradedOrder(order_id, trading_qty);
  }

  void MasterNewCloseAndFill(const std::string& order_id,
                             OrderDirection position_effect_direction,
                             double price,
                             int qty,
                             int trading_qty) {
    MasterNewCloseOrder(order_id, position_effect_direction, price, qty);
    MasterTradedOrder(order_id, trading_qty);
  }

  void MasterNewOpenOrder(const std::string& order_id,
                          OrderDirection position_effect_direction,
                          double price,
                          int qty) {
    Send(CTASignalAtom::value,
         MakeNewOpenOrder(master_account_id, order_id, defalut_instrument_id,
                          position_effect_direction, price, qty));
  }

  void MasterNewCloseOrder(const std::string& order_id,
                           OrderDirection position_effect_direction,
                           double price,
                           int qty) {
    Send(CTASignalAtom::value,
         MakeNewCloseOrder(master_account_id, order_id, defalut_instrument_id,
                           position_effect_direction, price, qty));
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
  void HandleCTARtnOrderSignal(const std::shared_ptr<OrderField>& order,
                               const CTAPositionQty& position) {
    event_queues_.push_back(std::make_tuple(order, position));
  }

  virtual void SetUp() override {
    CreateStrategy<CTAOrderSignalSubscriber<UnittestMailBox> >();
    Send(ExchangeStatus::kContinous);
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

TEST_F(CTAOrderSignalScriberFixture, OpeningCompleLockOrder) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  MasterNewOpenAndFill("1", OrderDirection::kSell, 87.1, 10, 10);
  Clear();
  MasterNewOpenOrder("2", OrderDirection::kSell, 85.2, 7);

  MasterTradedOrder("2", 7);

  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
  ASSERT_TRUE(params);
  const auto& order = std::get<0>(*params);
  const auto& position_qty = std::get<1>(*params);

  EXPECT_EQ(7, order->trading_qty);
  EXPECT_EQ(7, order->qty);
  EXPECT_EQ(0, order->leaves_qty);
  EXPECT_EQ(85.2, order->input_price);
  EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
  EXPECT_EQ(OrderDirection::kSell, order->direction);

  EXPECT_EQ(7, position_qty.position);
  EXPECT_EQ(0, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture,
       OpeningPartiallyLockOrderAndOpenLessDirection) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 7, 7);
  MasterNewOpenAndFill("1", OrderDirection::kSell, 87.1, 10, 10);
  Clear();
  MasterNewOpenOrder("2", OrderDirection::kBuy, 85.2, 10);
  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(0, order->trading_qty);
    EXPECT_EQ(3, order->qty);
    EXPECT_EQ(3, order->leaves_qty);
    EXPECT_EQ(85.2, order->input_price);
    EXPECT_EQ(PositionEffect::kClose, order->position_effect);
    EXPECT_EQ(OrderDirection::kBuy, order->direction);
    EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);

    EXPECT_EQ(3, position_qty.position);
    EXPECT_EQ(3, position_qty.frozen);
  }
  ASSERT_FALSE((PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>()));
  MasterTradedOrder("2", 10);
  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(3, order->trading_qty);
    EXPECT_EQ(3, order->qty);
    EXPECT_EQ(0, order->leaves_qty);
    EXPECT_EQ(85.2, order->input_price);
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
    EXPECT_EQ(7, order->qty);
    EXPECT_EQ(0, order->leaves_qty);
    EXPECT_EQ(85.2, order->input_price);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderDirection::kBuy, order->direction);
    EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);

    EXPECT_EQ(7, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }
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
  EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
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
  EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
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
    EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kClose, order->position_effect);

    EXPECT_EQ(10, position_qty.position);
    EXPECT_EQ(10, position_qty.frozen);
  }

  ASSERT_FALSE((PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>()));
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
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
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
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);

    EXPECT_EQ(7, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }
}

TEST_F(CTAOrderSignalScriberFixture, FillLockOrderAndReOpenOneSide) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 1.2, 4, 4);
  MasterNewOpenAndFill("1", OrderDirection::kSell, 1.3, 3, 3);
  MasterNewCloseAndFill("2", OrderDirection::kSell, 1.3, 4, 4);
  MasterNewOpenAndFill("3", OrderDirection::kSell, 1.3, 1, 1);
  MasterNewCloseOrder("4", OrderDirection::kBuy, 1.4, 2);
  Clear();
  MasterTradedOrder("4", 2);
  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(2, order->trading_qty);
    EXPECT_EQ(0, order->leaves_qty);
    EXPECT_EQ(2, order->qty);
    EXPECT_EQ(1.4, order->input_price);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(OrderDirection::kBuy, order->direction);
    EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kClose, order->position_effect);

    EXPECT_EQ(2, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }
}

TEST_F(CTAOrderSignalScriberFixture, CloseingRealAndVirtualBothLockPosition) {
  // READ: https://trello.com/c/AsimJQXc
  // TODO:
  MasterNewOpenOrder("0", OrderDirection::kBuy, 88.8, 1);
  MasterNewOpenOrder("1", OrderDirection::kSell, 80.8, 1);
  MasterTradedOrder("1", 1);
  MasterTradedOrder("0", 1);
  Clear();
  MasterNewCloseOrder("2", OrderDirection::kSell, 81.2, 1);
  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
  // ASSERT_TRUE(params);
  // const auto& order = std::get<0>(*params);
  // const auto& position_qty = std::get<1>(*params);

  // EXPECT_EQ(0, order->trading_qty);
  // EXPECT_EQ(1, order->leaves_qty);
  // EXPECT_EQ(1, order->qty);
  // EXPECT_EQ(81.2, order->input_price);
  // EXPECT_EQ(0.0, order->trading_price);
  // EXPECT_EQ(OrderStatus::kActive, order->status);
  // EXPECT_EQ(OrderDirection::kSell, order->direction);
  // EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
  // EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  // EXPECT_EQ(1, position_qty.position);
  // EXPECT_EQ(1, position_qty.frozen);
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
    EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
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
    EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
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
    EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
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
    EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
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
    EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kClose, order->position_effect);

    EXPECT_EQ(2, position_qty.position);
    EXPECT_EQ(2, position_qty.frozen);
  }

  EXPECT_FALSE((PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>()));
}

TEST_F(CTAOrderSignalScriberFixture, ClosingOrder_Fully) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  Clear();
  MasterNewCloseOrder("1", OrderDirection::kSell, 80.1, 10);
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
  EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
  EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  EXPECT_EQ(10, position_qty.position);
  EXPECT_EQ(10, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture, ClosingOrder_Partially) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  Clear();
  MasterNewCloseOrder("1", OrderDirection::kSell, 80.1, 7);
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
  EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
  EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  EXPECT_EQ(10, position_qty.position);
  EXPECT_EQ(7, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture, CloseingCompleLockOrder) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  MasterNewOpenAndFill("1", OrderDirection::kSell, 81.1, 10, 10);
  Clear();
  MasterNewCloseOrder("2", OrderDirection::kSell, 80.1, 10);
  ASSERT_FALSE((PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>()));
  MasterTradedOrder("2", 10);
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
  EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
  EXPECT_EQ(PositionEffect::kOpen, order->position_effect);

  EXPECT_EQ(10, position_qty.position);
  EXPECT_EQ(0, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture,
       CloseingPartiallyLockOrderAndCloseGreaterDirection) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  MasterNewOpenAndFill("1", OrderDirection::kSell, 81.1, 7, 7);
  Clear();
  MasterNewCloseOrder("2", OrderDirection::kSell, 80.1, 10);
  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(0, order->trading_qty);
    EXPECT_EQ(3, order->leaves_qty);
    EXPECT_EQ(3, order->qty);
    EXPECT_EQ(80.1, order->input_price);
    EXPECT_EQ(0, order->trading_price);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kClose, order->position_effect);

    EXPECT_EQ(3, position_qty.position);
    EXPECT_EQ(3, position_qty.frozen);
  }

  ASSERT_FALSE((PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>()));
  MasterTradedOrder("2", 10);

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(3, order->trading_qty);
    EXPECT_EQ(0, order->leaves_qty);
    EXPECT_EQ(3, order->qty);
    EXPECT_EQ(80.1, order->input_price);
    EXPECT_EQ(80.1, order->trading_price);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
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
    EXPECT_EQ(80.1, order->input_price);
    EXPECT_EQ(80.1, order->trading_price);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);

    EXPECT_EQ(7, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }
}

TEST_F(CTAOrderSignalScriberFixture,
       CloseingPartiallyLockOrderAndCloseGreaterDirectionMultiFill) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  MasterNewOpenAndFill("1", OrderDirection::kSell, 81.1, 7, 7);
  MasterNewCloseOrder("2", OrderDirection::kSell, 80.1, 10);
  Clear();
  MasterTradedOrder("2", 5);
  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(3, order->trading_qty);
    EXPECT_EQ(0, order->leaves_qty);
    EXPECT_EQ(3, order->qty);
    EXPECT_EQ(80.1, order->input_price);
    EXPECT_EQ(80.1, order->trading_price);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kClose, order->position_effect);

    EXPECT_EQ(0, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(2, order->trading_qty);
    EXPECT_EQ(5, order->leaves_qty);
    EXPECT_EQ(7, order->qty);
    EXPECT_EQ(80.1, order->input_price);
    EXPECT_EQ(80.1, order->trading_price);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);

    EXPECT_EQ(2, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }

  MasterTradedOrder("2", 5);
  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(5, order->trading_qty);
    EXPECT_EQ(0, order->leaves_qty);
    EXPECT_EQ(7, order->qty);
    EXPECT_EQ(80.1, order->input_price);
    EXPECT_EQ(80.1, order->trading_price);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);

    EXPECT_EQ(7, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }
}

TEST_F(CTAOrderSignalScriberFixture, Traded_Fully_Close_Order) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  Clear();
  MasterNewCloseOrder("1", OrderDirection::kSell, 80.1, 10);
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
  EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
  EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  EXPECT_EQ(0, position_qty.position);
  EXPECT_EQ(0, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture, Traded_Partially_Close_Order) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  Clear();
  MasterNewCloseOrder("1", OrderDirection::kSell, 80.1, 10);
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
  EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
  EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  EXPECT_EQ(3, position_qty.position);
  EXPECT_EQ(3, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture, CancelCloseOrder) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 10, 10);
  MasterNewCloseOrder("1", OrderDirection::kSell, 80.1, 10);
  Clear();
  MasterCancelOrder("1");
  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();
  ASSERT_TRUE(params);
  const auto& order = std::get<0>(*params);
  const auto& position_qty = std::get<1>(*params);

  EXPECT_EQ(10, order->leaves_qty);
  EXPECT_EQ(10, order->qty);
  EXPECT_EQ(0, order->trading_qty);
  EXPECT_EQ(0, order->trading_price);
  EXPECT_EQ(80.1, order->input_price);
  EXPECT_EQ(OrderStatus::kCanceled, order->status);
  EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
  EXPECT_EQ(PositionEffect::kClose, order->position_effect);

  EXPECT_EQ(10, position_qty.position);
  EXPECT_EQ(0, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture, AuctionDuringNewOpen) {
  Send(ExchangeStatus::kNoTrading);
  MasterNewOpenOrder("0", OrderDirection::kBuy, 88.8, 10);

  auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();

  ASSERT_TRUE(params);
  const auto& order = std::get<0>(*params);
  const auto& position_qty = std::get<1>(*params);

  EXPECT_EQ(10, order->leaves_qty);
  EXPECT_EQ(10, order->qty);
  EXPECT_EQ(0, order->trading_qty);
  EXPECT_EQ(0, order->trading_price);
  EXPECT_EQ(88.8, order->input_price);
  EXPECT_EQ(OrderStatus::kActive, order->status);
  EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
  EXPECT_EQ(PositionEffect::kOpen, order->position_effect);

  EXPECT_EQ(0, position_qty.position);
  EXPECT_EQ(0, position_qty.frozen);
}

TEST_F(CTAOrderSignalScriberFixture, AuctionDuringOverOppositionNewOpen) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 5, 5);
  Clear();
  Send(ExchangeStatus::kNoTrading);
  MasterNewOpenOrder("1", OrderDirection::kSell, 80.1, 8);

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();

    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(5, order->leaves_qty);
    EXPECT_EQ(5, order->qty);
    EXPECT_EQ(0, order->trading_qty);
    EXPECT_EQ(0, order->trading_price);
    EXPECT_EQ(80.1, order->input_price);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kClose, order->position_effect);

    EXPECT_EQ(5, position_qty.position);
    EXPECT_EQ(5, position_qty.frozen);
  }

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();

    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(3, order->leaves_qty);
    EXPECT_EQ(3, order->qty);
    EXPECT_EQ(0, order->trading_qty);
    EXPECT_EQ(0, order->trading_price);
    EXPECT_EQ(80.1, order->input_price);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);

    EXPECT_EQ(0, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }
}

TEST_F(CTAOrderSignalScriberFixture, AuctionDuringCloseLockOrder) {
  MasterNewOpenAndFill("0", OrderDirection::kBuy, 88.8, 5, 5);
  MasterNewOpenAndFill("1", OrderDirection::kSell, 78.1, 5, 5);
  Clear();
  Send(ExchangeStatus::kNoTrading);
  MasterNewCloseOrder("2", OrderDirection::kSell, 80.1, 5);

  {
    auto params = PopupRntOrder<std::shared_ptr<OrderField>, CTAPositionQty>();

    ASSERT_TRUE(params);
    const auto& order = std::get<0>(*params);
    const auto& position_qty = std::get<1>(*params);

    EXPECT_EQ(5, order->leaves_qty);
    EXPECT_EQ(5, order->qty);
    EXPECT_EQ(0, order->trading_qty);
    EXPECT_EQ(0, order->trading_price);
    EXPECT_EQ(80.1, order->input_price);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);

    EXPECT_EQ(0, position_qty.position);
    EXPECT_EQ(0, position_qty.frozen);
  }
}
