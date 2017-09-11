#ifndef follow_strategy_CTA_SIGNAL_H
#define follow_strategy_CTA_SIGNAL_H
#include "cta_sigle_observer.h"
#include "enter_order_observer.h"
#include "orders_context.h"

class CTASignal : public CTASignalObserver {
 public:
  void SetOrdersContext(std::shared_ptr<OrdersContext> master_context,
                        std::shared_ptr<OrdersContext> slave_context);

  virtual void HandleOpening(
      const std::shared_ptr<const OrderField>& order_data) override;

  virtual void HandleCloseing(
      const std::shared_ptr<const OrderField>& order_data) override;

  virtual void HandleCanceled(
      const std::shared_ptr<const OrderField>& order_data) override;

  virtual void HandleClosed(
      const std::shared_ptr<const OrderField>& order_data) override;

  virtual void HandleOpened(
      const std::shared_ptr<const OrderField>& order_data) override;

  virtual void Subscribe(CTASignalObserver::Observable* observer) override;

 private:
  std::shared_ptr<OrdersContext> master_context_;
  std::shared_ptr<OrdersContext> slave_context_;
  CTASignalObserver::Observable* observer_;
};

#endif  // follow_strategy_CTA_SIGNAL_H
