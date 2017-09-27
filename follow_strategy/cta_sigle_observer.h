#ifndef follow_strategy_CTA_SIGLE_OBSERVER_H
#define follow_strategy_CTA_SIGLE_OBSERVER_H
#include <boost/shared_ptr.hpp>
#include "common/api_struct.h"

class CTASignalObserver {
 public:
  class Observable {
   public:
    virtual void OpenOrder(const std::string& instrument,
                           const std::string& order_id,
                           OrderDirection direction,
                           double price,
                           int quantity) = 0;

    virtual void CloseOrder(const std::string& instrument,
                            const std::string& order_id,
                            OrderDirection direction,
                            PositionEffect position_effect,
                            double price,
                            int quantity) = 0;

    virtual void CancelOrder(const std::string& order_id) = 0;
  };

  virtual void HandleOpening(
      const std::shared_ptr<const OrderField>& order_data) = 0;
  virtual void HandleCloseing(
      const std::shared_ptr<const OrderField>& order_data) = 0;
  virtual void HandleCanceled(
      const std::shared_ptr<const OrderField>& order_data) = 0;
  virtual void HandleClosed(
      const std::shared_ptr<const OrderField>& order_data) = 0;
  virtual void HandleOpened(
      const std::shared_ptr<const OrderField>& order_data) = 0;
};
#endif  // follow_strategy_CTA_SIGLE_OBSERVER_H
