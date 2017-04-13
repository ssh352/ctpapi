#ifndef FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_FIXTURE_H
#define FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_FIXTURE_H
#include "gtest/gtest.h"
#include "follow_strategy_mode/follow_strategy_service.h"
#include "follow_strategy_mode/string_util.h"

extern const char kMasterAccountID[];
extern const char kSlaveAccountID[];

struct OrderInsertForTest {
  std::string instrument;
  std::string order_no;
  OrderDirection direction;
  PositionEffect position_effect;
  OrderPriceType price_type;
  double price;
  int quantity;
};

class FollowStragetyServiceFixture : public testing::Test,
                                     public FollowStragetyService::Delegate {
 public:
  typedef std::tuple<OrderInsertForTest, std::vector<std::string>> TestRetType;
  FollowStragetyServiceFixture();

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          OrderPriceType price_type,
                          double price,
                          int quantity) override;

  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         OrderPriceType price_type,
                         double price,
                         int quantity) override;

  virtual void CancelOrder(const std::string& order_no) override;

 protected:
  void InitDefaultOrderExchangeId(std::string exchange_id);
  void InitService(int seq);

  OrderInsertForTest PopOrderInsert();

  OrderData MakeMasterOrderData(const std::string& order_no,
                                OrderDirection order_direction,
                                PositionEffect position_effect,
                                OrderStatus status,
                                int filled_quantity = 0,
                                int quantity = 10,
                                double order_price = 1234.1,
                                const std::string& instrument = "abc",
                                const std::string& user_product_info = "Q7");

  OrderData MakeSlaveOrderData(
      const std::string& order_no,
      OrderDirection order_direction,
      PositionEffect position_effect,
      OrderStatus status,
      int filled_quantity = 0,
      int quantity = 10,
      double order_price = 1234.1,
      const std::string& instrument = "abc",
      const std::string& user_product_info = kStrategyUserProductInfo);

  TestRetType PushOpenOrderForMaster(
      const std::string& order_id,
      int quantity = 10,
      OrderDirection direction = OrderDirection::kBuy);

  TestRetType PushOpenOrderForSlave(
      const std::string& order_id,
      int quantity = 10,
      OrderDirection direction = OrderDirection::kBuy);

  void PushOpenOrder(const std::string& order_id,
                     int quantity = 10,
                     OrderDirection direction = OrderDirection::kBuy);

  void OpenAndFilledOrder(const std::string& order_id,
                          int quantity = 10,
                          int master_filled_quantity = 10,
                          int slave_filled_quantity = 10,
                          OrderDirection direction = OrderDirection::kBuy);

  TestRetType PushNewOpenOrderForMaster(
      const std::string& order_no = "0001",
      OrderDirection direction = OrderDirection::kBuy,
      int quantity = 10);

  TestRetType PushNewCloseOrderForMaster(
      const std::string& order_id = "0002",
      OrderDirection direction = OrderDirection::kSell,
      int quantity = 10,
      double price = 1234.1,
      PositionEffect position_effect = PositionEffect::kClose);

  TestRetType PushCloseOrderForMaster(
      const std::string& order_id = "0002",
      OrderDirection direction = OrderDirection::kSell,
      int filled_quantity = 10,
      int quantity = 10,
      double price = 1234.1,
      PositionEffect position_effect = PositionEffect::kClose);

  TestRetType PushCancelOrderForMaster(
      const std::string& order_no = "0001",
      OrderDirection direction = OrderDirection::kBuy,
      PositionEffect position_effect = PositionEffect::kOpen,
      int fill_quantity = 0,
      int quantity = 10);

  TestRetType PushNewCloseOrderForSlave(
      const std::string& order_id = "0002",
      OrderDirection direction = OrderDirection::kSell,
      int quantity = 10,
      PositionEffect position_effect = PositionEffect::kClose);


  TestRetType PushCancelOrderForSlave(
      const std::string& order_no = "0001",
      OrderDirection direction = OrderDirection::kBuy,
      PositionEffect position_effect = PositionEffect::kOpen,
      int fill_quantity = 10,
      int quantity = 10);

  FollowStragetyServiceFixture::TestRetType PushOrderForMaster(
      const std::string& order_no,
      OrderDirection direction,
      PositionEffect position_effect,
      OrderStatus status,
      int filled_quantity,
      int quantity,
      double price);

  FollowStragetyServiceFixture::TestRetType PushOrderForSlave(
      const std::string& order_no,
      OrderDirection direction,
      PositionEffect position_effect,
      OrderStatus status,
      int filled_quantity,
      int quantity,
      double price);

  TestRetType PopOrderEffectForTest();

  virtual void SetUp() override;

  std::unique_ptr<FollowStragetyService> service;

  std::deque<OrderInsertForTest> order_inserts;
  std::vector<std::string> cancel_orders;

 private:
  std::string default_order_exchange_id_;
};

#endif  // FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_FIXTURE_H
