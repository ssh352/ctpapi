#include "gtest/gtest.h"
#include "geek_quant/follow_strategy_service.h"

static const char kMasterAccountID[] = "5000";
static const char kSlaveAccountID[] = "5001";

struct OrderInsertForTest {
  std::string instrument;
  std::string order_no;
  OrderDirection direction;
  PositionEffect position_effect;
  double price;
  int quantity;
};

class FollowStragetyServiceFixture : public testing::Test,
                                     public FollowStragetyService::Delegate {
 public:
  FollowStragetyServiceFixture()
      : service(kMasterAccountID, kSlaveAccountID, this, 1) {}

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) override {
    order_inserts.push_back(OrderInsertForTest{
        instrument, order_no, direction, position_effect, price, quantity});
  }

  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         double price,
                         int quantity) override {
    order_inserts.push_back(OrderInsertForTest{instrument, order_no, direction,
                                               PositionEffect::kOpen, price,
                                               quantity});
  }

  virtual void CancelOrder(const std::string& order_no) override {
    cancel_orders.push_back(order_no);
  }

 protected:
  OrderInsertForTest PopOrderInsert() {
    OrderInsertForTest order_insert = order_inserts.front();
    order_inserts.pop_front();
    return order_insert;
  }

  OrderData MakeMasterOrderData(const std::string& order_no,
                                OrderDirection order_direction,
                                PositionEffect position_effect,
                                OrderStatus status,
                                int filled_quantity = 0,
                                int quantity = 10,
                                double order_price = 1234.1,
                                const std::string& instrument = "abc",
                                const std::string& user_product_info = "Q7") {
    return OrderData{
        kMasterAccountID,   // account_id,
        order_no,           // order_id,
        instrument,         // instrument,
        "",                 // datetime,
        "q7",               // user_product_info,
        quantity,           // quanitty,
        filled_quantity,    // filled_quantity,
        0,                  // session_id,
        order_price,        // price,
        order_direction,    // direction
        OrderType::kLimit,  // type
        status,             // status
        position_effect     // position_effect
    };
  }

  OrderData MakeSlaveOrderData(
      const std::string& order_no,
      OrderDirection order_direction,
      PositionEffect position_effect,
      OrderStatus status,
      int filled_quantity = 0,
      int quantity = 10,
      double order_price = 1234.1,
      const std::string& instrument = "abc",
      const std::string& user_product_info = kStrategyUserProductInfo) {
    return OrderData{
        kSlaveAccountID,           // account_id,
        order_no,                  // order_id,
        instrument,                // instrument,
        "",                        // datetime,
        kStrategyUserProductInfo,  // user_product_info,
        quantity,                  // quanitty,
        filled_quantity,           // filled_quantity,
        1,                         // session_id,
        order_price,               // price,
        order_direction,           // direction
        OrderType::kLimit,         // type
        status,                    // status
        position_effect            // position_effect
    };
  }

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushOpenOrderForMaster(const std::string& order_id,
                         int quantity = 10,
                         OrderDirection direction = OrderDirection::kBuy) {
    service.HandleRtnOrder(
        MakeMasterOrderData(order_id, direction, PositionEffect::kOpen,
                            OrderStatus::kActive, 0, quantity));

    return PopOrderEffectForTest();
  }

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushOpenOrderForSlave(const std::string& order_id,
                        int quantity = 10,
                        OrderDirection direction = OrderDirection::kBuy) {
    service.HandleRtnOrder(
        MakeSlaveOrderData(order_id, direction, PositionEffect::kOpen,
                           OrderStatus::kActive, 0, quantity));

    return PopOrderEffectForTest();
  }

  void PushOpenOrder(const std::string& order_id,
                     int quantity = 10,
                     OrderDirection direction = OrderDirection::kBuy) {
    (void)PushOpenOrderForMaster(order_id, quantity, direction);
    (void)PushOpenOrderForSlave(order_id, quantity, direction);
  }

  void OpenAndFilledOrder(const std::string& order_id,
                          int quantity = 10,
                          int master_filled_quantity = 10,
                          int slave_filled_quantity = 10,
                          OrderDirection direction = OrderDirection::kBuy) {
    PushOpenOrder(order_id, quantity, direction);

    service.HandleRtnOrder(MakeMasterOrderData(
        order_id, direction, PositionEffect::kOpen,
        master_filled_quantity == quantity ? OrderStatus::kFilled
                                           : OrderStatus::kActive,
        master_filled_quantity, quantity));

    if (slave_filled_quantity > 0) {
      service.HandleRtnOrder(MakeSlaveOrderData(
          order_id, direction, PositionEffect::kOpen,
          slave_filled_quantity == quantity ? OrderStatus::kFilled
                                            : OrderStatus::kActive,
          slave_filled_quantity, quantity));
    }
  }

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushNewOpenOrderForMaster(const std::string& order_no = "0001",
                            OrderDirection direction = OrderDirection::kBuy,
                            int quantity = 10) {
    service.HandleRtnOrder(
        MakeMasterOrderData(order_no, direction, PositionEffect::kOpen,
                            OrderStatus::kActive, 0, quantity));
    return PopOrderEffectForTest();
  }

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushNewCloseOrderForMaster(const std::string& order_id = "0002",
                             OrderDirection direction = OrderDirection::kSell,
                             int quantity = 10,
                             double price = 1234.1) {
    service.HandleRtnOrder(
        MakeMasterOrderData(order_id, direction, PositionEffect::kClose,
                            OrderStatus::kActive, 0, quantity, price));

    return PopOrderEffectForTest();
  }

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushNewCloseOrderForSlave(const std::string& order_id = "0002",
                            OrderDirection direction = OrderDirection::kSell,
                            int quantity = 10) {
    service.HandleRtnOrder(
        MakeSlaveOrderData(order_id, direction, PositionEffect::kClose,
                           OrderStatus::kActive, 0, quantity));

    return PopOrderEffectForTest();
  }

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PopOrderEffectForTest() {
    std::vector<std::string> ret_cancel_orders;
    ret_cancel_orders.swap(cancel_orders);
    return {!order_inserts.empty() ? PopOrderInsert() : OrderInsertForTest(),
            ret_cancel_orders};
  }

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushCancelOrderForMaster(
      const std::string& order_no = "0001",
      OrderDirection direction = OrderDirection::kBuy,
      PositionEffect position_effect = PositionEffect::kOpen,
      int fill_quantity = 0,
      int quantity = 10) {
    service.HandleRtnOrder(
        MakeMasterOrderData(order_no, direction, position_effect,
                            OrderStatus::kCancel, fill_quantity, quantity));

    return PopOrderEffectForTest();
  }

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushCancelOrderForSlave(
      const std::string& order_no = "0001",
      OrderDirection direction = OrderDirection::kBuy,
      PositionEffect position_effect = PositionEffect::kOpen,
      int fill_quantity = 10,
      int quantity = 10) {
    service.HandleRtnOrder(
        MakeSlaveOrderData(order_no, direction, position_effect,
                           OrderStatus::kCancel, fill_quantity, quantity));
    return PopOrderEffectForTest();
  }

  FollowStragetyService service;

  std::deque<OrderInsertForTest> order_inserts;
  std::vector<std::string> cancel_orders;
};

// Test Open
TEST_F(FollowStragetyServiceFixture, OpenBuy) {
  auto ret = PushNewOpenOrderForMaster();

  OrderInsertForTest order_insert = std::get<0>(ret);

  EXPECT_EQ("1", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(10, order_insert.quantity);
  EXPECT_EQ(OrderDirection::kBuy, order_insert.direction);
}

TEST_F(FollowStragetyServiceFixture, OpenSell) {
  auto ret = PushNewOpenOrderForMaster("1", OrderDirection::kSell, 20);

  OrderInsertForTest order_insert = std::get<0>(ret);

  EXPECT_EQ("1", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(20, order_insert.quantity);
  EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
}

TEST_F(FollowStragetyServiceFixture, IncreaseOpenOrder) {
  OpenAndFilledOrder("1");

  auto order = std::get<0>(PushNewOpenOrderForMaster("2"));
  EXPECT_EQ("2", order.order_no);
}

// Test Close

TEST_F(FollowStragetyServiceFixture, CloseOrderCase1) {
  OpenAndFilledOrder("1");

  auto order_insert =
      std::get<0>(PushNewCloseOrderForMaster("2", OrderDirection::kSell, 10));
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(10, order_insert.quantity);
}

TEST_F(FollowStragetyServiceFixture, CloseOrderCase2) {
  OpenAndFilledOrder("1", 10, 10, 5);

  auto ret = PushNewCloseOrderForMaster("2");

  auto order_insert = std::get<0>(ret);
  EXPECT_EQ("2", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(5, order_insert.quantity);

  auto cancels = std::get<1>(ret);
  EXPECT_EQ(1, cancels.size());
  EXPECT_EQ("1", cancels.at(0));
}

TEST_F(FollowStragetyServiceFixture, CloseOrderCase3) {
  OpenAndFilledOrder("1", 10, 10, 6);

  {
    auto ret = PushNewCloseOrderForMaster("2", OrderDirection::kSell, 4);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);

    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }

  (void)PushCancelOrderForSlave("1", OrderDirection::kBuy,
                                PositionEffect::kOpen, 10, 6);

  {
    auto ret = PushNewCloseOrderForMaster("3", OrderDirection::kSell, 6);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(6, order_insert.quantity);

    auto cancels = std::get<1>(ret);
    EXPECT_EQ(0, cancels.size());
  }
}

TEST_F(FollowStragetyServiceFixture, CloseOrderCase4) {
  OpenAndFilledOrder("1", 10, 10, 6);

  {
    auto ret = PushNewCloseOrderForMaster("2", OrderDirection::kSell, 5);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("2", order_insert.order_no);
    EXPECT_EQ(1, order_insert.quantity);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }

  (void)PushCancelOrderForSlave("1", OrderDirection::kBuy,
                                PositionEffect::kOpen, 10, 6);

  {
    service.HandleRtnOrder(MakeSlaveOrderData("2", OrderDirection::kSell,
                                              PositionEffect::kClose,
                                              OrderStatus::kActive, 0, 1));
  }

  {
    auto ret = PushNewCloseOrderForMaster("3", OrderDirection::kSell, 5);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(5, order_insert.quantity);

    auto cancels = std::get<1>(ret);
    EXPECT_EQ(0, cancels.size());
  }
}

TEST_F(FollowStragetyServiceFixture, CloseOrderCase5) {
  OpenAndFilledOrder("1", 10, 10, 6);

  {
    auto ret = PushNewCloseOrderForMaster("2", OrderDirection::kSell, 1);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }

  (void)PushCancelOrderForSlave("1", OrderDirection::kBuy,
                                PositionEffect::kOpen, 6, 10);

  {
    auto ret = PushNewCloseOrderForMaster("3", OrderDirection::kSell, 3);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(0, cancels.size());
  }

  {
    auto ret = PushNewCloseOrderForMaster("4", OrderDirection::kSell, 4);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("4", order_insert.order_no);
    EXPECT_EQ(4, order_insert.quantity);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(0, cancels.size());
  }

  {
    service.HandleRtnOrder(MakeSlaveOrderData("4", OrderDirection::kSell,
                                              PositionEffect::kClose,
                                              OrderStatus::kActive, 0, 4));
  }

  {
    auto ret = PushNewCloseOrderForMaster("5", OrderDirection::kSell, 2);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("5", order_insert.order_no);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(2, order_insert.quantity);

    auto cancels = std::get<1>(ret);
    EXPECT_EQ(0, cancels.size());
  }
}

TEST_F(FollowStragetyServiceFixture, CloseOrderCase6) {
  OpenAndFilledOrder("1");
  (void)PushNewCloseOrderForMaster("2", OrderDirection::kSell, 10);
  (void)PushNewCloseOrderForSlave("2", OrderDirection::kSell, 10);
  (void)PushCancelOrderForMaster("2", OrderDirection::kSell,
                                 PositionEffect::kClose);
  (void)PushNewCloseOrderForMaster("3", OrderDirection::kSell, 10, 8888.8);

  {
    auto ret = PushCancelOrderForSlave("2", OrderDirection::kSell,
                                       PositionEffect::kClose);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(8888.8, order_insert.price);
  }
}

// Cancel Order
TEST_F(FollowStragetyServiceFixture, CancelOrderCase1) {
  PushOpenOrder("1");
  {
    auto ret = PushCancelOrderForMaster("1");

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);

    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }
}

TEST_F(FollowStragetyServiceFixture, CancelOrderCase2) {
  OpenAndFilledOrder("1", 10, 5, 0);

  {
    auto ret = PushCancelOrderForMaster("1", OrderDirection::kBuy,
                                        PositionEffect::kOpen, 5, 10);

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }
}

TEST_F(FollowStragetyServiceFixture, CancelOrderCase3) {
  service.HandleRtnOrder(MakeMasterOrderData("1", OrderDirection::kBuy,
                                             PositionEffect::kOpen,
                                             OrderStatus::kActive, 0, 10));

  service.HandleRtnOrder(MakeMasterOrderData("1", OrderDirection::kBuy,
                                             PositionEffect::kOpen,
                                             OrderStatus::kCancel, 0, 10));

  (void)PopOrderInsert();

  service.HandleRtnOrder(MakeSlaveOrderData("1", OrderDirection::kBuy,
                                            PositionEffect::kOpen,
                                            OrderStatus::kActive, 0, 10));
  {
    auto ret = PopOrderEffectForTest();

    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("1", cancels.at(0));
  }
}

// Multi Open Order With One Close

TEST_F(FollowStragetyServiceFixture, FillMultiOrder) {
  OpenAndFilledOrder("1");
  OpenAndFilledOrder("2");

  {
    auto ret =
        PushNewCloseOrderForMaster("3", OrderDirection::kSell, 20, 6666.6);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(6666.6, order_insert.price);
    EXPECT_EQ(20, order_insert.quantity);
  }
}

TEST_F(FollowStragetyServiceFixture, PartFillMultiOrder) {
  OpenAndFilledOrder("1");
  OpenAndFilledOrder("2", 10, 10, 6);
  {
    auto ret =
        PushNewCloseOrderForMaster("3", OrderDirection::kSell, 20, 6666.6);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(6666.6, order_insert.price);
    EXPECT_EQ(16, order_insert.quantity);
    auto cancels = std::get<1>(ret);
    EXPECT_EQ(1, cancels.size());
    EXPECT_EQ("2", cancels.at(0));
  }
}

/*


TEST_F(FollowStragetyServiceFixture, CancelMultiOrder) {
  PushOpenOrderForMaster("1");
  PushOpenOrderForMaster("2");

  (void)PushNewCloseOrderForMaster("3", OrderDirection::kSell, 20, 6666.6);

  {
    auto ret = PushOpenOrderForSlave("1");
    EXPECT_EQ("", std::get<0>(ret).order_no);
    EXPECT_EQ(1, std::get<1>(ret).size());
    EXPECT_EQ("1", std::get<1>(ret).at(0));
  }

  {
    auto ret = PushOpenOrderForSlave("2");
    EXPECT_EQ("", std::get<0>(ret).order_no);
    EXPECT_EQ(1, std::get<1>(ret).size());
    EXPECT_EQ("2", std::get<1>(ret).at(0));
  }
}
*/

// Open Opposite Order

TEST_F(FollowStragetyServiceFixture, OpenOppositeOrderCase1) {
  OpenAndFilledOrder("1");

  {
    auto ret = PushOpenOrderForMaster("2", 10, OrderDirection::kSell);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("2", order_insert.order_no);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }
}

TEST_F(FollowStragetyServiceFixture, OpenOppositeOrderCase2) {
  OpenAndFilledOrder("1", 10, 10, 6);

  {
    auto ret = PushOpenOrderForMaster("2", 10, OrderDirection::kSell);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("2", order_insert.order_no);
    EXPECT_EQ(6, order_insert.quantity);
    EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
    EXPECT_EQ(1, std::get<1>(ret).size());
    EXPECT_EQ("1", std::get<1>(ret).at(0));
  }
}

TEST_F(FollowStragetyServiceFixture, OpenOppositeOrderCase3) {
  OpenAndFilledOrder("1", 10, 5, 0);

  {
    auto ret = PushOpenOrderForMaster("2", 5, OrderDirection::kSell);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }
  {
    auto ret = PushCancelOrderForMaster("1", OrderDirection::kBuy,
                                        PositionEffect::kClose, 5, 10);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("", order_insert.order_no);
    auto cancel_orders = std::get<1>(ret);
    EXPECT_EQ(1, cancel_orders.size());
    EXPECT_EQ("1", cancel_orders.at(0));
  }
}

TEST_F(FollowStragetyServiceFixture, OpenOppositeThenCloseCase1) {
  OpenAndFilledOrder("1", 10, 10, 10, OrderDirection::kBuy);
  OpenAndFilledOrder("2", 10, 10, 10, OrderDirection::kSell);

  {
    auto ret = PushNewCloseOrderForMaster("3", OrderDirection::kSell, 10);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(PositionEffect::kClose, order_insert.position_effect);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }

  (void)PushNewCloseOrderForSlave("3", OrderDirection::kSell, 10);
  {
    auto ret = PushNewCloseOrderForMaster("4", OrderDirection::kBuy, 10);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("4", order_insert.order_no);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(PositionEffect::kClose, order_insert.position_effect);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }
}

TEST_F(FollowStragetyServiceFixture, OpenOppositeThenCloseCase2) {
  OpenAndFilledOrder("1", 10, 10, 10, OrderDirection::kBuy);
  OpenAndFilledOrder("2", 10, 10, 10, OrderDirection::kSell);
  {
    auto ret = PushNewCloseOrderForMaster("3", OrderDirection::kBuy, 10);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("3", order_insert.order_no);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(PositionEffect::kClose, order_insert.position_effect);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }

  (void)PushNewCloseOrderForSlave("3", OrderDirection::kBuy, 10);

  {
    auto ret = PushNewCloseOrderForMaster("4", OrderDirection::kSell, 10);
    auto order_insert = std::get<0>(ret);
    EXPECT_EQ("4", order_insert.order_no);
    EXPECT_EQ(10, order_insert.quantity);
    EXPECT_EQ(PositionEffect::kClose, order_insert.position_effect);
    EXPECT_EQ(0, std::get<1>(ret).size());
  }

}
