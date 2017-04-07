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
                          OrderDirection direction = OrderDirection::kBuy,
                          int quantity = 10) {
    service.HandleRtnOrder(
        MakeMasterOrderData(order_id, direction, PositionEffect::kOpen,
                            OrderStatus::kActive, 0, quantity));

    (void)PopOrderInsert();

    service.HandleRtnOrder(
        MakeSlaveOrderData(order_id, direction, PositionEffect::kOpen,
                           OrderStatus::kActive, 0, quantity));

    service.HandleRtnOrder(
        MakeMasterOrderData(order_id, direction, PositionEffect::kOpen,
                            OrderStatus::kFilled, quantity, quantity));

    service.HandleRtnOrder(
        MakeSlaveOrderData(order_id, direction, PositionEffect::kOpen,
                           OrderStatus::kFilled, quantity, quantity));
  }

  void PushNewCloseOrder(const std::string& account_id = "0002",
                         OrderDirection direction = OrderDirection::kSell,
                         int quantity = 10) {
    service.HandleRtnOrder(
        MakeMasterOrderData(account_id, direction, PositionEffect::kClose,
                            OrderStatus::kActive, 0, quantity));
  }

  FollowStragetyService service;

  std::deque<OrderInsertForTest> order_inserts;
};

TEST_F(FollowStragetyServiceFixture, OpenOrder) {
  service.HandleRtnOrder(MakeMasterOrderData("0001", OrderDirection::kBuy,
                                             PositionEffect::kOpen,
                                             OrderStatus::kActive));
  auto order_insert = PopOrderInsert();
  EXPECT_EQ("1000", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(10, order_insert.quantity);
}

TEST_F(FollowStragetyServiceFixture, CloseOrder) {
  OpenAndFilledOrder("0001");

  PushNewCloseOrder("0002", OrderDirection::kSell, 10);

  auto order_insert = PopOrderInsert();
  EXPECT_EQ("1001", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(10, order_insert.quantity);
}
