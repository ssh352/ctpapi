#include "gtest/gtest.h"
#include "caf/all.hpp"
#include "caf_test_fixture.h"
#include "caf_defines.h"
#include "order_agent.h"

class CtaOrderAgentFixture : public CafTestCoordinatorFixture {
 public:
  CtaOrderAgentFixture() {
    direction_test = OrderDirection::kODBuy;
    order_action_test = EnterOrderAction::kEOAOpen;
    order_price_test = 0.0;
    volume_test = 0;
    old_volume_test = 0;
    receive = false;
  }
  // Sets up the test fixture.
  virtual void SetUp() {
    order_agent = sys.spawn<OrderAgent>();
    sched.run();
    dummy_listenr = [&](OrderSubscriberActor::pointer self)
        -> OrderSubscriberActor::behavior_type {
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
    OrderSubscriberActor actor = sys.spawn(dummy_listenr);
    sched.run();
    anon_send(order_agent, AddStrategySubscriberAtom::value, actor);
    sched.run();
  }

 protected:
    FollowTAStrategyActor strategy_actor;
    typedef std::function<typename OrderSubscriberActor::behavior_type(
        OrderSubscriberActor::pointer self)>
        DummyType;
  DummyType dummy_listenr;
  OrderAgentActor order_agent;
  std::string instrument_test;
  std::string order_no_test;
  OrderDirection direction_test;
  EnterOrderAction order_action_test;
  double order_price_test;
  int volume_test;
  int old_volume_test;
  bool receive;
};

TEST_F(CtaOrderAgentFixture, OpenOrder) {
  {
    EnterOrderData order;
    order.action = EnterOrderAction::kEOAOpen;
    order.instrument = "abc";
    order.order_direction = OrderDirection::kODBuy;
    order.order_no = "0001";
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(order_agent, EnterOrderAtom::value, order);
    sched.run();
  }

  EXPECT_TRUE(receive);
}
