#ifndef FOLLOW_TRADE_FOLLOW_STRAGETY_H
#define FOLLOW_TRADE_FOLLOW_STRAGETY_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/context.h"
#include "geek_quant/trade_order_delegate.h"

class FollowStragety {
 public:
  class Delegate {
   public:
    virtual void Trade(const std::string& order_no) = 0;
  };
  FollowStragety(const std::string& master_account_id,
                 const std::string& slave_account_id,
                 TradeOrderDelegate* trade_order_delegate,
                 Delegate* delegate,
                 Context* context);

  void HandleOpening(const OrderData& order_data);

  void HandleCloseing(const OrderData& order_data);

  void HandleCanceled(const OrderData& order_data);

  void HandleClosed(const OrderData& order_data);

  void HandleOpened(const OrderData& order_data);

 private:
  std::string master_account_id_;
  std::string slave_account_id_;
  TradeOrderDelegate* trade_order_delegate_;
  Delegate* delegate_;
  Context* context_;
};

#endif  // FOLLOW_TRADE_FOLLOW_STRAGETY_H
