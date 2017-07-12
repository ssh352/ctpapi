#ifndef FOLLOW_TRADE_FOLLOW_strategy_H
#define FOLLOW_TRADE_FOLLOW_strategy_H
#include "follow_strategy_mode/base_follow_stragety.h"
#include "follow_strategy_mode/defines.h"
#include "follow_strategy_mode/orders_context.h"

class FollowStragety : public BaseFollowStragety {
 public:
   FollowStragety(const std::string& master_account_id,
     const std::string& slave_account_id,
     BaseFollowStragety::Delegate* delegate,
     std::shared_ptr<OrdersContext> master_context,
     std::shared_ptr<OrdersContext> slave_context);

  virtual void HandleOpening(const OrderData& order_data);

  virtual void HandleCloseing(const OrderData& order_data);

  virtual void HandleCanceled(const OrderData& order_data);

  virtual void HandleClosed(const OrderData& order_data);

  virtual void HandleOpened(const OrderData& order_data);

 private:
  std::string master_account_id_;
  std::string slave_account_id_;
  BaseFollowStragety::Delegate* delegate_;
  std::shared_ptr<OrdersContext> master_context_;
  std::shared_ptr<OrdersContext> slave_context_;
};

#endif  // FOLLOW_TRADE_FOLLOW_strategy_H
