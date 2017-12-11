
#include "gtest/gtest.h"
#include <list>

using BeforeTradingAtom = int;
using BacktestingAtom = int;

#include "backtesting/execution_handler.h"
#include "backtesting/backtesting_mail_box.h"
#include "unittest_helper.h"
#include "bft_core/channel_delegate.h"

class UnittestMailBox {
 public:
  UnittestMailBox() {}

  class ChannelDelegateImpl : public bft::ChannelDelegate {
   public:
    virtual void Subscribe(bft::MessageHandler handler) override {
      message_handler_ = std::move(handler);
    }

    virtual void Send(bft::Message message) override {
      message_handler_.message_handler()(message.caf_message());
    }

   private:
    bft::MessageHandler message_handler_;
  };

  void Subscribe(bft::MessageHandler handler)  {
    impl_.Subscribe(std::move(handler));
  }

  template<typename... Ts>
  void Send(Ts&&... args) {
    impl_.Send(bft::MakeMessage(std::forward<Ts>(args)...));
  }

  bft::ChannelDelegate* Impl() {
    return &impl_;
  }

 private:
   ChannelDelegateImpl impl_;
};

class TestEventFactory {
 public:
  TestEventFactory(UnittestMailBox* mail_box) : mail_box_(mail_box) {
    bft::MessageHandler handler;
    handler.Subscribe(&TestEventFactory::EnqueueRtnOrderEvent, this);
    mail_box_->Subscribe(std::move(handler));
  }

  void EnqueueRtnOrderEvent(const std::shared_ptr<OrderField>& order) {
    orders_.push_back(order);
  }

  std::shared_ptr<OrderField> PopupRntOrder() {
    if (orders_.empty()) {
      return std::shared_ptr<OrderField>();
    }
    auto order = orders_.front();
    orders_.pop_front();
    return std::move(order);
  }

  mutable std::list<std::shared_ptr<OrderField>> orders_;

 private:
  UnittestMailBox* mail_box_;
};

auto MakeInputOrder(std::string instrument,
                    PositionEffect position_effect,
                    OrderDirection direction,
                    double price,
                    int qty,
                    TimeStamp timestamp) {
  return InputOrder{instrument, "A1", position_effect, direction,
                    price,      qty,  timestamp};
}

TEST(BacktestingExecutionHandler, OpenBuyOrder) {
  UnittestMailBox mail_box;
  TestEventFactory event_factory(&mail_box);
  SimulatedExecutionHandler execution_handler(mail_box.Impl(), true);

  mail_box.Send(BacktestingAtom(),
                MakeInputOrder("S1", PositionEffect::kOpen,
                               OrderDirection::kBuy, 1.1, 10, 0));
  {
    auto order = event_factory.PopupRntOrder();

    ASSERT_TRUE(order);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(0, order->trading_qty);
  }

  mail_box.Send(BacktestingAtom(),
                MakeInputOrder("S1", PositionEffect::kOpen,
                               OrderDirection::kBuy, 0.9, 10, 0));

  {
    auto order = event_factory.PopupRntOrder();

    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(0, order->trading_qty);
  }

  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);
  execution_handler.HandleTick(MakeTick("S1", 1.2, 200));
  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);

  execution_handler.HandleTick(MakeTick("S1", 1.0, 200));

  {
    auto order = event_factory.PopupRntOrder();
    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderDirection::kBuy, order->position_effect_direction);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(1.1, order->input_price);
    EXPECT_EQ(10, order->trading_qty);
  }

  execution_handler.HandleTick(MakeTick("S1", 1.0, 200));

  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);

  execution_handler.HandleTick(MakeTick("S1", 0.9, 100));
  {
    auto order = event_factory.PopupRntOrder();
    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(0.9, order->input_price);
    EXPECT_EQ(10, order->trading_qty);
  }

  execution_handler.HandleTick(MakeTick("S1", 0.9, 100));
  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);
}

TEST(BacktestingExecutionHandler, OpenSellOrder) {
  UnittestMailBox mail_box;
  TestEventFactory event_factory(&mail_box);
  SimulatedExecutionHandler execution_handler(mail_box.Impl(), true);

  mail_box.Send(BacktestingAtom(),
                MakeInputOrder("S1", PositionEffect::kOpen,
                               OrderDirection::kSell, 1.1, 10, 0));
  {
    auto order = event_factory.PopupRntOrder();

    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(0, order->trading_qty);
  }

  mail_box.Send(BacktestingAtom(),
                MakeInputOrder("S1", PositionEffect::kOpen,
                               OrderDirection::kSell, 0.9, 10, 0));

  {
    auto order = event_factory.PopupRntOrder();

    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(0, order->trading_qty);
  }

  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);
  execution_handler.HandleTick(MakeTick("S1", 0.8, 200));
  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);

  execution_handler.HandleTick(MakeTick("S1", 1.0, 200));

  {
    auto order = event_factory.PopupRntOrder();
    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(0.9, order->input_price);
    EXPECT_EQ(10, order->trading_qty);
  }

  execution_handler.HandleTick(MakeTick("S1", 1.0, 200));

  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);

  execution_handler.HandleTick(MakeTick("S1", 1.1, 100));
  {
    auto order = event_factory.PopupRntOrder();
    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(1.1, order->input_price);
    EXPECT_EQ(10, order->trading_qty);
  }

  execution_handler.HandleTick(MakeTick("S1", 0.9, 100));
  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);
}

TEST(BacktestingExecutionHandler, CancelOrder) {
  UnittestMailBox mail_box;
  TestEventFactory event_factory(&mail_box);
  SimulatedExecutionHandler execution_handler(mail_box.Impl(), true);

  mail_box.Send(BacktestingAtom(),
                MakeInputOrder("S1", PositionEffect::kOpen,
                               OrderDirection::kSell, 1.1, 10, 0));

  auto order = event_factory.PopupRntOrder();
  ASSERT_TRUE(order);
  EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
  EXPECT_EQ(OrderDirection::kSell, order->position_effect_direction);
  EXPECT_EQ(OrderStatus::kActive, order->status);
  EXPECT_EQ(1.1, order->input_price);
  EXPECT_EQ(10, order->qty);
  execution_handler.HandleTick(MakeTick("S1", 0.9, 100));
  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);

  mail_box.Send(BacktestingAtom(), CancelOrder{order->order_id});
  {
    auto order = event_factory.PopupRntOrder();
    ASSERT_TRUE(order);
    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(OrderStatus::kCanceled, order->status);
    EXPECT_EQ(10, order->qty);
    EXPECT_EQ(10, order->leaves_qty);
    EXPECT_EQ(0, order->trading_qty);
  }
}
