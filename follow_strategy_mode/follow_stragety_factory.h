#ifndef FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FACTORY_H
#define FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FACTORY_H
#include "follow_strategy_mode/base_follow_stragety.h"


template <class T>
class FollowStragetyFactory : public BaseFollowStragetyFactory {
 public:
  virtual BaseFollowStragety* Create(const std::string& master_account_id,
                                     const std::string& slave_account_id,
                                     BaseFollowStragety::Delegate* delegate,
                                     OrdersContext* context) override {
    return new T(master_account_id, slave_account_id, delegate, context);
  }
};

#endif  // FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FACTORY_H
