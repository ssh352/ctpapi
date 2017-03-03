#ifndef STRATEGY_UNITTEST_ORDER_AGENT_H
#define STRATEGY_UNITTEST_ORDER_AGENT_H
#include "caf_defines.h"

class OrderAgent : public OrderAgentActor::base {
public:
  OrderAgentActor::behavior_type make_behavior();
};

#endif // STRATEGY_UNITTEST_ORDER_AGENT_H



