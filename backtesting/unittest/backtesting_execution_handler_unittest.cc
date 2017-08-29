
#include "gtest/gtest.h"
#include <list>
#include "execution_handler.h"
#include "event_factory.h"
#include "unittest_helper.h"
#include "backtesting_execution_handler.h"

class TestEventFactory : public AbstractEventFactory {
 public:
  // Inherited via AbstractEventFactory
  virtual void EnqueueTickEvent(
      const std::shared_ptr<TickData>& tick) const override {}

  virtual void EnqueueFillEvent(
      const std::shared_ptr<OrderField>& order) const override {
    orders_.push_back(order);
  }

  virtual void EnqueueInputOrderEvent(const std::string& instrument,
                                      PositionEffect position_effect,
                                      OrderDirection order_direction,
                                      double price,
                                      int qty) const override {}

  virtual void EnqueueCloseMarketEvent() override {}

  std::shared_ptr<OrderField> PopupRntOrder() {
    if (orders_.empty()) {
      return std::shared_ptr<OrderField>();
    }
    auto order = orders_.front();
    orders_.pop_front();
    return std::move(order);
  }

  mutable std::list<std::shared_ptr<OrderField> > orders_;
};

TEST(BacktestingExecutionHandler, OpenBuyOrder) {
  TestEventFactory event_factory;
  SimulatedExecutionHandler execution_handler(&event_factory);

  execution_handler.HandlerInputOrder("S1", PositionEffect::kOpen,
                                      OrderDirection::kBuy, 1.1, 10);
  {
    auto order = event_factory.PopupRntOrder();

    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(0, order->traded_qty);
  }

  execution_handler.HandlerInputOrder("S1", PositionEffect::kOpen,
                                      OrderDirection::kBuy, 0.9, 10);

  {
    auto order = event_factory.PopupRntOrder();

    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(0, order->traded_qty);
  }

  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);
  execution_handler.HandleTick(MakeTick("S1", 1.2, 200));
  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);

  execution_handler.HandleTick(MakeTick("S1", 1.0, 200));

  {
    auto order = event_factory.PopupRntOrder();
    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderDirection::kBuy, order->direction);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(1.1, order->price);
    EXPECT_EQ(10, order->traded_qty);
  }

  execution_handler.HandleTick(MakeTick("S1", 1.0, 200));

  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);

  execution_handler.HandleTick(MakeTick("S1", 0.9, 100));
  {
    auto order = event_factory.PopupRntOrder();
    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(0.9, order->price);
    EXPECT_EQ(10, order->traded_qty);
  }

  execution_handler.HandleTick(MakeTick("S1", 0.9, 100));
  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);
}

TEST(BacktestingExecutionHandler, OpenSellOrder) {
  TestEventFactory event_factory;
  SimulatedExecutionHandler execution_handler(&event_factory);

  execution_handler.HandlerInputOrder("S1", PositionEffect::kOpen,
                                      OrderDirection::kSell, 1.1, 10);
  {
    auto order = event_factory.PopupRntOrder();

    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(0, order->traded_qty);
  }

  execution_handler.HandlerInputOrder("S1", PositionEffect::kOpen,
                                      OrderDirection::kSell, 0.9, 10);

  {
    auto order = event_factory.PopupRntOrder();

    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderStatus::kActive, order->status);
    EXPECT_EQ(0, order->traded_qty);
  }

  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);
  execution_handler.HandleTick(MakeTick("S1", 0.8, 200));
  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);

  execution_handler.HandleTick(MakeTick("S1", 1.0, 200));

  {
    auto order = event_factory.PopupRntOrder();
    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
    EXPECT_EQ(OrderDirection::kSell, order->direction);
    EXPECT_EQ(OrderStatus::kAllFilled, order->status);
    EXPECT_EQ(0.9, order->price);
    EXPECT_EQ(10, order->traded_qty);
  }

  execution_handler.HandleTick(MakeTick("S1", 1.0, 200));

  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);

  execution_handler.HandleTick(MakeTick("S1", 1.1, 100));
  {
    auto order = event_factory.PopupRntOrder();
    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(1.1, order->price);
    EXPECT_EQ(10, order->traded_qty);
  }

  execution_handler.HandleTick(MakeTick("S1", 0.9, 100));
  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);
}

TEST(BacktestingExecutionHandler, CancelOrder) {
  TestEventFactory event_factory;
  SimulatedExecutionHandler execution_handler(&event_factory);

  execution_handler.HandlerInputOrder("S1", PositionEffect::kOpen,
                                      OrderDirection::kSell, 1.1, 10);

  auto order = event_factory.PopupRntOrder();
  EXPECT_EQ(PositionEffect::kOpen, order->position_effect);
  EXPECT_EQ(OrderDirection::kSell, order->direction);
  EXPECT_EQ(OrderStatus::kActive, order->status);
  EXPECT_EQ(1.1, order->price);
  EXPECT_EQ(10, order->qty);
  execution_handler.HandleTick(MakeTick("S1", 0.9, 100));
  EXPECT_EQ(true, event_factory.PopupRntOrder() == nullptr);

  execution_handler.HandleCancelOrder(order->order_id);
  {
    auto order = event_factory.PopupRntOrder();
    EXPECT_EQ(true, order != nullptr);
    EXPECT_EQ(OrderStatus::kCanceled, order->status);
  }
}
