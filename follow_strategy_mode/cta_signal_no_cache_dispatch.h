#ifndef FOLLOW_STRATEGY_MODE_CTA_SIGNAL_NO_CACHE_DISPATCH_H
#define FOLLOW_STRATEGY_MODE_CTA_SIGNAL_NO_CACHE_DISPATCH_H
#include "cta_sigle_observer.h"
#include "enter_order_observer.h"
#include "orders_context.h"
#include "rtn_order_observer.h"

class CTASignalNoCacheDispatch : public EnterOrderObserver,
                                 public RtnOrderObserver {
 public:
  CTASignalNoCacheDispatch(CTASignalObserver* signal_observer);

  virtual void SubscribeEnterOrderObserver(EnterOrderObserver* observer);

  // RtnOrderObservable::Observer
  virtual void RtnOrder(OrderData order) override;

  // EnterOrderObservable::Observer
  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_id,
                         OrderDirection direction,
                         double price,
                         int quantity) override;

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_id,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) override;

  virtual void CancelOrder(const std::string& order_id) override;

  void SetOrdersContext(OrdersContext* master_context,
                        OrdersContext* slave_context);

 private:
  OrderEventType OrdersContextHandleRtnOrder(OrderData order);
  CTASignalObserver* signal_observer_;
  EnterOrderObserver* enter_order_observer_;
  OrdersContext* master_context_;
  OrdersContext* slave_context_;
};
#endif  // FOLLOW_STRATEGY_MODE_CTA_SIGNAL_NO_CACHE_DISPATCH_H
