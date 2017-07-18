#ifndef FOLLOW_STRATEGY_MODE_CTA_SIGLE_OBSERVER_H
#define FOLLOW_STRATEGY_MODE_CTA_SIGLE_OBSERVER_H
#include "defines.h"

class CTASignalObserver {
public:
  class Observable {
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

  virtual void HandleOpening(const OrderData& order_data) = 0;
  virtual void HandleCloseing(const OrderData& order_data) = 0;
  virtual void HandleCanceled(const OrderData& order_data) = 0;
  virtual void HandleClosed(const OrderData& order_data) = 0;
  virtual void HandleOpened(const OrderData& order_data) = 0;

  virtual void Subscribe(Observable* observer) = 0;
};
#endif  // FOLLOW_STRATEGY_MODE_CTA_SIGLE_OBSERVER_H
