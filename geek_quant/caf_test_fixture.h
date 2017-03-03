#ifndef STRATEGY_UNITTEST_CAF_TEST_FIXTURE_H
#define STRATEGY_UNITTEST_CAF_TEST_FIXTURE_nH
#include "gtest/gtest.h"
#include "caf/all.hpp"


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

#endif // STRATEGY_UNITTEST_CAF_TEST_FIXTURE_H
