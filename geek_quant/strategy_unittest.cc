#include "gtest/gtest.h"
#include "caf/all.hpp"
#include "geek_quant/follow_strategy.h"

using namespace caf;

class CafTestCoordinatorFixture : public testing::Test {
  using ConfigType = caf::actor_system_config;
  using SchedulerType = caf::scheduler::test_coordinator;

 public:
  CafTestCoordinatorFixture()
      : sys((cfg.scheduler_policy = caf::atom("testing"), cfg)),
        self(sys),
        sched(dynamic_cast<SchedulerType&>(sys.scheduler())) {}

 protected:
  ConfigType cfg;
  caf::actor_system sys;
  caf::scoped_actor self;
  SchedulerType& sched;
};

class CtaFollowOrderStrategyFixture : public CafTestCoordinatorFixture {
 public:
  CtaFollowOrderStrategyFixture() {
    direction_test = OrderDirection::kODBuy;
    order_action_test = EnterOrderAction::kEOAOpen;
    order_price_test = 0.0;
    volume_test = 0;
    old_volume_test = 0;
    receive = false;
  }
  // Sets up the test fixture.
  virtual void SetUp() {
    strategy_actor = sys.spawn<FollowStrategy>();
    sched.run();
    dummy_listenr = [&](StrategySubscriberActor::pointer self)
        -> StrategySubscriberActor::behavior_type {
      return {
          [&](EnterOrderAtom, EnterOrderData order) {
            instrument_test = order.instrument;
            direction_test = order.order_direction;
            order_no_test = order.order_no;
            order_price_test = order.order_price;
            volume_test = order.volume;
            old_volume_test = order.old_volume;
            order_action_test = order.action;
            receive = true;
          },
          [&](CancelOrderAtom, std::string order_no) {
            order_no_test = order_no;
            order_action_test = EnterOrderAction::kEOACancelForTest;
            receive = true;
          },
      };
    };
    StrategySubscriberActor actor = sys.spawn(dummy_listenr);
    sched.run();
    anon_send(strategy_actor, AddStrategySubscriberAtom::value, actor);
    sched.run();
  }

 protected:
  FollowTAStrategyActor strategy_actor;
  typedef std::function<typename StrategySubscriberActor::behavior_type(
      StrategySubscriberActor::pointer self)>
      DummyType;
  // typedef typename
  // StrategySubscriberActor::behavior_type(*DummyType)(event_based_actor*);
  DummyType dummy_listenr;
  std::string instrument_test;
  std::string order_no_test;
  OrderDirection direction_test;
  EnterOrderAction order_action_test;
  double order_price_test;
  int volume_test;
  int old_volume_test;
  bool receive;
};

/*
OrderStatus : THOST_FTDC_OST_NoTradeQueueing => THOST_FTDC_OST_AllTraded
*/

TEST_F(CtaFollowOrderStrategyFixture, OpenOrder) {
  // Open order
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpening;
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ("0001", order_no_test);
  EXPECT_EQ(OrderDirection::kODBuy, direction_test);
  EXPECT_EQ(EnterOrderAction::kEOAOpen, order_action_test);
  EXPECT_EQ(1234.1, order_price_test);
  EXPECT_EQ(10, volume_test);
  EXPECT_EQ(0, old_volume_test);
  EXPECT_TRUE(receive);

  receive = false;
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpening;
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_FALSE(receive);

  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpened;
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ("0001", order_no_test);
  EXPECT_EQ(OrderDirection::kODBuy, direction_test);
  EXPECT_EQ(EnterOrderAction::kEOAOpenConfirm, order_action_test);
  EXPECT_EQ(1234.1, order_price_test);
  EXPECT_EQ(0, old_volume_test);
  EXPECT_EQ(10, volume_test);
  EXPECT_TRUE(receive);

  // Open reverse order
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0002";
    order.order_direction = OrderDirection::kODSell;
    order.order_status = OrderStatus::kOSOpening;
    order.order_price = 1235.1;
    order.volume = 10;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ("0002", order_no_test);
  EXPECT_EQ(OrderDirection::kODSell, direction_test);
  EXPECT_EQ(EnterOrderAction::kEOAOpenReverseOrder, order_action_test);
  EXPECT_EQ(1235.1, order_price_test);
  EXPECT_EQ(10, volume_test);
  EXPECT_EQ(10, old_volume_test);
  EXPECT_TRUE(receive);

  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0002";
    order.order_direction = OrderDirection::kODSell;
    order.order_status = OrderStatus::kOSOpened;
    order.order_price = 1235.1;
    order.volume = 10;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ("0002", order_no_test);
  EXPECT_EQ(OrderDirection::kODSell, direction_test);
  EXPECT_EQ(EnterOrderAction::kEOAOpenReverseOrderConfirm, order_action_test);
  EXPECT_EQ(1235.1, order_price_test);
  EXPECT_EQ(10, volume_test);
  EXPECT_EQ(10, old_volume_test);
  EXPECT_TRUE(receive);
}

TEST_F(CtaFollowOrderStrategyFixture, CloseOpenOrder) {
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpening;
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_TRUE(receive);

  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpened;
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_TRUE(receive);

  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0002";
    order.order_direction = OrderDirection::kODSell;
    order.order_status = OrderStatus::kOSCloseing;
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }

  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ("0002", order_no_test);
  EXPECT_EQ(OrderDirection::kODSell, direction_test);
  EXPECT_EQ(EnterOrderAction::kEOAClose, order_action_test);
  EXPECT_EQ(1234.1, order_price_test);
  EXPECT_EQ(10, volume_test);
  EXPECT_EQ(10, old_volume_test);
  EXPECT_TRUE(receive);

  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0002";
    order.order_direction = OrderDirection::kODSell;
    order.order_status = OrderStatus::kOSClosed;
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }

  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ("0002", order_no_test);
  EXPECT_EQ(OrderDirection::kODSell, direction_test);
  EXPECT_EQ(EnterOrderAction::kEOACloseConfirm, order_action_test);
  EXPECT_EQ(1234.1, order_price_test);
  EXPECT_EQ(10, volume_test);
  EXPECT_EQ(10, old_volume_test);
  EXPECT_TRUE(receive);
}

TEST_F(CtaFollowOrderStrategyFixture, PartCloseTrade) {
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpening;
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_TRUE(receive);

  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpened;
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_TRUE(receive);

  // Close order part 1
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0002";
    order.order_direction = OrderDirection::kODSell;
    order.order_status = OrderStatus::kOSCloseing;
    order.order_price = 1234.1;
    order.volume = 5;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }

  EXPECT_EQ(5, volume_test);
  EXPECT_EQ(10, old_volume_test);
  EXPECT_TRUE(receive);

  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0002";
    order.order_direction = OrderDirection::kODSell;
    order.order_status = OrderStatus::kOSClosed;
    order.order_price = 1234.1;
    order.volume = 5;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }

  EXPECT_EQ(5, volume_test);
  EXPECT_EQ(10, old_volume_test);
  EXPECT_TRUE(receive);


  // Close order part 2
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0003";
    order.order_direction = OrderDirection::kODSell;
    order.order_status = OrderStatus::kOSCloseing;
    order.order_price = 1235.1;
    order.volume = 5;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }

  EXPECT_EQ(5, volume_test);
  EXPECT_EQ(5, old_volume_test);
  EXPECT_TRUE(receive);

  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0002";
    order.order_direction = OrderDirection::kODSell;
    order.order_status = OrderStatus::kOSClosed;
    order.order_price = 1235.1;
    order.volume = 5;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }

  EXPECT_EQ(5, volume_test);
  EXPECT_EQ(5, old_volume_test);
  EXPECT_TRUE(receive);
}

TEST_F(CtaFollowOrderStrategyFixture, IncreaseOpenOrder) {
  // Open order part 1
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpening;
    order.order_price = 1234.1;
    order.volume = 5;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_TRUE(receive);

  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpened;
    order.order_price = 1234.1;
    order.volume = 5;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_TRUE(receive);

  // Open order part 2
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0002";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpening;
    order.order_price = 1244.1;
    order.volume = 6;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }

  EXPECT_EQ(6, volume_test);
  EXPECT_EQ(5, old_volume_test);
  EXPECT_TRUE(receive);

  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0002";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpened;
    order.order_price = 1234.1;
    order.volume = 6;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }

  EXPECT_EQ(6, volume_test);
  EXPECT_EQ(5, old_volume_test);
  EXPECT_TRUE(receive);
}

TEST_F(CtaFollowOrderStrategyFixture, CancelOrder) {
  // Open order
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpening;
    order.order_price = 1234.1;
    order.volume = 5;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_TRUE(receive);

  // Cancel order
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSCancel;
    order.order_price = 1234.1;
    order.volume = 5;
    anon_send(strategy_actor, TARtnOrderAtom::value, order);
    sched.run();
  }
  EXPECT_TRUE(receive);
  EXPECT_EQ(EnterOrderAction::kEOACancelForTest, order_action_test);
}
