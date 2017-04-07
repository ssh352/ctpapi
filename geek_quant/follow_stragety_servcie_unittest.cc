#include "gtest/gtest.h"
#include "geek_quant/follow_stragety_service.h"

static const char kMasterAccountID[] = "5000";
static const char kSlaveAccountID[] = "5001";

struct OrderInsertForTest {
  std::string instrument;
  std::string order_no;
  OrderDirection direction;
  double price;
  int quantity;
};

class FollowStragetyServiceFixture : public testing::Test,
                                     public FollowStragetyService::Delegate {
 public:
  FollowStragetyServiceFixture()
      : service(kMasterAccountID, kSlaveAccountID, this) {}

 protected:
  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         double price,
                         int quantity) override {
    order_inserts.push_back(
        OrderInsertForTest{instrument, order_no, direction, price, quantity});
  }

  OrderInsertForTest PopOrderInsert() {
    OrderInsertForTest order_insert = order_inserts.front();
    order_inserts.pop_front();
    return order_insert;
  }

  RtnOrderData MakeMasterRtnOrderData(const std::string& order_no,
                                      OrderDirection order_direction,
                                      OrderStatus order_status,
                                      int volume = 10,
                                      double order_price = 1234.1,
                                      const std::string& instrument = "abc") {
    return MakeRtnOrderData(kMasterAccountID, order_no, order_direction,
                            order_status, volume, order_price, instrument);
  }

  RtnOrderData MakeSlaveRtnOrderData(const std::string& order_no,
                                     OrderDirection order_direction,
                                     OrderStatus order_status,
                                     int volume = 10,
                                     double order_price = 1234.1,
                                     const std::string& instrument = "abc") {
    return MakeRtnOrderData(kSlaveAccountID, order_no, order_direction,
                            order_status, volume, order_price, instrument);
  }

  RtnOrderData MakeRtnOrderData(const std::string& account_id,
                                const std::string& order_no,
                                OrderDirection order_direction,
                                OrderStatus order_status,
                                int volume = 10,
                                double order_price = 1234.1,
                                const std::string& instrument = "abc") {
    RtnOrderData order;
    order.account_id = account_id;
    order.order_no = std::move(order_no);
    order.order_direction = order_direction;
    order.order_status = order_status;
    order.order_price = order_price;
    order.volume = volume;
    order.instrument = std::move(instrument);
    return order;
  }

  FollowStragetyService service;

  std::deque<OrderInsertForTest> order_inserts;
};

TEST_F(FollowStragetyServiceFixture, OpenOrder) {
  service.HandleRtnOrder(MakeMasterRtnOrderData("0001", OrderDirection::kBuy,
                                                OrderStatus::kOpening));
  auto order_insert = PopOrderInsert();
  EXPECT_EQ("0001", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(10, order_insert.quantity);
}

TEST_F(FollowStragetyServiceFixture, CloseOrder) {
  service.HandleRtnOrder(MakeMasterRtnOrderData("0001", OrderDirection::kBuy,
                                                OrderStatus::kOpening));
  (void)PopOrderInsert();
  service.HandleRtnOrder(MakeSlaveRtnOrderData("0001", OrderDirection::kBuy,
                                               OrderStatus::kOpening));

  service.HandleRtnOrder(MakeMasterRtnOrderData("0001", OrderDirection::kBuy,
                                                OrderStatus::kOpened));

  service.HandleRtnOrder(MakeSlaveRtnOrderData("0001", OrderDirection::kBuy,
                                                OrderStatus::kOpened));

  service.HandleRtnOrder(MakeMasterRtnOrderData("0002", OrderDirection::kSell,
                                                OrderStatus::kCloseing));

  auto order_insert = PopOrderInsert();
  EXPECT_EQ("0002", order_insert.order_no);
  EXPECT_EQ(1234.1, order_insert.price);
  EXPECT_EQ(10, order_insert.quantity);
}
