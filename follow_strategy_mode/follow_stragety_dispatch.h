#ifndef FOLLOW_TRADE_FOLLOW_STRAGETY_DISPATCH_H
#define FOLLOW_TRADE_FOLLOW_STRAGETY_DISPATCH_H
#include <boost/scoped_ptr.hpp>
#include "follow_strategy_mode/base_follow_stragety_factory.h"
#include "follow_strategy_mode/defines.h"
#include "follow_strategy_mode/follow_strategy.h"
#include "follow_strategy_mode/order_id_mananger.h"
#include "follow_strategy_mode/orders_context.h"

class FollowStragetyDispatch : public BaseFollowStragety::Delegate {
 public:
  class Delegate {
   public:
    virtual void OpenOrder(const std::string& instrument,
                           const std::string& order_no,
                           OrderDirection direction,
                           OrderPriceType price_type,
                           double price,
                           int quantity) = 0;


    virtual void CloseOrder(const std::string& instrument,
                            const std::string& order_no,
                            OrderDirection direction,
                            PositionEffect position_effect,
                            OrderPriceType price_type,
                            double price,
                            int quantity) = 0;

    virtual void CancelOrder(const std::string& order_no) = 0;
  };

  FollowStragetyDispatch(const std::string& master_account,
                         const std::string& slave_account,
                         std::shared_ptr<BaseFollowStragety> stragety,
                         std::shared_ptr<Delegate> delegate,
                         std::shared_ptr<OrdersContext> master_context,
                         std::shared_ptr<OrdersContext> slave_context);

  void HandleRtnOrder(OrderData rtn_order);

  virtual void CancelOrder(const std::string& order_no) override;

  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         OrderPriceType price_type,
                         double price,
                         int quantity) override;

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          OrderPriceType price_type,
                          double price,
                          int quantity) override;

  const std::string& master_account_id() const;

  const std::string& slave_account_id() const;

 private:
  enum class StragetyStatus {
    kWaitReply,
    kReady,
    kSkip,
  };

  void Trade(const std::string& order_no, OrderStatus status);

  void DoHandleRtnOrder(OrderData rtn_order);

  StragetyStatus BeforeHandleOrder(OrderData order);

  OrderEventType OrdersContextHandleRtnOrder(OrderData order);

  std::shared_ptr<OrdersContext> master_context_;

  std::shared_ptr<OrdersContext> slave_context_;

  std::shared_ptr<Delegate> delegate_;

  std::vector<std::pair<std::string, OrderStatus> > waiting_reply_order_;

  std::deque<OrderData> outstanding_orders_;

  std::shared_ptr<BaseFollowStragety> stragety_;

  std::string master_account_;

  std::string slave_account_;
};

#endif  // FOLLOW_TRADE_FOLLOW_STRAGETY_DISPATCH_H
