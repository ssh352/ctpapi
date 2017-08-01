#ifndef FOLLOW_TRADE_SERVER_STRATEGY_DRIVIER_H
#define FOLLOW_TRADE_SERVER_STRATEGY_DRIVIER_H
#include "caf/all.hpp"

class StrategyDriver : public caf::event_based_actor {
public:
	virtual caf::behavior make_behavior();
};

#endif // FOLLOW_TRADE_SERVER_STRATEGY_DRIVIER_H



