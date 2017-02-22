#include "gtest/gtest.h"
#include "caf/all.hpp"
#include "geek_quant/follow_strategy.h"

using namespace caf;

behavior DummyListener(event_based_actor* self, const CtpObserver& actor) {
  return {};
}

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

TEST_F(CafTestCoordinatorFixture, Simple) {
  auto strategy_actor = sys.spawn<FollowStrategy>();

  std::string instrument_test;
  Direction direction_test = kDirectionBuy;
  double price_test = 0.0;
  int lots_test = 0;

  sched.run();

  auto dummy = [&](event_based_actor* self) -> behavior {
    self->send(strategy_actor, AddListenerAtom::value,
               actor_cast<strong_actor_ptr>(self));
    return {
        [&](RtnOrderAtom, std::string instrument, Direction direction,
            double price, int lots) {
          instrument_test = instrument;
          direction_test = direction;
          price_test = price;
          lots_test = lots;
        },
    };
  };
  auto listener = sys.spawn(dummy);

  self->receive([](RtnOrderAtom, std::string instrument, Direction direction,
                   double price, int lots) {

  });

  sched.run();

  CThostFtdcOrderField filed = {0};
  strcpy(filed.InstrumentID, "abc");
  filed.Direction = THOST_FTDC_D_Buy;
  filed.LimitPrice = 1234.0;
  filed.VolumeTotalOriginal = 100;
  anon_send(actor_cast<CtpObserver::pointer>(strategy_actor),
            RtnOrderAtom::value, filed);
  sched.run();
  EXPECT_EQ("abc", instrument_test);
  EXPECT_EQ(kDirectionBuy, direction_test);
  EXPECT_EQ(1234.0, price_test);
  EXPECT_EQ(100, lots_test);
}
