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
  void SendEmptyUnfillOrdersAndPositions() {
    anon_send(order_agent, TAUnfillOrdersAtom::value,
              std::vector<OrderRtnData>{});
    anon_send(order_agent, TAPositionAtom::value, std::vector<PositionData>{});
    sched.run();
  }
  void SendEnterOrder(const char* order_no,
                      EnterOrderAction order_action,
                      OrderDirection order_direction,
                      const char* instrument = "abc",
                      double order_price = 1234.1,
                      int volume = 10,
                      int old_volume = 0) {
    receive = false;
    EnterOrderData order;
    order.action = order_action;
    order.order_direction = order_direction;
    order.instrument = instrument;
    order.order_no = order_no;
    order.order_price = order_price;
    order.volume = volume;
    order.old_volume = old_volume;
    anon_send(order_agent, EnterOrderAtom::value, order);
    sched.run();
  }

  void SendOrderRtn(const char* order_no,
                    OrderStatus order_status,
                    OrderDirection order_direction,
                    const char* instrument = "abc",
                    double order_price = 1234.1,
                    int volume = 10) {
    receive = false;
    OrderRtnData order;
    order.instrument = instrument;
    order.order_no = order_no;
    order.order_direction = order_direction;
    order.order_status = order_status;
    order.order_price = order_price;
    order.volume = volume;
    anon_send(order_agent, TARtnOrderAtom::value, order);
    sched.run();
  }

  void CancelOrder(const char* order_no) {
    receive = false;
    anon_send(order_agent, CancelOrderAtom::value, "0001");
    sched.run();
  }

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
  SendEmptyUnfillOrdersAndPositions();
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

  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ("0001", order_no_test);
  EXPECT_EQ(OrderDirection::kODBuy, direction_test);
  EXPECT_EQ(EnterOrderAction::kEOAOpen, order_action_test);
  EXPECT_EQ(1234.1, order_price_test);
  EXPECT_EQ(10, volume_test);
  EXPECT_TRUE(receive);
}

TEST_F(CtaOrderAgentFixture, CloseOrder) {
  SendEmptyUnfillOrdersAndPositions();
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
  receive = false;
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODBuy;
    order.order_status = OrderStatus::kOSOpened;
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(order_agent, TARtnOrderAtom::value, order);
    sched.run();
  }
  receive = false;
  {
    EnterOrderData order;
    order.action = EnterOrderAction::kEOAClose;
    order.instrument = "abc";
    order.order_direction = OrderDirection::kODSell;
    order.order_no = "0002";
    order.order_price = 1235.1;
    order.volume = 20;
    anon_send(order_agent, EnterOrderAtom::value, order);
    sched.run();
  }

  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ("0002", order_no_test);
  EXPECT_EQ(OrderDirection::kODSell, direction_test);
  EXPECT_EQ(EnterOrderAction::kEOAClose, order_action_test);
  EXPECT_EQ(1235.1, order_price_test);
  EXPECT_EQ(10, volume_test);
  EXPECT_TRUE(receive);
}

TEST_F(CtaOrderAgentFixture, OpenReverseOrder) {
  SendEmptyUnfillOrdersAndPositions();
  {
    PositionData position;
    position.instrument = "abc";
    position.order_direction = OrderDirection::kODBuy;
    position.volume = 10;
    anon_send(order_agent, TAPositionAtom::value,
              std::vector<PositionData>{position});
    sched.run();
  }
  {
    EnterOrderData order;
    order.action = EnterOrderAction::kEOAOpenReverseOrder;
    order.instrument = "abc";
    order.order_direction = OrderDirection::kODSell;
    order.order_no = "0002";
    order.order_price = 1235.1;
    order.volume = 10;
    order.old_volume = 10;
    anon_send(order_agent, EnterOrderAtom::value, order);
    sched.run();
  }

  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ("0002", order_no_test);
  EXPECT_EQ(OrderDirection::kODSell, direction_test);
  EXPECT_EQ(EnterOrderAction::kEOAOpenReverseOrder, order_action_test);
  EXPECT_EQ(1235.1, order_price_test);
  EXPECT_EQ(10, volume_test);
  EXPECT_TRUE(receive);
}

TEST_F(CtaOrderAgentFixture, CancelOrder) {
  SendEmptyUnfillOrdersAndPositions();
  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0001";
    order.order_direction = OrderDirection::kODSell;
    order.order_status = OrderStatus::kOSOpening;
    order.order_price = 1234.1;
    order.volume = 10;
    anon_send(order_agent, TAUnfillOrdersAtom::value,
              std::vector<OrderRtnData>{order});
    sched.run();
  }

  EXPECT_FALSE(receive);
  receive = false;
  {
    anon_send(order_agent, CancelOrderAtom::value, "0001");
    sched.run();
  }
  EXPECT_EQ("0001", order_no_test);
  EXPECT_EQ(EnterOrderAction::kEOACancelForTest, order_action_test);
  EXPECT_TRUE(receive);
}

TEST_F(CtaOrderAgentFixture, TestClosedOrder) {
  SendEmptyUnfillOrdersAndPositions();
  {
    PositionData position;
    position.instrument = "abc";
    position.order_direction = OrderDirection::kODBuy;
    position.volume = 10;
    anon_send(order_agent, TAPositionAtom::value,
              std::vector<PositionData>{position});
    sched.run();
  }

  {
    EnterOrderData order;
    order.action = EnterOrderAction::kEOAClose;
    order.instrument = "abc";
    order.order_direction = OrderDirection::kODSell;
    order.order_no = "0002";
    order.order_price = 1235.1;
    order.volume = 10;
    order.old_volume = 0;
    anon_send(order_agent, EnterOrderAtom::value, order);
    sched.run();
  }
  EXPECT_TRUE(receive);
  receive = false;

  {
    OrderRtnData order;
    order.instrument = "abc";
    order.order_no = "0002";
    order.order_direction = OrderDirection::kODSell;
    order.order_status = OrderStatus::kOSClosed;
    order.order_price = 1235.1;
    order.volume = 10;
    anon_send(order_agent, TARtnOrderAtom::value, order);
    sched.run();
  }

  {
    EnterOrderData order;
    order.action = EnterOrderAction::kEOAClose;
    order.instrument = "abc";
    order.order_direction = OrderDirection::kODSell;
    order.order_no = "0003";
    order.order_price = 1235.1;
    order.volume = 10;
    order.old_volume = 0;
    anon_send(order_agent, EnterOrderAtom::value, order);
    sched.run();
  }

  EXPECT_FALSE(receive);
}

TEST_F(CtaOrderAgentFixture, CancelAlreadyOpenedOrder) {
  SendEmptyUnfillOrdersAndPositions();
  SendEnterOrder("0001", EnterOrderAction::kEOAOpen, OrderDirection::kODBuy);
  SendOrderRtn("0001", OrderStatus::kOSOpened, OrderDirection::kODBuy);
  CancelOrder("0001");

  EXPECT_FALSE(receive);
}

TEST_F(CtaOrderAgentFixture, CancelPartOpenedOrder) {
  SendEmptyUnfillOrdersAndPositions();
  SendEnterOrder("0001", EnterOrderAction::kEOAOpen, OrderDirection::kODBuy);
  SendOrderRtn("0001", OrderStatus::kOSOpened, OrderDirection::kODBuy, "abc",1234.1, 5);
  CancelOrder("0001");

  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ("0001", order_no_test);
  EXPECT_EQ(OrderDirection::kODBuy, direction_test);
  EXPECT_EQ(EnterOrderAction::kEOACancelForTest, order_action_test);
}
