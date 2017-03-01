#ifndef FOLLOW_STRATEGY_H
#define FOLLOW_STRATEGY_H
#include "geek_quant/caf_defines.h"


class FollowStrategy : public FollowTAStrategyActor::base {
 public:
   FollowStrategy(caf::actor_config& cfg);
   virtual ~FollowStrategy();

  virtual FollowTAStrategyActor::behavior_type make_behavior() override;
 private:
  // std::vector<StrategySubscriberActor> subscribers_;
};

#endif /* FOLLOW_STRATEGY_H */
