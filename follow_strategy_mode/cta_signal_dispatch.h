#ifndef FOLLOW_STRATEGY_MODE_CTA_SIGNAL_DISPATCH_H
#define FOLLOW_STRATEGY_MODE_CTA_SIGNAL_DISPATCH_H
#include "cta_sigle_observer.h"
#include "enter_order_observer.h"
#include "orders_context.h"
#include "rtn_order_observer.h"

class CTASignalDispatch : public CTASignalObserver::Observable, public RtnOrderObserver {
 public:
  CTASignalDispatch(std::shared_ptr<CTASignalObserver> signal_observer);

  virtual void SubscribeEnterOrderObserver(std::shared_ptr<EnterOrderObserver> observer);

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

  void SetOrdersContext(std::shared_ptr<OrdersContext> master_context,
                        std::shared_ptr<OrdersContext> slave_context);

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
  std::shared_ptr<CTASignalObserver> signal_observer_;
  std::shared_ptr<EnterOrderObserver> enter_order_observer_;
  std::shared_ptr<OrdersContext> master_context_;
  std::shared_ptr<OrdersContext> slave_context_;
};
#endif  // FOLLOW_STRATEGY_MODE_CTA_SIGNAL_DISPATCH_H
