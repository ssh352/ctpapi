#ifndef FOLLOW_STRATEGY_MODE_CTA_SIGLE_OBSERVER_H
#define FOLLOW_STRATEGY_MODE_CTA_SIGLE_OBSERVER_H
#include "defines.h"

class CTASignalObserver {
 public:
  virtual void HandleOpening(const OrderData& order_data) = 0;
  virtual void HandleCloseing(const OrderData& order_data) = 0;
  virtual void HandleCanceled(const OrderData& order_data) = 0;
  virtual void HandleClosed(const OrderData& order_data) = 0;
  virtual void HandleOpened(const OrderData& order_data) = 0;
};

#endif  // FOLLOW_STRATEGY_MODE_CTA_SIGLE_OBSERVER_H
