#ifndef FOLLOW_STRATEGY_MODE_CTA_SIGNAL_DISPATCH_H
#define FOLLOW_STRATEGY_MODE_CTA_SIGNAL_DISPATCH_H
#include "cta_sigle_observer.h"
#include "orders_context.h"
#include "enter_order_observer.h"
#include "rtn_order_observer.h"

class CTASignalDispatch : public EnterOrderObserver,
                          public RtnOrderObserver {
 public:
  CTASignalDispatch(CTASignalObserver* signal_observer);

  virtual void SubscribeEnterOrderObserver(EnterOrderObserver* observer);

  // RtnOrderObservable::Observer
  virtual void RtnOrder(OrderData order) override;

  // EnterOrderObservable::Observer
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

  virtual void CancelOrder(const std::string& order_no) override;

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

  std::vector<std::pair<std::string, OrderStatus> > waiting_reply_order_;

  std::deque<OrderData> outstanding_orders_;

  std::string master_account_;

  std::string slave_account_;

  CTASignalObserver* signal_observer_;
  EnterOrderObserver* enter_order_observer_;
  OrdersContext* master_context_;
  OrdersContext* slave_context_;
};
#endif  // FOLLOW_STRATEGY_MODE_CTA_SIGNAL_DISPATCH_H
