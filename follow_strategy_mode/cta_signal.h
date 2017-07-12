#ifndef FOLLOW_STRATEGY_MODE_CTA_SIGNAL_H
#define FOLLOW_STRATEGY_MODE_CTA_SIGNAL_H
#include "cta_sigle_observer.h"
#include "orders_context.h"
#include "enter_order_observer.h"

class CTASignal : public CTASignalObserver {
public:
  void SetObserver(EnterOrderObserver* observer);

  virtual void HandleOpening(const OrderData& order_data) override;

  virtual void HandleCloseing(const OrderData& order_data) override;

  virtual void HandleCanceled(const OrderData& order_data) override;

  virtual void HandleClosed(const OrderData& order_data) override;

  virtual void HandleOpened(const OrderData& order_data) override;
private:
  OrdersContext* master_context_;
  OrdersContext* slave_context_;
  EnterOrderObserver* observer_;
  std::string master_account_id_;
  std::string slave_account_id_;
};

#endif // FOLLOW_STRATEGY_MODE_CTA_SIGNAL_H
