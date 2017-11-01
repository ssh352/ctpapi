#include "gtest/gtest.h"
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <tuple>
#include "ctp_instrument_broker.h"

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

  virtual void ReturnOrderField(const std::shared_ptr<OrderField>& order) override {
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


 private:
  std::string GenerateOrderId() {
    return boost::lexical_cast<std::string>(order_seq_++);
  }

  std::list<boost::any> event_queues_;
  int order_seq_ = 0;
};

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect, OpenOrder) {
  broker_.HandleInputOrder(InputOrder{"I1", "0", "S1", PositionEffect::kOpen,
                                      OrderDirection::kBuy, 1.1, 10, 0});
  auto enter_order = PopupOrder<CTPEnterOrder>();
  ASSERT_TRUE(enter_order);
  EXPECT_EQ(CTPPositionEffect::kOpen, enter_order->position_effect);
  EXPECT_EQ(OrderDirection::kBuy, enter_order->direction);
  EXPECT_EQ(10, enter_order->qty);
  EXPECT_EQ(1.1, enter_order->price);
  EXPECT_EQ("I1", enter_order->instrument);
}

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect,
       OpenOrderThenRecvCTPRtnOrder) {
  broker_.HandleInputOrder(InputOrder{"I1", "0", "S1", PositionEffect::kOpen,
                                      OrderDirection::kBuy, 1.1, 10, 0});
  Clear();
  broker_.HandleRtnOrder(MakeCTPOrderField(
      "0", "I1", CTPPositionEffect::kOpen, OrderDirection::kBuy,
      OrderStatus::kActive, 1.1, 10, 0, 10, 0, 0));
  auto order = PopupOrder<std::shared_ptr<OrderField>>();
  EXPECT_EQ("I1", (*order)->instrument_id);
  EXPECT_EQ(PositionEffect::kOpen, (*order)->position_effect);
  EXPECT_EQ(OrderDirection::kBuy, (*order)->direction);
  EXPECT_EQ(1.1, (*order)->input_price);
  EXPECT_EQ(10, (*order)->qty);
}

TEST_F(NoCloseTodayCostNoCloseTodayPositionEffect,
       OpenOrderThenRecvCTPOrderFill) {
}
