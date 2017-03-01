#ifndef FOLLOW_STRATEGY_H
#define FOLLOW_STRATEGY_H
#include "geek_quant/caf_defines.h"

class FollowStrategy : public FollowTAStrategyActor::base {
 public:
  FollowStrategy(caf::actor_config& cfg);
  virtual ~FollowStrategy();

  virtual FollowTAStrategyActor::behavior_type make_behavior() override;

 private:
  void HandleOpenging(const OrderRtnData& order);
  void HandleCloseing(const OrderRtnData& order);
  void HandleOpened(const OrderRtnData& order);
  void HandleClosed(const OrderRtnData& order);
  void HandleCancel(const OrderRtnData& order);
  std::vector<StrategySubscriberActor> subscribers_;
  std::vector<OrderRtnData> unfill_orders_;
  std::vector<PositionData> positions_;
};

#endif /* FOLLOW_STRATEGY_H */
