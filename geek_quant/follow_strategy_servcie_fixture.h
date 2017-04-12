#ifndef FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_FIXTURE_H
#define FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_FIXTURE_H
#include "gtest/gtest.h"
#include "geek_quant/follow_strategy_service.h"



extern const char kMasterAccountID[];
extern const char kSlaveAccountID[];

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
  FollowStragetyServiceFixture();

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) override;

  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         double price,
                         int quantity) override;

  virtual void CancelOrder(const std::string& order_no) override;

 protected:
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

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushOpenOrderForMaster(const std::string& order_id,
                         int quantity = 10,
                         OrderDirection direction = OrderDirection::kBuy);

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushOpenOrderForSlave(const std::string& order_id,
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

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushNewOpenOrderForMaster(const std::string& order_no = "0001",
                            OrderDirection direction = OrderDirection::kBuy,
                            int quantity = 10);

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushNewCloseOrderForMaster(const std::string& order_id = "0002",
                             OrderDirection direction = OrderDirection::kSell,
                             int quantity = 10,
                             double price = 1234.1);

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushNewCloseOrderForSlave(const std::string& order_id = "0002",
                            OrderDirection direction = OrderDirection::kSell,
                            int quantity = 10);

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PopOrderEffectForTest();

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushCancelOrderForMaster(
      const std::string& order_no = "0001",
      OrderDirection direction = OrderDirection::kBuy,
      PositionEffect position_effect = PositionEffect::kOpen,
      int fill_quantity = 0,
      int quantity = 10);

  std::tuple<OrderInsertForTest, std::vector<std::string>>
  PushCancelOrderForSlave(
      const std::string& order_no = "0001",
      OrderDirection direction = OrderDirection::kBuy,
      PositionEffect position_effect = PositionEffect::kOpen,
      int fill_quantity = 10,
      int quantity = 10);

  FollowStragetyService service;

  std::deque<OrderInsertForTest> order_inserts;
  std::vector<std::string> cancel_orders;
};

#endif // FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_FIXTURE_H



