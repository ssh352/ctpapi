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
    price_test = 0.0;
    lots_test = 0;
    receive = false;
  }
  // Sets up the test fixture.
  virtual void SetUp() {
    strategy_actor = sys.spawn<FollowStrategy>();
    sched.run();
    dummy_listenr =
        [&](StrategySubscriberActor::pointer self) -> StrategySubscriberActor::behavior_type {
      return {
          [&](EnterOrderAtom, EnterOrderData order) {
            instrument_test = order.instrument;
            direction_test = order.order_direction;
            order_no_test = order.order_no;
            price_test = order.order_price;
            lots_test = order.volume;
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
  double price_test;
  int lots_test;
  bool receive;
};

/*
OrderStatus : THOST_FTDC_OST_NoTradeQueueing => THOST_FTDC_OST_AllTraded
*/

TEST_F(CtaFollowOrderStrategyFixture, OpenThenCloseOrder) {
}

TEST_F(CtaFollowOrderStrategyFixture, CancelOpenOrder) {}

TEST_F(CtaFollowOrderStrategyFixture, PartTrade) {}
