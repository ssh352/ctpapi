#ifndef FOLLOW_TRADE_FOLLOW_STRAGETY_H
#define FOLLOW_TRADE_FOLLOW_STRAGETY_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/context.h"

class FollowStragety {
 public:
  class OrderDelegate {
   public:
    virtual void OpenOrder(const std::string& instrument,
                           const std::string& order_no,
                           OrderDirection direction,
                           double price,
                           int quantity) = 0;
  };
  FollowStragety(const std::string& master_account_id,
                 const std::string& slave_account_id,
                 OrderDelegate* del);

  void HandleOpening(RtnOrderData rtn_order, const Context& context_);

  void HandleCloseing(RtnOrderData rtn_order, const Context& context_);

  void HandleCanceled(RtnOrderData rtn_order, const Context& context);

 private:
  std::string master_account_id_;
  std::string slave_account_id_;
  OrderDelegate* delegate_;
};

#endif  // FOLLOW_TRADE_FOLLOW_STRAGETY_H
