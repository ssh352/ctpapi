#ifndef FOLLOW_TRADE_FOLLOW_strategy_H
#define FOLLOW_TRADE_FOLLOW_strategy_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/context.h"

class FollowStragety {
 public:
  class Delegate {
   public:
    virtual void OpenOrder(const std::string& instrument,
                           const std::string& order_no,
                           OrderDirection direction,
                           double price,
                           int quantity) = 0;

    virtual void CloseOrder(const std::string& instrument,
                            const std::string& order_no,
                            OrderDirection direction,
                            PositionEffect position_effect,
                            double price,
                            int quantity) = 0;


    virtual void CancelOrder(const std::string& order_no) = 0;
  };
  FollowStragety(const std::string& master_account_id,
                 const std::string& slave_account_id,
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
  Delegate* delegate_;
  Context* context_;
};

#endif  // FOLLOW_TRADE_FOLLOW_strategy_H
