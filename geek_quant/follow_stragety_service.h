#ifndef FOLLOW_TRADE_FOLLOW_STRAGETY_SERVICE_H
#define FOLLOW_TRADE_FOLLOW_STRAGETY_SERVICE_H
#include "geek_quant/caf_defines.h"
#include "geek_quant/follow_stragety.h"
#include "geek_quant/context.h"

class FollowStragetyService : public FollowStragety::OrderDelegate {
 public:
  class Delegate {
   public:
    virtual void OpenOrder(const std::string& instrument,
                           const std::string& order_no,
                           OrderDirection direction,
                           double price,
                           int quantity) = 0;
  };
   

  FollowStragetyService(const std::string& master_account,
                        const std::string& slave_account,
                        Delegate* delegate);

  void HandleRtnOrder(RtnOrderData rtn_order);

  void DoHandleRtnOrder(RtnOrderData& rtn_order);

  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         double price,
                         int quantity) override;
  

 private:
  FollowStragety stragety_;
  Context context_;
  Delegate* delegate_;
  std::vector<std::string> waiting_reply_order_;
  std::deque<RtnOrderData> outstanding_rtn_orders_;
  std::string master_account_;
  std::string slave_account_;
};

#endif  // FOLLOW_TRADE_FOLLOW_STRAGETY_SERVICE_H
