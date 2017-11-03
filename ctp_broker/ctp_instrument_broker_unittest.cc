#include "gtest/gtest.h"
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <tuple>
#include "ctp_instrument_broker.h"

const static std::string default_instrument = "I1";
const static std::string account_id = "A1";

class NoCloseTodayCostNoCloseTodayPositionEffect : public testing::Test,
                                                   public CTPOrderDelegate {
 public:
  NoCloseTodayCostNoCloseTodayPositionEffect()
      : broker_(
            this,
            std::bind(
                &NoCloseTodayCostNoCloseTodayPositionEffect::GenerateOrderId,
                this)) {}
  virtual void EnterOrder(CTPEnterOrder enter_order) override {
    event_queues_.push_back(enter_order);
  }

  virtual void CancelOrder(const std::string& account_id,
                           const std::string& order_id) override {
    event_queues_.push_back(std::make_tuple(account_id, order_id));
  }

  virtual void ReturnOrderField(
      const std::shared_ptr<OrderField>& order) override {
    event_queues_.push_back(order);
  }

  template <typename T>
  boost::optional<T> PopupOrder() {
    if (event_queues_.empty()) {
      return boost::optional<T>();
    }
    auto event = event_queues_.front();
    event_queues_.pop_front();
    return boost::any_cast<T>(event);
  }

  template <typename... Ts>
  std::enable_if_t<sizeof...(Ts) >= 2, boost::optional<std::tuple<Ts...>>>
  PopupOrder() {
    if (event_queues_.empty()) {
      return boost::optional<std::tuple<Ts...>>();
    }
    auto event = event_queues_.front();
    event_queues_.pop_front();
    return boost::any_cast<std::tuple<Ts...>>(event);
  }

  void Clear() { event_queues_.clear(); }

 protected:
  CTPInstrumentBroker broker_;
  std::shared_ptr<CTPOrderField> MakeCTPOrderField(
      const std::string& order_id,
      const std::string& instrument,
      CTPPositionEffect position_effect,
      OrderDirection direction,
      OrderStatus status,
      double price,
      double leaves_qty,
      double traded_qty,
      double qty,
      TimeStamp input_timestamp,
      TimeStamp update_timestamp) {
    auto order = std::make_shared<CTPOrderField>();
    order->order_id = order_id;
    order->position_effect = position_effect;
    order->direction = direction;
    order->status = status;
    order->instrument = instrument;
    order->input_price = price;
    order->leaves_qty = leaves_qty;
    order->trading_qty = traded_qty;
    order->qty = qty;
    order->input_timestamp = input_timestamp;
    order->update_timestamp = update_timestamp;
    return std::move(order);
  }

  void MakeOpenOrderRequest(const std::string& order_id,
                            OrderDirection direction,
                            double price,
                            int qty) {
    broker_.HandleInputOrder(InputOrder{default_instrument, order_id,
                                        account_id, PositionEffect::kOpen,
                                        direction, price, 10, 0});
  }

  void MakeCloseOrderRequest(const std::string& order_id,
                             OrderDirection direction,
                             double price,
                             int qty) {
    broker_.HandleInputOrder(InputOrder{default_instrument, account_id,
                                        order_id, PositionEffect::kClose,
                                        direction, price, 10, 0});
  }

  void SimulateCTPNewOpenOrderField(const std::string& order_id,
                                    OrderDirection direction,
                                    double price,
                                    int qty) {
    auto order = MakeCTPOrderField(
        order_id, default_instrument, CTPPositionEffect::kOpen, direction,
        OrderStatus::kActive, price, qty, 0, qty, 0, 0);
    broker_.HandleRtnOrder(order);
    orders_.emplace(std::move(order));
  }

  void SimulateCTPTradedOrderField(const std::string& order_id, int qty) {
    auto order =
        orders_.find(order_id, HashCTPOrderField(), CompareCTPOrderField());
    (*order)->trading_qty = qty;
    (*order)->leaves_qty -= qty;
    (*order)->status = (*order)->leaves_qty == 0 ? OrderStatus::kAllFilled
                                                 : OrderStatus::kActive;
    (*order)->trading_price = (*order)->input_price;
    BOOST_ASSERT((*order)->leaves_qty >= 0);
    broker_.HandleRtnOrder(*order);
  }

  void SimulateCTPTradedOrderFieldWithPrice(const std::string& order_id,
                                            double trading_price,
                                            int qty) {
    auto order =
        orders_.find(order_id, HashCTPOrderField(), CompareCTPOrderField());
    (*order)->trading_qty = qty;
    (*order)->leaves_qty -= qty;
    (*order)->status = (*order)->leaves_qty == 0 ? OrderStatus::kAllFilled
                                                 : OrderStatus::kActive;
    (*order)->trading_price = trading_price;
    BOOST_ASSERT((*order)->leaves_qty >= 0);
    broker_.HandleRtnOrder(*order);
  }

 private:
  struct HashCTPOrderField {
    size_t operator()(const std::shared_ptr<CTPOrderField>& order) const {
      return std::hash<std::string>()(order->order_id);
    }
    size_t operator()(const std::string& order_id) const {
      return std::hash<std::string>()(order_id);
    }
  };

  struct CompareCTPOrderField {
    bool operator()(const std::shared_ptr<CTPOrderField>& l,
                    const std::shared_ptr<CTPOrderField>& r) const {
      return l->order_id == r->order_id;
    }
    bool operator()(const std::string& l,
                    const std::shared_ptr<CTPOrderField>& r) const {
      return l == r->order_id;
    }
  };
  std::string GenerateOrderId() {
    return boost::lexical_cast<std::string>(order_seq_++);
  }
  std::list<boost::any> event_queues_;

  boost::unordered_set<std::shared_ptr<CTPOrderField>,
                       HashCTPOrderField,
                       CompareCTPOrderField>
      orders_;
  int order_seq_ = 0;
};

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect, OpenOrder) {
  // broker_.HandleInputOrder(InputOrder{"I1", "0", "S1", PositionEffect::kOpen,
  //                                    OrderDirection::kBuy, 1.1, 10, 0});
  MakeOpenOrderRequest("xxxx", OrderDirection::kBuy, 1.1, 10);

  auto enter_order = PopupOrder<CTPEnterOrder>();
  ASSERT_TRUE(enter_order);
  EXPECT_EQ(CTPPositionEffect::kOpen, enter_order->position_effect);
  EXPECT_EQ(OrderDirection::kBuy, enter_order->direction);
  EXPECT_EQ(10, enter_order->qty);
  EXPECT_EQ(1.1, enter_order->price);
  EXPECT_EQ("I1", enter_order->instrument);
  EXPECT_EQ("0", enter_order->order_id);
}

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect,
       OpenOrderThenRecvCTPRtnOrder) {
  MakeOpenOrderRequest("0", OrderDirection::kBuy, 1.1, 10);
  Clear();
  SimulateCTPNewOpenOrderField("0", OrderDirection::kBuy, 1.1, 10);

  auto order = PopupOrder<std::shared_ptr<OrderField>>();
  EXPECT_EQ("I1", (*order)->instrument_id);
  EXPECT_EQ(PositionEffect::kOpen, (*order)->position_effect);
  EXPECT_EQ(OrderDirection::kBuy, (*order)->direction);
  EXPECT_EQ(1.1, (*order)->input_price);
  EXPECT_EQ(10, (*order)->qty);
}

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect,
       OpenOrderThenRecvCTPOrderFill) {
  MakeOpenOrderRequest("0", OrderDirection::kBuy, 1.1, 10);
  SimulateCTPNewOpenOrderField("0", OrderDirection::kBuy, 1.1, 10);
  Clear();
  SimulateCTPTradedOrderField("0", 10);

  auto order = PopupOrder<std::shared_ptr<OrderField>>();
  ASSERT_TRUE(order);
  EXPECT_EQ("I1", (*order)->instrument_id);
  EXPECT_EQ(PositionEffect::kOpen, (*order)->position_effect);
  EXPECT_EQ(OrderDirection::kBuy, (*order)->direction);
  EXPECT_EQ(OrderStatus::kAllFilled, (*order)->status);
  EXPECT_EQ(1.1, (*order)->input_price);
  EXPECT_EQ(10, (*order)->qty);
  EXPECT_EQ(10, (*order)->trading_qty);
  EXPECT_EQ(0, (*order)->leaves_qty);
}

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect,
       OpenOrderThenRecvCTPOrderFillWithTradingPrice) {
  MakeOpenOrderRequest("0", OrderDirection::kBuy, 1.1, 10);
  SimulateCTPNewOpenOrderField("0", OrderDirection::kBuy, 1.1, 10);
  Clear();
  SimulateCTPTradedOrderFieldWithPrice("0", 1.2, 10);

  auto order = PopupOrder<std::shared_ptr<OrderField>>();
  ASSERT_TRUE(order);
  EXPECT_EQ("I1", (*order)->instrument_id);
  EXPECT_EQ(PositionEffect::kOpen, (*order)->position_effect);
  EXPECT_EQ(OrderDirection::kBuy, (*order)->direction);
  EXPECT_EQ(OrderStatus::kAllFilled, (*order)->status);
  EXPECT_EQ(1.1, (*order)->input_price);
  EXPECT_EQ(1.2, (*order)->trading_price);
  EXPECT_EQ(10, (*order)->qty);
  EXPECT_EQ(10, (*order)->trading_qty);
  EXPECT_EQ(0, (*order)->leaves_qty);
}

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect,
       OpenOrderThenReceMultiCTPOrderField) {
  MakeOpenOrderRequest("0", OrderDirection::kBuy, 1.2, 10);
  SimulateCTPNewOpenOrderField("0", OrderDirection::kBuy, 1.2, 10);
  Clear();
  SimulateCTPTradedOrderField("0", 4);

  {
    auto order = PopupOrder<std::shared_ptr<OrderField>>();
    ASSERT_TRUE(order);
    EXPECT_EQ("I1", (*order)->instrument_id);
    EXPECT_EQ(PositionEffect::kOpen, (*order)->position_effect);
    EXPECT_EQ(OrderDirection::kBuy, (*order)->direction);
    EXPECT_EQ(OrderStatus::kActive, (*order)->status);
    EXPECT_EQ(1.2, (*order)->input_price);
    EXPECT_EQ(10, (*order)->qty);
    EXPECT_EQ(4, (*order)->trading_qty);
    EXPECT_EQ(6, (*order)->leaves_qty);
  }

  SimulateCTPTradedOrderField("0", 6);

  {
    auto order = PopupOrder<std::shared_ptr<OrderField>>();
    ASSERT_TRUE(order);
    EXPECT_EQ("I1", (*order)->instrument_id);
    EXPECT_EQ(PositionEffect::kOpen, (*order)->position_effect);
    EXPECT_EQ(OrderDirection::kBuy, (*order)->direction);
    EXPECT_EQ(OrderStatus::kAllFilled, (*order)->status);
    EXPECT_EQ(1.2, (*order)->input_price);
    EXPECT_EQ(10, (*order)->qty);
    EXPECT_EQ(6, (*order)->trading_qty);
    EXPECT_EQ(0, (*order)->leaves_qty);
  }
}

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect, TestDiffrenceOrderId) {
  MakeOpenOrderRequest("xxxx", OrderDirection::kBuy, 1.1, 10);
  Clear();
  SimulateCTPNewOpenOrderField("0", OrderDirection::kBuy, 1.1, 10);

  auto order = PopupOrder<std::shared_ptr<OrderField>>();
  ASSERT_TRUE(order);
  EXPECT_EQ("xxxx", (*order)->order_id);
}

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect, CloseOrderThenRecvCTPOrder) {
  MakeOpenOrderRequest("0", OrderDirection::kBuy, 1.2, 10);
  SimulateCTPNewOpenOrderField("0", OrderDirection::kBuy, 1.2, 10);
  SimulateCTPTradedOrderField("0", 10);
  Clear();
  MakeCloseOrderRequest("1", OrderDirection::kBuy, 1.3, 10);
  auto enter_order = PopupOrder<CTPEnterOrder>();
  ASSERT_TRUE(enter_order);
  EXPECT_EQ(CTPPositionEffect::kClose, enter_order->position_effect);
  EXPECT_EQ(OrderDirection::kBuy, enter_order->direction);
  EXPECT_EQ(10, enter_order->qty);
  EXPECT_EQ(1.3, enter_order->price);
  EXPECT_EQ("I1", enter_order->instrument);
  EXPECT_EQ("1", enter_order->order_id);
}

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect, InitPosition) {
  broker_.InitPosition(10, 0);
  MakeCloseOrderRequest("xx", OrderDirection::kBuy, 1.2, 10);
  auto enter_order = PopupOrder<CTPEnterOrder>();
  ASSERT_TRUE(enter_order);
  EXPECT_EQ("0", enter_order->order_id);
  EXPECT_EQ(10, enter_order->qty);
  EXPECT_EQ(1.2, enter_order->price);
}

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect, CloseToPosition) {
  MakeCloseOrderRequest("xx", OrderDirection::kBuy, 1.2, 10);
  ASSERT_FALSE(PopupOrder<CTPEnterOrder>());
}
