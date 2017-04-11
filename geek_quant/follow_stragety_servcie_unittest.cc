#include "gtest/gtest.h"
#include "geek_quant/follow_stragety_service.h"

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
                                     public TradeOrderDelegate {
 public:
  FollowStragetyServiceFixture()
      : service(kMasterAccountID, kSlaveAccountID, this) {}

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

  void OpenAndFilledOrder(const std::string& order_id,
                          int quantity = 10,
                          int master_filled_quantity = 10,
                          int slave_filled_quantity = 10,
                          OrderDirection direction = OrderDirection::kBuy) {
    service.HandleRtnOrder(
        MakeMasterOrderData(order_id, direction, PositionEffect::kOpen,
                            OrderStatus::kActive, 0, quantity));

    (void)PopOrderInsert();

    service.HandleRtnOrder(
        MakeSlaveOrderData(order_id, direction, PositionEffect::kOpen,
                           OrderStatus::kActive, 0, quantity));

    service.HandleRtnOrder(MakeMasterOrderData(
        order_id, direction, PositionEffect::kOpen, OrderStatus::kFilled,
        master_filled_quantity, quantity));

    service.HandleRtnOrder(MakeSlaveOrderData(
        order_id, direction, PositionEffect::kOpen, OrderStatus::kFilled,
        slave_filled_quantity, quantity));
  }

  std::tuple<OrderInsertForTest, std::vector<std::string> >
  PushNewOpenOrderForMaster(const std::string& order_no = "0001",
                            OrderDirection direction = OrderDirection::kBuy,
                            int quantity = 10) {
    service.HandleRtnOrder(
        MakeMasterOrderData(order_no, direction, PositionEffect::kOpen,
                            OrderStatus::kActive, 0, quantity));
    return {!order_inserts.empty() ? PopOrderInsert() : OrderInsertForTest(),
            {}};
  }

  std::tuple<OrderInsertForTest, std::vector<std::string> >
  PushNewCloseOrderForMaster(const std::string& account_id = "0002",
                             OrderDirection direction = OrderDirection::kSell,
                             int quantity = 10) {
    service.HandleRtnOrder(
        MakeMasterOrderData(account_id, direction, PositionEffect::kClose,
                            OrderStatus::kActive, 0, quantity));
    return {!order_inserts.empty() ? PopOrderInsert() : OrderInsertForTest(),
            {}};
  }

  FollowStragetyService service;

  std::deque<OrderInsertForTest> order_inserts;
};

// Test Open
TEST_F(FollowStragetyServiceFixture, OpenBuy) {
  auto ret = PushNewOpenOrderForMaster();

  OrderInsertForTest order_insert = std::get<0>(ret);

  EXPECT_EQ("1000", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(10, order_insert.quantity);
  EXPECT_EQ(OrderDirection::kBuy, order_insert.direction);
}

TEST_F(FollowStragetyServiceFixture, OpenSell) {
  auto ret = PushNewOpenOrderForMaster("0001", OrderDirection::kSell, 20);

  OrderInsertForTest order_insert = std::get<0>(ret);

  EXPECT_EQ("1000", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(20, order_insert.quantity);
  EXPECT_EQ(OrderDirection::kSell, order_insert.direction);
}

TEST_F(FollowStragetyServiceFixture, IncreaseOpenOrder) {
  OpenAndFilledOrder("0001");

  auto order = std::get<0>(PushNewOpenOrderForMaster("0002"));
  EXPECT_EQ("1001", order.order_no);
}

// Test Close

TEST_F(FollowStragetyServiceFixture, CloseOrderCase1) {
  OpenAndFilledOrder("0001");

  auto order_insert = std::get<0>(
      PushNewCloseOrderForMaster("0002", OrderDirection::kSell, 10));
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(10, order_insert.quantity);
}

TEST_F(FollowStragetyServiceFixture, CloseOrderCase2) {
  OpenAndFilledOrder("0001", 10, 10, 5);

  auto ret = PushNewCloseOrderForMaster("0002");

  auto order_insert = std::get<0>(ret);
  EXPECT_EQ("1001", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(10, order_insert.quantity);

  auto cancels = std::get<1>(ret);
  EXPECT_EQ(1, cancels.size());
  EXPECT_EQ("1000", cancels.at(0));
}
