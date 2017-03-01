#include "geek_quant/follow_strategy.h"
#include "follow_strategy.h"

FollowStrategy::FollowStrategy(caf::actor_config& cfg)
    : FollowTAStrategyActor::base(cfg) {}

FollowStrategy::~FollowStrategy() {}

FollowTAStrategyActor::behavior_type FollowStrategy::make_behavior() {
  return {
      [&](AddStrategySubscriberAtom, StrategySubscriberActor subscriber) {
        // subscribers_.push_back(subscriber);
      },
      [](TAPositionAtom, std::vector<OrderPositionData> positions) {

      },
      [](TAUnfillOrdersAtom, std::vector<OrderRtnData> orders) {

      },
      [](TARtnOrderAtom, OrderRtnData order) {

      },
  };
}
