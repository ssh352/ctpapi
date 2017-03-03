#ifndef STRATEGY_UNITTEST_ORDER_AGENT_H
#define STRATEGY_UNITTEST_ORDER_AGENT_H
#include "caf_defines.h"

class OrderAgent : public OrderAgentActor::base {
 public:
  OrderAgent(caf::actor_config& cfg) : OrderAgentActor::base(cfg) {}
  OrderAgentActor::behavior_type make_behavior();


private:
  OrderSubscriberActor subscriber_;
};

#endif  // STRATEGY_UNITTEST_ORDER_AGENT_H
