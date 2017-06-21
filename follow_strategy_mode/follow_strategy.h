#ifndef FOLLOW_TRADE_FOLLOW_strategy_H
#define FOLLOW_TRADE_FOLLOW_strategy_H
#include "follow_strategy_mode/base_follow_stragety.h"
#include "follow_strategy_mode/context.h"
#include "follow_strategy_mode/defines.h"

class FollowStragety : public BaseFollowStragety {
public:
  FollowStragety(const std::string &master_account_id,
                 const std::string &slave_account_id,
                 BaseFollowStragety::Delegate *delegate, Context *context);

  virtual void HandleOpening(const OrderData &order_data);

  virtual void HandleCloseing(const OrderData &order_data);

  virtual void HandleCanceled(const OrderData &order_data);

  virtual void HandleClosed(const OrderData &order_data);

  virtual void HandleOpened(const OrderData &order_data);

private:
  std::string master_account_id_;
  std::string slave_account_id_;
  BaseFollowStragety::Delegate *delegate_;
  Context *context_;
};

#endif // FOLLOW_TRADE_FOLLOW_strategy_H
