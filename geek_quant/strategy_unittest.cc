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
    direction_test = OrderDirection::kBuy;
    order_action_test = OrderAction::kOpen;
    price_test = 0.0;
    lots_test = 0;
    receive = false;
  }
  // Sets up the test fixture.
  virtual void SetUp() {
    strategy_actor = sys.spawn<FollowStrategy>();
    sched.run();
    dummy_listenr =
        [&](event_based_actor* self) -> StrategyOrderAction::behavior_type {
      self->send(strategy_actor, AddListenerAtom::value,
                 actor_cast<strong_actor_ptr>(self));
      return {
          [&](OpenOrderAtom, std::string instrument, std::string order_no,
              OrderDirection direction, double price, int lots) {
            instrument_test = instrument;
            direction_test = direction;
            order_no_test = order_no;
            price_test = price;
            lots_test = lots;
            order_action_test = OrderAction::kOpen;
            receive = true;
          },
          [&](CloseOrderAtom, std::string instrument, std::string order_no,
              OrderDirection direction, double price, int lots) {
            instrument_test = instrument;
            order_no_test = order_no;
            direction_test = direction;
            price_test = price;
            lots_test = lots;
            order_action_test = OrderAction::kClose;
            receive = true;
          },
          [&](CancelOrderAtom, std::string order_no) {
            order_no_test = order_no;
            order_action_test = OrderAction::kCancel;
            receive = true;
          },
      };
    };
    sys.spawn(dummy_listenr);
    sched.run();
  }


 protected:
  CtpObserver strategy_actor;
  typedef std::function<typename StrategyOrderAction::behavior_type(
      event_based_actor*)>
      DummyType;
  // typedef typename
  // StrategyOrderAction::behavior_type(*DummyType)(event_based_actor*);
  DummyType dummy_listenr;
  std::string instrument_test;
  std::string order_no_test;
  OrderDirection direction_test;
  OrderAction order_action_test;
  double price_test;
  int lots_test;
  bool receive;
};

/*
OrderStatus : THOST_FTDC_OST_NoTradeQueueing => THOST_FTDC_OST_AllTraded
*/

TEST_F(CtaFollowOrderStrategyFixture, OpenThenCloseOrder) {
  // Test Open Order
  {
    CThostFtdcOrderField filed = {0};
    strcpy(filed.InstrumentID, "abc");
    filed.Direction = THOST_FTDC_D_Buy;
    filed.LimitPrice = 1234.0;
    filed.VolumeTotalOriginal = 2;
    filed.OrderSubmitStatus = THOST_FTDC_OSS_InsertSubmitted;
    filed.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    filed.OrderStatus = THOST_FTDC_OST_Unknown;
    filed.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
    strcpy(filed.OrderRef, "0001");
    anon_send(actor_cast<CtpObserver::pointer>(strategy_actor),
              CtpRtnOrderAtom::value, filed);
    sched.run();
  }

  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ(OrderDirection::kBuy, direction_test);
  EXPECT_EQ(OrderAction::kOpen, order_action_test);
  EXPECT_EQ(1234.0, price_test);
  EXPECT_EQ(2, lots_test);

  receive = false;
  {
    CThostFtdcOrderField filed = {0};
    strcpy(filed.InstrumentID, "abc");
    filed.Direction = THOST_FTDC_D_Buy;
    filed.LimitPrice = 1234.0;
    filed.VolumeTotalOriginal = 2;
    filed.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    filed.OrderSubmitStatus = THOST_FTDC_OSS_Accepted;
    filed.OrderStatus = THOST_FTDC_OST_NoTradeQueueing;
    filed.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
    strcpy(filed.OrderRef, "0001");
    anon_send(actor_cast<CtpObserver::pointer>(strategy_actor),
              CtpRtnOrderAtom::value, filed);
    sched.run();
  }

  EXPECT_FALSE(receive);

  receive = false;
  {
    CThostFtdcOrderField filed = {0};
    strcpy(filed.InstrumentID, "abc");
    filed.Direction = THOST_FTDC_D_Buy;
    filed.LimitPrice = 1234.0;
    filed.VolumeTotalOriginal = 2;
    filed.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    filed.OrderSubmitStatus = THOST_FTDC_OSS_Accepted;
    filed.OrderStatus = THOST_FTDC_OST_AllTraded;
    filed.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
    strcpy(filed.OrderRef, "0001");
    anon_send(actor_cast<CtpObserver::pointer>(strategy_actor),
              CtpRtnOrderAtom::value, filed);
    sched.run();
  }

  EXPECT_FALSE(receive);

  // Test Close Order
  receive = false;
  {
    CThostFtdcOrderField filed = {0};
    strcpy(filed.InstrumentID, "abc");
    filed.Direction = THOST_FTDC_D_Sell;
    filed.LimitPrice = 1240.0;
    filed.VolumeTotalOriginal = 2;
    filed.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    filed.OrderSubmitStatus = THOST_FTDC_OSS_InsertSubmitted;
    filed.OrderStatus = THOST_FTDC_OST_Unknown;
    filed.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
    strcpy(filed.OrderRef, "0002");
    anon_send(actor_cast<CtpObserver::pointer>(strategy_actor),
              CtpRtnOrderAtom::value, filed);
    sched.run();
  }

  EXPECT_TRUE(receive);
  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ(OrderDirection::kSell, direction_test);
  EXPECT_EQ(OrderAction::kClose, order_action_test);
  EXPECT_EQ(1240.0, price_test);
  EXPECT_EQ(2, lots_test);

  receive = false;
  {
    CThostFtdcOrderField filed = {0};
    strcpy(filed.InstrumentID, "abc");
    filed.Direction = THOST_FTDC_D_Sell;
    filed.LimitPrice = 1240.0;
    filed.VolumeTotalOriginal = 2;
    filed.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    filed.OrderSubmitStatus = THOST_FTDC_OSS_Accepted;
    filed.OrderStatus = THOST_FTDC_OST_NoTradeQueueing;
    filed.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
    strcpy(filed.OrderRef, "0002");
    anon_send(actor_cast<CtpObserver::pointer>(strategy_actor),
              CtpRtnOrderAtom::value, filed);
    sched.run();
  }

  EXPECT_FALSE(receive);

  receive = false;
  {
    CThostFtdcOrderField filed = {0};
    strcpy(filed.InstrumentID, "abc");
    filed.Direction = THOST_FTDC_D_Sell;
    filed.LimitPrice = 1240.0;
    filed.VolumeTotalOriginal = 2;
    filed.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    filed.OrderSubmitStatus = THOST_FTDC_OSS_Accepted;
    filed.OrderStatus = THOST_FTDC_OST_AllTraded;
    filed.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
    strcpy(filed.OrderRef, "0002");
    anon_send(actor_cast<CtpObserver::pointer>(strategy_actor),
              CtpRtnOrderAtom::value, filed);
    sched.run();
  }

  EXPECT_FALSE(receive);
}

TEST_F(CtaFollowOrderStrategyFixture, CancelOpenOrder) {
  // Test cancel open order
  {
    CThostFtdcOrderField filed = {0};
    strcpy(filed.InstrumentID, "abc");
    filed.Direction = THOST_FTDC_D_Buy;
    filed.LimitPrice = 1234.0;
    filed.VolumeTotalOriginal = 2;
    filed.OrderSubmitStatus = THOST_FTDC_OSS_InsertSubmitted;
    filed.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    filed.OrderStatus = THOST_FTDC_OST_Unknown;
    filed.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
    strcpy(filed.OrderRef, "0001");
    anon_send(actor_cast<CtpObserver::pointer>(strategy_actor),
              CtpRtnOrderAtom::value, filed);
    sched.run();
  }


  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ(OrderDirection::kBuy, direction_test);
  EXPECT_EQ(OrderAction::kOpen, order_action_test);
  EXPECT_EQ(1234.0, price_test);
  EXPECT_EQ(2, lots_test);

  receive = false;
  {
    CThostFtdcOrderField filed = {0};
    strcpy(filed.InstrumentID, "abc");
    filed.Direction = THOST_FTDC_D_Buy;
    filed.LimitPrice = 1234.0;
    filed.VolumeTotalOriginal = 2;
    filed.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    filed.OrderSubmitStatus = THOST_FTDC_OSS_Accepted;
    filed.OrderStatus = THOST_FTDC_OST_NoTradeQueueing;
    filed.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
    strcpy(filed.OrderRef, "0001");
    anon_send(actor_cast<CtpObserver::pointer>(strategy_actor),
              CtpRtnOrderAtom::value, filed);
    sched.run();
  }

  EXPECT_FALSE(receive);

  receive = false;
  {
    CThostFtdcOrderField filed = {0};
    strcpy(filed.InstrumentID, "abc");
    filed.Direction = THOST_FTDC_D_Buy;
    filed.LimitPrice = 1234.0;
    filed.VolumeTotalOriginal = 2;
    filed.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    filed.OrderSubmitStatus = THOST_FTDC_OSS_Accepted;
    filed.OrderStatus = THOST_FTDC_OST_Canceled;
    filed.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
    strcpy(filed.OrderRef, "0001");
    anon_send(actor_cast<CtpObserver::pointer>(strategy_actor),
              CtpRtnOrderAtom::value, filed);
    sched.run();
  }

  EXPECT_TRUE(receive);
  EXPECT_EQ("0001", order_no_test);
  EXPECT_EQ(OrderAction::kCancel, order_action_test);
}

TEST_F(CtaFollowOrderStrategyFixture, PartTrade) {

}

