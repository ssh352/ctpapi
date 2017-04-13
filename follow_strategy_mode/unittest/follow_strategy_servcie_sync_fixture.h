#ifndef FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_SYNC_FIXTURE_H
#define FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_SYNC_FIXTURE_H
#include "follow_strategy_mode/unittest/follow_strategy_servcie_fixture.h"

class FollowStragetyServiceSyncFixture : public FollowStragetyServiceFixture {
 protected:
  virtual void SetUp() override { InitService(1000); }
};

#endif  // FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_SYNC_FIXTURE_H
