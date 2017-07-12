#ifndef FOLLOW_STRATEGY_MODE_BASE_FOLLOW_STRAGETY_FACTORY_H
#define FOLLOW_STRATEGY_MODE_BASE_FOLLOW_STRAGETY_FACTORY_H
#include "follow_strategy_mode/base_follow_stragety.h"

class OrdersContext;
class BaseFollowStragetyFactory {
 public:
  virtual BaseFollowStragety* Create(const std::string& master_account_id,
                                     const std::string& slave_account_id,
                                     BaseFollowStragety::Delegate* delegate,
                                     OrdersContext* context) = 0;
};

#endif  // FOLLOW_STRATEGY_MODE_BASE_FOLLOW_STRAGETY_FACTORY_H
